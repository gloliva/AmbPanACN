// AmbiBin.cpp
// 1st through 7th order Ambisonics Binaural Decoders
// Very simple virtual loudspeaker dome; not the most accurate / effective but can be useful for testing / keeping everything in chuck

#include "chugin.h"
#include <cmath>


// precomputed matrix calculations for a basic virtual dome
static const float dec_L[7][64] = {
    // Order 1 (4 ch)
    { 0.50000000f, 0.17727273f, 0.12085662f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 2 (9 ch)
    { 0.50000000f, 0.18145161f, 0.11016334f, -0.00000000f, 0.00000000f, 0.03083514f, -0.04435484f, -0.00000000f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 3 (16 ch)
    { 0.50000000f, 0.18303571f, 0.10788307f, -0.00000000f, 0.00000000f, 0.03108974f, -0.04910714f, 0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.01982009f, 0.01813472f, -0.00000000f, -0.00000000f, 0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 4 (25 ch)
    { 0.50000000f, 0.18407012f, 0.10482526f, -0.00000000f, -0.00000000f, 0.03127638f, -0.05221037f, -0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, -0.01907830f, 0.01453813f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, 0.00681235f, 0.00231040f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 5 (36 ch)
    { 0.50000000f, 0.18464467f, 0.10426101f, -0.00000000f, -0.00000000f, 0.03136531f, -0.05393401f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.01981662f, 0.01371717f, -0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00719272f, 0.00269670f, 0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00028799f, -0.00675368f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 6 (49 ch)
    { 0.50000000f, 0.18510638f, 0.10271275f, -0.00000000f, -0.00000000f, 0.03145100f, -0.05531915f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.01929950f, 0.01192154f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00691091f, -0.00016622f, -0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00096567f, -0.00774725f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00316761f, 0.00257646f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 7 (64 ch)
    { 0.50000000f, 0.18540112f, 0.10255810f, -0.00000000f, 0.00000000f, 0.03149487f, -0.05620336f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.01981499f, 0.01164026f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00717838f, 0.00042269f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00010585f, -0.00878976f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00325209f, 0.00012207f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, -0.00179410f, -0.01801540f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f },
};

static const float dec_R[7][64] = {
    // Order 1 (4 ch)
    { 0.50000000f, -0.17727273f, 0.12085662f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 2 (9 ch)
    { 0.50000000f, -0.18145161f, 0.11016334f, -0.00000000f, 0.00000000f, -0.03083514f, -0.04435484f, -0.00000000f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 3 (16 ch)
    { 0.50000000f, -0.18303571f, 0.10788307f, -0.00000000f, 0.00000000f, -0.03108974f, -0.04910714f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, 0.01982009f, 0.01813472f, 0.00000000f, 0.00000000f, 0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 4 (25 ch)
    { 0.50000000f, -0.18407012f, 0.10482526f, -0.00000000f, 0.00000000f, -0.03127638f, -0.05221037f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.01907830f, 0.01453813f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00681235f, 0.00231040f, -0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 5 (36 ch)
    { 0.50000000f, -0.18464467f, 0.10426101f, -0.00000000f, 0.00000000f, -0.03136531f, -0.05393401f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.01981662f, 0.01371717f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, -0.00719272f, 0.00269670f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00028799f, -0.00675368f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 6 (49 ch)
    { 0.50000000f, -0.18510638f, 0.10271275f, -0.00000000f, 0.00000000f, -0.03145100f, -0.05531915f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.01929950f, 0.01192154f, 0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00691091f, -0.00016622f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00096567f, -0.00774725f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, -0.00316761f, 0.00257646f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    // Order 7 (64 ch)
    { 0.50000000f, -0.18540112f, 0.10255810f, -0.00000000f, 0.00000000f, -0.03149487f, -0.05620336f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.01981499f, 0.01164026f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, -0.00717838f, 0.00042269f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00010585f, -0.00878976f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00325209f, 0.00012207f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, -0.00000000f, 0.00000000f, -0.00000000f, -0.00000000f, -0.00000000f, 0.00000000f, 0.00179410f, -0.01801540f, -0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f, 0.00000000f },
};


// declaration of chugin functions
#define DECLARE_ORDER_FUNCS(N)           \
    CK_DLL_CTOR(ambibin##N##_ctor);      \
    CK_DLL_DTOR(ambibin##N##_dtor);      \
    CK_DLL_TICKF(ambibin##N##_tickf);    \
    t_CKINT ambibin##N##_data_offset = 0;

DECLARE_ORDER_FUNCS(1)
DECLARE_ORDER_FUNCS(2)
DECLARE_ORDER_FUNCS(3)
DECLARE_ORDER_FUNCS(4)
DECLARE_ORDER_FUNCS(5)
DECLARE_ORDER_FUNCS(6)
DECLARE_ORDER_FUNCS(7)


// class definition for internal chugin data
class AmbiBin
{
public:
    AmbiBin( t_CKINT order )
        : m_order(order), m_order_idx(order - 1) {}

    template<int N_CH>
    void tick( SAMPLE * in, SAMPLE * out, int nframes )
    {
        const float * dL = dec_L[m_order_idx];
        const float * dR = dec_R[m_order_idx];
        for (int f = 0; f < nframes; f++) {
            float L = 0.f, R = 0.f;
            for (int c = 0; c < N_CH; c++) {
                float s = in[f * N_CH + c];
                L += dL[c] * s;
                R += dR[c] * s;
            }
            out[f * 2 + 0] = L;
            out[f * 2 + 1] = R;
        }
    }

private:
    t_CKINT m_order;
    t_CKINT m_order_idx;
};


// constructors and functions for each order
#define DEFINE_ORDER_CALLBACKS(N)                                                \
CK_DLL_CTOR(ambibin##N##_ctor) {                                                 \
    OBJ_MEMBER_INT(SELF, ambibin##N##_data_offset) = 0;                          \
    AmbiBin * obj = new AmbiBin(N);                                              \
    OBJ_MEMBER_INT(SELF, ambibin##N##_data_offset) = (t_CKINT)obj;               \
}                                                                                \
CK_DLL_DTOR(ambibin##N##_dtor) {                                                 \
    AmbiBin * obj = (AmbiBin *)OBJ_MEMBER_INT(SELF, ambibin##N##_data_offset);   \
    CK_SAFE_DELETE(obj);                                                         \
    OBJ_MEMBER_INT(SELF, ambibin##N##_data_offset) = 0;                          \
}                                                                                \
CK_DLL_TICKF(ambibin##N##_tickf) {                                               \
    AmbiBin * obj = (AmbiBin *)OBJ_MEMBER_INT(SELF, ambibin##N##_data_offset);   \
    if (obj) obj->tick<(N+1)*(N+1)>(in, out, nframes);                           \
    return TRUE;                                                                 \
}

DEFINE_ORDER_CALLBACKS(1)
DEFINE_ORDER_CALLBACKS(2)
DEFINE_ORDER_CALLBACKS(3)
DEFINE_ORDER_CALLBACKS(4)
DEFINE_ORDER_CALLBACKS(5)
DEFINE_ORDER_CALLBACKS(6)
DEFINE_ORDER_CALLBACKS(7)


// register every class / constructor / function per order
CK_DLL_INFO( AmbiBin )
{
    QUERY->setinfo( QUERY, CHUGIN_INFO_CHUGIN_VERSION, "v0.1.0" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_AUTHORS, "Gregg Oliva" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_DESCRIPTION, "1st-7th Order Ambisonics binaural decoders using ACN/SN3D ordering and normalization" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_URL, "" );
    QUERY->setinfo( QUERY, CHUGIN_INFO_EMAIL, "" );
}

#define REGISTER_ORDER_CLASS(N, N_CH)                                            \
do {                                                                             \
    QUERY->begin_class(QUERY, "AmbiBin" #N, "UGen");                             \
    QUERY->doc_class(QUERY, "Order-" #N " ambisonics binaural decoder. "         \
        #N_CH " inputs (ACN/SN3D), 2 outputs (L/R headphone).");                 \
    QUERY->add_ctor(QUERY, ambibin##N##_ctor);                                   \
    QUERY->add_dtor(QUERY, ambibin##N##_dtor);                                   \
    QUERY->add_ugen_funcf(QUERY, ambibin##N##_tickf, NULL, N_CH, 2);             \
    ambibin##N##_data_offset =                                                   \
        QUERY->add_mvar(QUERY, "int", "@ab" #N "_data", false);                  \
    QUERY->end_class(QUERY);                                                     \
} while(0)

CK_DLL_QUERY( AmbiBin )
{
    QUERY->setname(QUERY, "AmbiBin");
    REGISTER_ORDER_CLASS(1,  4);
    REGISTER_ORDER_CLASS(2,  9);
    REGISTER_ORDER_CLASS(3, 16);
    REGISTER_ORDER_CLASS(4, 25);
    REGISTER_ORDER_CLASS(5, 36);
    REGISTER_ORDER_CLASS(6, 49);
    REGISTER_ORDER_CLASS(7, 64);
    return TRUE;
}
