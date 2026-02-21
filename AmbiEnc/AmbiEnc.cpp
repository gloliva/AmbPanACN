// AmbiEnc.cpp
// 1st through 7th order Ambisonics Encoders
// For basic functionality like panning azimuth and elevation values

#include "chugin.h"
#include <cmath>

const int MAX_CHANNELS = 64;
static t_CKUINT ambienc_bounds_normalized = 0;
static t_CKUINT ambienc_bounds_radians = 1;


// declaration of chugin functions
#define DECLARE_ORDER_FUNCS(N)                           \
    CK_DLL_CTOR(ambienc##N##_ctor);                      \
    CK_DLL_CTOR(ambienc##N##_ctor_period);               \
    CK_DLL_CTOR(ambienc##N##_ctor_periodAndBounds);      \
    CK_DLL_DTOR(ambienc##N##_dtor);                      \
    CK_DLL_TICKF(ambienc##N##_tickf);                    \
    CK_DLL_MFUN(ambienc##N##_setAzimuth);                \
    CK_DLL_MFUN(ambienc##N##_getAzimuth);                \
    CK_DLL_MFUN(ambienc##N##_setElevation);              \
    CK_DLL_MFUN(ambienc##N##_getElevation);              \
    CK_DLL_MFUN(ambienc##N##_pan);                       \
    CK_DLL_MFUN(ambienc##N##_setUpdatePeriod);           \
    CK_DLL_MFUN(ambienc##N##_getUpdatePeriod);           \
    CK_DLL_MFUN(ambienc##N##_setBoundsType);             \
    CK_DLL_MFUN(ambienc##N##_getBoundsType);             \
    t_CKINT ambienc##N##_data_offset = 0;

DECLARE_ORDER_FUNCS(1)
DECLARE_ORDER_FUNCS(2)
DECLARE_ORDER_FUNCS(3)
DECLARE_ORDER_FUNCS(4)
DECLARE_ORDER_FUNCS(5)
DECLARE_ORDER_FUNCS(6)
DECLARE_ORDER_FUNCS(7)


// class definition for internal chugin data
class AmbiEnc
{
public:
    AmbiEnc( t_CKINT order, t_CKINT update_period, t_CKINT bounds_type )
    {
        m_order = order;
        m_out_channels = (order + 1) * (order + 1);
        m_azimuth = 0;
        m_elevation = 0;
        m_pan_change = false;
        m_bounds_type = bounds_type;
        m_update_period = (update_period < 1 ? 1 : update_period);
        m_samples_left = 0;

        for (int c = 0; c < MAX_CHANNELS; c++) {
            m_gain_cur[c]  = 0;
            m_gain_next[c] = 0;
            m_gain_step[c] = 0;
        }

        // Compute initial coefficients and gains
        compute_coeffs();
        compute_gains();
        for (int c = 0; c < MAX_CHANNELS; c++)
            m_gain_cur[c] = m_gain_next[c];
    }

    // setters
    t_CKFLOAT setAzimuth( t_CKFLOAT a )
    {
        if (m_bounds_type == ambienc_bounds_normalized)
            a = scalef(a, -1., 1., -M_PI, M_PI);
        if (a != m_azimuth) { m_azimuth = a; m_pan_change = true; }
        return m_azimuth;
    }

    t_CKFLOAT setElevation( t_CKFLOAT e )
    {
        if (m_bounds_type == ambienc_bounds_normalized)
            e = scalef(e, -1., 1., -M_PI, M_PI);
        if (e != m_elevation) { m_elevation = e; m_pan_change = true; }
        return m_elevation;
    }

    t_CKVEC2 pan( t_CKFLOAT a, t_CKFLOAT e )
    {
        if (m_bounds_type == ambienc_bounds_normalized) {
            a = scalef(a, -1., 1., -M_PI, M_PI);
            e = scalef(e, -1., 1., -M_PI, M_PI);
        }
        m_azimuth = a; m_elevation = e; m_pan_change = true;
        t_CKVEC2 v; v.x = m_azimuth; v.y = m_elevation;
        return v;
    }

    t_CKINT setUpdatePeriod( t_CKINT p )
    {
        m_update_period = (p < 1 ? 1 : p);
        m_samples_left = 0;
        return m_update_period;
    }

    t_CKINT setBoundsType(t_CKINT b)
    {
        if (b == ambienc_bounds_normalized || b == ambienc_bounds_radians) {
            m_bounds_type = b;
            return b;
        }

        return -1;
    }

    // getters
    t_CKFLOAT getAzimuth()
    {
        return m_azimuth;
    }

    t_CKFLOAT getElevation()
    {
        return m_elevation;
    }

    t_CKINT getUpdatePeriod()
    {
        return m_update_period;
    }

    t_CKINT getBoundsType()
    {
        return m_bounds_type;
    }

    // tick template
    template<int N_CH>
    void tick( SAMPLE * in, SAMPLE * out, int nframes )
    {
        for (int f = 0; f < nframes; f++) {
            // check if we need to recompute gains
            if (m_samples_left <= 0 && m_pan_change) {
                compute_gains();
                for (int c = 0; c < N_CH; c++)
                    m_gain_step[c] = (m_gain_next[c] - m_gain_cur[c]) / m_update_period;
                m_samples_left = m_update_period;
                m_pan_change = false;
            }

            // Set output channel every tick
            for (int c = 0; c < N_CH; c++)
                out[f * N_CH + c] = m_gain_cur[c] * in[f];

            // gain interpolation
            if (m_samples_left > 0) {
                for (int c = 0; c < N_CH; c++)
                    m_gain_cur[c] += m_gain_step[c];
                m_samples_left--;

                // if finished interpolating, set step size to 0 so we don't blow up the gain
                if (m_samples_left == 0) {
                    for (int c = 0; c < N_CH; c++) {
                        m_gain_cur[c]  = m_gain_next[c];
                        m_gain_step[c] = 0;
                    }
                }
            }
        }
    }

private:
    void compute_coeffs()
    {
        // 1st order — 4 channels
        m_coeffs[0] = 1.;
        m_coeffs[1] = 1.;
        m_coeffs[2] = 1.;
        m_coeffs[3] = 1.;

        // 2nd order — 9 channels
        m_coeffs[4] = sqrt(3);
        m_coeffs[5] = (1. / 4.) * sqrt(3);
        m_coeffs[6] = (3. / 2.);
        m_coeffs[7] = (1. / 4.) * sqrt(3);
        m_coeffs[8] = (1. / 2.) * sqrt(3);

        // 3rd order — 16 channels
        m_coeffs[9]  = (1. / 4.) * sqrt(10);
        m_coeffs[10] = sqrt(15);
        m_coeffs[11] = (1. / 4.) * sqrt(6);
        m_coeffs[12] = (1. / 2.);
        m_coeffs[13] = (1. / 4.) * sqrt(6);
        m_coeffs[14] = (1. / 2.) * sqrt(15);
        m_coeffs[15] = (1. / 4.) * sqrt(10);

        // 4th order — 25 channels
        m_coeffs[16] = (1. / 32.) * sqrt(35);
        m_coeffs[17] = (1. / 4.) * sqrt(70);
        m_coeffs[18] = (1. / 2.) * sqrt(5);
        m_coeffs[19] = (1. / 4.) * sqrt(10);
        m_coeffs[20] = (35. / 8.);
        m_coeffs[21] = (1. / 4.) * sqrt(10);
        m_coeffs[22] = (1. / 4.) * sqrt(5);
        m_coeffs[23] = (1. / 4.) * sqrt(70);
        m_coeffs[24] = sqrt(35);

        // 5th order — 36 channels
        m_coeffs[25] = (3. / 16.) * sqrt(14);
        m_coeffs[26] = (3. / 64.) * sqrt(35);
        m_coeffs[27] = (-1. / 16.) * sqrt(70);
        m_coeffs[28] = (1. / 2.) * sqrt(105);
        m_coeffs[29] = (1. / 8.) * sqrt(15);
        m_coeffs[30] = (1. / 8.);
        m_coeffs[31] = (1. / 8.) * sqrt(15);
        m_coeffs[32] = (1. / 4.) * sqrt(105);
        m_coeffs[33] = (-1. / 16.) * sqrt(70);
        m_coeffs[34] = (3. / 8.) * sqrt(35);
        m_coeffs[35] = (3. / 128.) * sqrt(14);

        // 6th order — 49 channels
        m_coeffs[36] = (1. / 16.) * sqrt(462);
        m_coeffs[37] = (3. / 16.) * sqrt(154);
        m_coeffs[38] = (3. / 256.) * sqrt(7);
        m_coeffs[39] = (-1. / 16.) * sqrt(210);
        m_coeffs[40] = (1. / 16.) * sqrt(210);
        m_coeffs[41] = (1. / 8.) * sqrt(21);
        m_coeffs[42] = (231. / 16.);
        m_coeffs[43] = (1. / 8.);
        m_coeffs[44] = (1. / 32.) * sqrt(210);
        m_coeffs[45] = (-1. / 16.) * sqrt(210);
        m_coeffs[46] = (3. / 16.) * sqrt(7);
        m_coeffs[47] = (3. / 256.) * sqrt(154);
        m_coeffs[48] = (1. / 256.) * sqrt(462);

        // 7th order — 64 channels
        m_coeffs[49] = (1. / 32.) * sqrt(429);
        m_coeffs[50] = (1. / 512.) * sqrt(6006);
        m_coeffs[51] = (1. / 32.) * sqrt(231);
        m_coeffs[52] = (1. / 512.) * sqrt(231);
        m_coeffs[53] = (-1. / 32.) * sqrt(21);
        m_coeffs[54] = (1. / 16.) * sqrt(42);
        m_coeffs[55] = (1. / 32.) * sqrt(7);
        m_coeffs[56] = (1. / 16.);
        m_coeffs[57] = (1. / 32.) * sqrt(7);
        m_coeffs[58] = (1. / 32.) * sqrt(42);
        m_coeffs[59] = (-1. / 32.) * sqrt(21);
        m_coeffs[60] = (1. / 16.) * sqrt(231);
        m_coeffs[61] = (1. / 32.) * sqrt(231);
        m_coeffs[62] = (1. / 512.) * sqrt(6006);
        m_coeffs[63] = (1. / 32.) * sqrt(429);
    }

    void compute_gains()
    {
        // azimuth repeated expressions
        t_CKFLOAT sinA = sinf(m_azimuth);
        t_CKFLOAT cosA = cosf(m_azimuth);

        t_CKFLOAT sinA2 = sinA * sinA;
        t_CKFLOAT sinA4 = sinA2 * sinA2;
        t_CKFLOAT sinA6 = sinA4 * sinA2;

        t_CKFLOAT cosA2 = cosA * cosA;
        t_CKFLOAT cosA4 = cosA2 * cosA2;
        t_CKFLOAT cosA6 = cosA4 * cosA2;

        t_CKFLOAT sin2A = 2 * sinA * cosA;
        t_CKFLOAT cos2A = cosA2 - sinA2;

        t_CKFLOAT sin3A = 2 * cosA * sin2A - sinA;
        t_CKFLOAT cos3A = 2 * cosA * cos2A - cosA;

        t_CKFLOAT sin4A = 2 * cosA * sin3A - sin2A;
        t_CKFLOAT cos4A = 2 * cosA * cos3A - cos2A;

        t_CKFLOAT sin5A = 2 * cosA * sin4A - sin3A;
        t_CKFLOAT cos5A = 2 * cosA * cos4A - cos3A;

        t_CKFLOAT sin6A = 2 * cosA * sin5A - sin4A;
        t_CKFLOAT cos6A = 2 * cosA * cos5A - cos4A;

        // elevation repeated expressions
        t_CKFLOAT sinE = sinf(m_elevation);
        t_CKFLOAT cosE = cosf(m_elevation);

        t_CKFLOAT sinE2 = sinE * sinE;
        t_CKFLOAT sinE4 = sinE2 * sinE2;
        t_CKFLOAT sinE6 = sinE4 * sinE2;

        t_CKFLOAT cosE2 = cosE * cosE;
        t_CKFLOAT cosE3 = cosE2 * cosE;
        t_CKFLOAT cosE4 = cosE2 * cosE2;
        t_CKFLOAT cosE6 = cosE4 * cosE2;
        t_CKFLOAT cosE7 = cosE6 * cosE;

        t_CKFLOAT sin2E = 2 * sinE * cosE;
        t_CKFLOAT cos2E = cosE2 - sinE2;

        t_CKFLOAT sin3E = 2 * cosE * sin2E - sinE;
        t_CKFLOAT cos3E = 2 * cosE * cos2E - cosE;

        t_CKFLOAT cos2E_12 = (cos2E + 1) * (cos2E + 1);
        t_CKFLOAT cos2E_13 = cos2E_12 * (cos2E + 1);

        // compute gains based on the order
        // 1st order — 4 channels
        if (m_order >= 1) {
            m_gain_next[0] = 1.;
            m_gain_next[1] = sinA * cosE;
            m_gain_next[2] = sinE;
            m_gain_next[3] = cosE * cosA;
        }

        // 2nd order — 9 channels
        if (m_order >= 2) {
            m_gain_next[4] = m_coeffs[4] * sinA * cosE2 * cosA;
            m_gain_next[5] = m_coeffs[5] * 2 * sin2E * sinA;
            m_gain_next[6] = m_coeffs[6] * sinE2 - 0.5;
            m_gain_next[7] = m_coeffs[7] * 2 * sin2E * cosA;
            m_gain_next[8] = m_coeffs[8] * cosE2 * cos2A;
        }

        // 3rd order — 16 channels
        if (m_order >= 3) {
            m_gain_next[9]  = m_coeffs[9]  * (3 - 4 * sinA2) * sinA * cosE3;
            m_gain_next[10] = m_coeffs[10] * sinE * sinA * cosE2 * cosA;
            m_gain_next[11] = m_coeffs[11] * (5 * sinE2 - 1) * sinA * cosE;
            m_gain_next[12] = m_coeffs[12] * (5 * sinE2 - 3) * sinE;
            m_gain_next[13] = m_coeffs[13] * (5 * sinE2 - 1) * cosE * cosA;
            m_gain_next[14] = m_coeffs[14] * sinE * cosE2 * cos2A;
            m_gain_next[15] = m_coeffs[15] * (1 - 4 * sinA2) * cosE3 * cosA;
        }

        // 4th order — 25 channels
        if (m_order >= 4) {
            m_gain_next[16] = m_coeffs[16] * cos2E_12 * sin4A;
            m_gain_next[17] = m_coeffs[17] * (3 - 4 * sinA2) * sinE * sinA * cosE3;
            m_gain_next[18] = m_coeffs[18] * (7 * sinE2 - 1) * sinA * cosE2 * cosA;
            m_gain_next[19] = m_coeffs[19] * (7 * sinE2 - 3) * sinE * sinA * cosE;
            m_gain_next[20] = m_coeffs[20] * sinE4 - 3.75 * sinE2 + 0.375;
            m_gain_next[21] = m_coeffs[21] * (7 * sinE2 - 3) * sinE * cosE * cosA;
            m_gain_next[22] = m_coeffs[22] * (7 * sinE2 - 1) * cosE2 * cos2A;
            m_gain_next[23] = m_coeffs[23] * (1 - 4 * sinA2) * sinE * cosE3 * cosA;
            m_gain_next[24] = m_coeffs[24] * (sinA4 - sinA2 + 0.125) * cosE4;
        }

        // 5th order — 36 channels
        if (m_order >= 5) {
            m_gain_next[25] = m_coeffs[25] * (16 * sinA4 - 20 * sinA2 + 5) * sinA * cosE3;
            m_gain_next[26] = m_coeffs[26] * cos2E_12 * 2 * sinE * sin4A;
            m_gain_next[27] = m_coeffs[27] * (9 * sinE2 - 1) * (4 * sinA2 - 3) * sinA * cosE3;
            m_gain_next[28] = m_coeffs[28] * (3 * sinE2 - 1) * sinE * sinA * cosE2 * cosA;
            m_gain_next[29] = m_coeffs[29] * (21 * sinE4 - 14 * sinE2 + 1) * sinA * cosE;
            m_gain_next[30] = m_coeffs[30] * (63 * sinE4 - 70 * sinE2 + 15) * sinE;
            m_gain_next[31] = m_coeffs[31] * (21 * sinE4 - 14 * sinE2 + 1) * cosE * cosA;
            m_gain_next[32] = m_coeffs[32] * (3 * sinE2 - 1) * sinE * cosE2 * cos2A;
            m_gain_next[33] = m_coeffs[33] * (9 * sinE2 - 1) * (4 * sinA2 - 1) * cosE3 * cosA;
            m_gain_next[34] = m_coeffs[34] * (8 * sinA4 - 8 * sinA2 + 1) * sinE * cosE4;
            m_gain_next[35] = m_coeffs[35] * cos2E_12 * 2 * cosE * cos5A;
        }

        // 6th order — 49 channels
        if (m_order >= 6) {
            m_gain_next[36] = m_coeffs[36] * (16 * sinA4 - 16 * sinA2 + 3) * sinA * cosE6 * cosA;
            m_gain_next[37] = m_coeffs[37] * (16 * sinA4 - 20 * sinA2 + 5) * sinE * sinA * cosE3;
            m_gain_next[38] = m_coeffs[38] * cos2E_12 * sin4A * (18 - 22 * cos2E);
            m_gain_next[39] = m_coeffs[39] * (11 * sinE2 - 3) * (4 * sinA2 - 3) * sinE * sinA * cosE3;
            m_gain_next[40] = m_coeffs[40] * (33 * sinE4 - 18 * sinE2 + 1) * sinA * cosE2 * cosA;
            m_gain_next[41] = m_coeffs[41] * (33 * sinE4 - 30 * sinE2 + 5) * sinE * sinA * cosE;
            m_gain_next[42] = m_coeffs[42] * sinE6 - 19.6875 * sinE4 + 6.5625 * sinE2 - 0.3125;
            m_gain_next[43] = m_coeffs[43] * 4.58257569496 * (33 * sinE4 - 30 * sinE2 + 5) * sinE * cosE * cosA;
            m_gain_next[44] = m_coeffs[44] * (33 * sinE4 - 18 * sinE2 + 1) * cosE2 * cos2A;
            m_gain_next[45] = m_coeffs[45] * (11 * sinE2 - 3) * (4 * sinA2 - 1) * sinE * cosE3 * cosA;
            m_gain_next[46] = m_coeffs[46] * (11 * sinE2 - 1) * (8 * sinA4 - 8 * sinA2 + 1) * cosE4;
            m_gain_next[47] = m_coeffs[47] * 2 * sin2E * cos5A * cos2E_12;
            m_gain_next[48] = m_coeffs[48] * cos2E_13 * cos6A;
        }

        // 7th order — 64 channels
        if (m_order >= 7) {
            m_gain_next[49] = m_coeffs[49] * (-57 * sinA6 + 91 * sinA4 - 35 * sinA2 + 7 * cosA6) * sinA * cosE7;
            m_gain_next[50] = m_coeffs[50] * cos2E_13 * (2 * sinE * sin6A);
            m_gain_next[51] = m_coeffs[51] * (13 * sinE2 - 1) * (16 * sinA4 - 20 * sinA2 + 5) * sinA * cosE3;
            m_gain_next[52] = m_coeffs[52] * cos2E_12 * sin4A * (54 * sinE - 26 * sin3E);
            m_gain_next[53] = m_coeffs[53] * (4 * sinA2 - 3) * (143 * sinE4 - 66 * sinE2 + 3) * sinA * cosE3;
            m_gain_next[54] = m_coeffs[54] * (143 * sinE4 - 110 * sinE2 + 15) * sinE * sinA * cosE2 * cosA;
            m_gain_next[55] = m_coeffs[55] * (429 * sinE6 - 495 * sinE4 + 135 * sinE2 - 5) * sinA * cosE;
            m_gain_next[56] = m_coeffs[56] * (429 * sinE6 - 693 * sinE4 + 315 * sinE2 - 35) * sinE;
            m_gain_next[57] = m_coeffs[57] * (429 * sinE6 - 495 * sinE4 + 135 * sinE2 - 5) * cosE * cosA;
            m_gain_next[58] = m_coeffs[58] * (143 * sinE4 - 110 * sinE2 + 15) * sinE * cosE2 * cos2A;
            m_gain_next[59] = m_coeffs[59] * (4 * sinA2 - 1) * (143 * sinE4 - 66 * sinE2 + 3) * cosE3 * cosA;
            m_gain_next[60] = m_coeffs[60] * (13 * sinE2 - 3) * (8 * sinA4 - 8 * sinA2 + 1) * sinE * cosE4;
            m_gain_next[61] = m_coeffs[61] * (13 * sinE2 - 1) * (16 * sinA4 - 12 * sinA2 + 1) * cosE3 * cosA;
            m_gain_next[62] = m_coeffs[62] * (2 * sinE * cos6A) * cos2E_13;
            m_gain_next[63] = m_coeffs[63] * (-63 * sinA6 + 77 * sinA4 - 21 * sinA2 + cosA6) * cosE7 * cosA;
        }
    }

    // helper functions
    float scalef( float x, float in_min, float in_max, float out_min, float out_max )
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // instance data
    t_CKINT   m_order;
    t_CKINT   m_out_channels;
    t_CKINT   m_update_period;
    t_CKINT   m_samples_left;
    t_CKINT   m_pan_change;
    t_CKINT   m_bounds_type;

    t_CKFLOAT m_azimuth;
    t_CKFLOAT m_elevation;

    t_CKFLOAT m_coeffs[MAX_CHANNELS];
    t_CKFLOAT m_gain_next[MAX_CHANNELS];
    t_CKFLOAT m_gain_cur[MAX_CHANNELS];
    t_CKFLOAT m_gain_step[MAX_CHANNELS];
};


// functions that are the same for each order
static void ambienc_setAzimuth( Chuck_Object * SELF, t_CKINT off, void * ARGS, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    RETURN->v_float = obj->setAzimuth(GET_NEXT_FLOAT(ARGS));
}

static void ambienc_getAzimuth( Chuck_Object * SELF, t_CKINT off, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    RETURN->v_float = obj->getAzimuth();
}

static void ambienc_setElevation( Chuck_Object * SELF, t_CKINT off, void * ARGS, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    RETURN->v_float = obj->setElevation(GET_NEXT_FLOAT(ARGS));
}

static void ambienc_getElevation( Chuck_Object * SELF, t_CKINT off, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    RETURN->v_float = obj->getElevation();
}

static void ambienc_pan( Chuck_Object * SELF, t_CKINT off, void * ARGS, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    t_CKFLOAT a = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT e = GET_NEXT_FLOAT(ARGS);
    RETURN->v_vec2 = obj->pan(a, e);
}

static void ambienc_setUpdatePeriod( Chuck_Object * SELF, t_CKINT off, void * ARGS, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    RETURN->v_int = obj->setUpdatePeriod(GET_NEXT_INT(ARGS));
}

static void ambienc_getUpdatePeriod( Chuck_Object * SELF, t_CKINT off, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    RETURN->v_int = obj->getUpdatePeriod();
}

static void ambienc_setBoundsType( Chuck_Object * SELF, t_CKINT off, void * ARGS, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    t_CKFLOAT b = GET_NEXT_INT(ARGS);
    RETURN->v_int = obj->setBoundsType(b);
}

static void ambienc_getBoundsType( Chuck_Object * SELF, t_CKINT off, Chuck_DL_Return * RETURN, CK_DL_API API )
{
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, off);
    RETURN->v_int = obj->getBoundsType();
}



// constructors and functions that differ per order
#define DEFINE_ORDER_CALLBACKS(N)                                                                                          \
CK_DLL_CTOR(ambienc##N##_ctor) {                                                                                           \
    OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset) = 0;                                                                    \
    AmbiEnc * obj = new AmbiEnc(N, 64, ambienc_bounds_normalized);                                                         \
    OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset) = (t_CKINT)obj;                                                         \
}                                                                                                                          \
CK_DLL_CTOR(ambienc##N##_ctor_period) {                                                                                    \
    OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset) = 0;                                                                    \
    t_CKINT p = GET_NEXT_INT(ARGS);                                                                                        \
    AmbiEnc * obj = new AmbiEnc(N, p, ambienc_bounds_normalized);                                                          \
    OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset) = (t_CKINT)obj;                                                         \
}                                                                                                                          \
CK_DLL_CTOR(ambienc##N##_ctor_periodAndBounds) {                                                                           \
    OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset) = 0;                                                                    \
    t_CKINT p = GET_NEXT_INT(ARGS); t_CKINT b = GET_NEXT_INT(ARGS);                                                        \
    AmbiEnc * obj = new AmbiEnc(N, p, b);                                                                                  \
    OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset) = (t_CKINT)obj;                                                         \
}                                                                                                                          \
CK_DLL_DTOR(ambienc##N##_dtor) {                                                                                           \
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset);                                             \
    CK_SAFE_DELETE(obj);                                                                                                   \
    OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset) = 0;                                                                    \
}                                                                                                                          \
CK_DLL_TICKF(ambienc##N##_tickf) {                                                                                         \
    AmbiEnc * obj = (AmbiEnc *)OBJ_MEMBER_INT(SELF, ambienc##N##_data_offset);                                             \
    if (obj) obj->tick<(N+1)*(N+1)>(in, out, nframes);                                                                     \
    return TRUE;                                                                                                           \
}                                                                                                                          \
CK_DLL_MFUN(ambienc##N##_setAzimuth)    { ambienc_setAzimuth(SELF, ambienc##N##_data_offset, ARGS, RETURN, API); }         \
CK_DLL_MFUN(ambienc##N##_getAzimuth)    { ambienc_getAzimuth(SELF, ambienc##N##_data_offset, RETURN, API); }               \
CK_DLL_MFUN(ambienc##N##_setElevation)  { ambienc_setElevation(SELF, ambienc##N##_data_offset, ARGS, RETURN, API); }       \
CK_DLL_MFUN(ambienc##N##_getElevation)  { ambienc_getElevation(SELF, ambienc##N##_data_offset, RETURN, API); }             \
CK_DLL_MFUN(ambienc##N##_pan)           { ambienc_pan(SELF, ambienc##N##_data_offset, ARGS, RETURN, API); }                \
CK_DLL_MFUN(ambienc##N##_setUpdatePeriod) { ambienc_setUpdatePeriod(SELF, ambienc##N##_data_offset, ARGS, RETURN, API); }  \
CK_DLL_MFUN(ambienc##N##_getUpdatePeriod) { ambienc_getUpdatePeriod(SELF, ambienc##N##_data_offset, RETURN, API); }        \
CK_DLL_MFUN(ambienc##N##_setBoundsType) { ambienc_setBoundsType(SELF, ambienc##N##_data_offset, ARGS, RETURN, API); }      \
CK_DLL_MFUN(ambienc##N##_getBoundsType) { ambienc_getBoundsType(SELF, ambienc##N##_data_offset, RETURN, API); }

DEFINE_ORDER_CALLBACKS(1)
DEFINE_ORDER_CALLBACKS(2)
DEFINE_ORDER_CALLBACKS(3)
DEFINE_ORDER_CALLBACKS(4)
DEFINE_ORDER_CALLBACKS(5)
DEFINE_ORDER_CALLBACKS(6)
DEFINE_ORDER_CALLBACKS(7)


// register every class / constructor / function per order
CK_DLL_INFO( AmbiEnc )
{
    QUERY->setinfo( QUERY, CHUGIN_INFO_CHUGIN_VERSION, "v0.1.0" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_AUTHORS, "Gregg Oliva" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_DESCRIPTION, "Order-specific Ambisonics encoders (1st–7th order). Uses ACN channel ordering and SN3D normaliztion" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_URL, "" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_EMAIL, "" );
}

#define REGISTER_ORDER_CLASS(N, N_CH)                                                                 \
do {                                                                                                  \
    QUERY->begin_class(QUERY, "AmbiEnc" #N, "UGen");                                                  \
    QUERY->doc_class(QUERY, "Order-" #N " ambisonics encoder. " #N_CH " output channels, ACN/SN3D."); \
    QUERY->add_ctor(QUERY, ambienc##N##_ctor);                                                        \
    QUERY->add_ctor(QUERY, ambienc##N##_ctor_period);                                                 \
        QUERY->add_arg(QUERY, "int", "updatePeriod");                                                 \
    QUERY->add_ctor(QUERY, ambienc##N##_ctor_periodAndBounds);                                        \
        QUERY->add_arg(QUERY, "int", "updatePeriod");                                                 \
        QUERY->add_arg(QUERY, "int", "boundsType");                                                   \
    QUERY->add_dtor(QUERY, ambienc##N##_dtor);                                                        \
    QUERY->add_ugen_funcf(QUERY, ambienc##N##_tickf, NULL, 1, N_CH);                                  \
    QUERY->add_mfun(QUERY, ambienc##N##_setAzimuth, "float", "azimuth");                              \
        QUERY->add_arg(QUERY, "float", "a");                                                          \
    QUERY->add_mfun(QUERY, ambienc##N##_getAzimuth, "float", "azimuth");                              \
    QUERY->add_mfun(QUERY, ambienc##N##_setElevation, "float", "elevation");                          \
        QUERY->add_arg(QUERY, "float", "e");                                                          \
    QUERY->add_mfun(QUERY, ambienc##N##_getElevation, "float", "elevation");                          \
    QUERY->add_mfun(QUERY, ambienc##N##_pan, "vec2", "pan");                                          \
        QUERY->add_arg(QUERY, "float", "a"); QUERY->add_arg(QUERY, "float", "e");                     \
    QUERY->add_mfun(QUERY, ambienc##N##_setUpdatePeriod, "int", "updatePeriod");                      \
        QUERY->add_arg(QUERY, "int", "p");                                                            \
    QUERY->add_mfun(QUERY, ambienc##N##_getUpdatePeriod, "int", "updatePeriod");                      \
    QUERY->add_mfun(QUERY, ambienc##N##_setBoundsType, "int", "boundsType");                          \
        QUERY->add_arg(QUERY, "int", "b");                                                            \
    QUERY->add_mfun(QUERY, ambienc##N##_getBoundsType, "int", "boundsType");                          \
    QUERY->add_svar(QUERY, "int", "NORMALIZED", true, (void *)&ambienc_bounds_normalized);            \
    QUERY->add_svar(QUERY, "int", "RADIANS",    true, (void *)&ambienc_bounds_radians);               \
    ambienc##N##_data_offset = QUERY->add_mvar(QUERY, "int", "@ae" #N "_data", false);                \
    QUERY->end_class(QUERY);                                                                          \
} while(0)

CK_DLL_QUERY( AmbiEnc )
{
    QUERY->setname(QUERY, "AmbiEnc");
    REGISTER_ORDER_CLASS(1,  4);
    REGISTER_ORDER_CLASS(2,  9);
    REGISTER_ORDER_CLASS(3, 16);
    REGISTER_ORDER_CLASS(4, 25);
    REGISTER_ORDER_CLASS(5, 36);
    REGISTER_ORDER_CLASS(6, 49);
    REGISTER_ORDER_CLASS(7, 64);
    return TRUE;
}
