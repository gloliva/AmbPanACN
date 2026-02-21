//---------------------------------------------------------------------
// name: AmbiPan-examplePath.ck
// desc: demo for the path function
//
// amb.path(float azi_init, float ele_init, float azi_final, float ele_final, dur path_time)
// Smoothly interpolates between starting and ending locations over set path time.
// If you change anything about the panner attributes, i.e.
// azimuth, elevation, azimuth velocity, elevation velocity,
// the panner abandons the path function.
//
// author: Zac Dulkin
//         Gregg Oliva
// date: 2/19/2026
//---------------------------------------------------------------------

5 => int order;
1 => int normalized;

// range for random path time in seconds
0.1 => float path_time_low;
2 => float path_time_high;

// range for random azimuth positions + random number of possible extra revolutions
-1 => float azimuth_low;
1 => float azimuth_high;
2 => int max_revolutions;

// range for random elevation positions
0 => float elevation_low;
0.5 => float elevation_high;

Noise noise => Gain g(0.2) => AmbiPan amb(order, normalized) => dac;

fun void monitor(dur durian) {
    while (true) {
        <<<amb.azimuth(), amb.elevation()>>>;
        durian => now;
    }
} spork ~ monitor(100::ms);

// set random starting & ending positions, set random path time, smoothly pan from start to finish
fun void random_path() {
    while (true) {
        Math.random2f(path_time_low, path_time_high) => float path_time;

        Math.random2f(azimuth_low, azimuth_high) => float azi_init;
        Math.random2f(azimuth_low, azimuth_high) => float azi_next;
        2 * Math.random2(0, max_revolutions) +=> azi_next;
        
        Math.random2f(elevation_low, elevation_high) => float ele_init;
        Math.random2f(elevation_low, elevation_high) => float ele_next;

        amb.path(azi_init, ele_init, azi_next, ele_next, path_time::second);
        <<<"\n", "new path:", azi_init, ele_init, azi_next, ele_next, path_time + " sec", "\n">>>;

        <<<amb.azimuth(), amb.elevation()>>>;
        path_time::second => now;

        <<<amb.azimuth(), amb.elevation()>>>;
        samp => now;
    }
} spork ~ random_path();

week => now;
