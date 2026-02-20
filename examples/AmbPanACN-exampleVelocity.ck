//---------------------------------------------------------------------
// name: AmbPanACN-exampleVelocity.ck
// desc: demo for the velocity parameters
//
// set with
// amb.aziVelocity(float a_v)
// amb.eleVelocity(float e_v)
// amb.setVelocities(float a_v, e_v)
//
// velocity is measured in revolutions / radians per second:
// if you set amb.aziVelocity(2) when the panner is in normalized mode
// (i.e. panning goes from [-1, 1] instead of [-pi, pi]), then the panner
// will rotate twice around the field every second.
//  
// velocities are set to 0 after a new amb.pan() call: circumvent by calling
// amb.set(newAzi, newEle, amb.azimuth(), amb.elevation())
//
// author: Zac Dulkin
//         Gregg Oliva
// date: 2/19/2026
//---------------------------------------------------------------------

5 => int order;
1 => int normalized;

Noise noise => Gain g(0.2) => AmbPanACN amb(order, normalized) => dac;
amb.pan(0, 0);

while (true) {
    amb.aziVelocity(Math.random2f(-1, 1));
    amb.eleVelocity(Math.random2f(-0.1, 0.1));
    
    <<<"\n", "new velocity:", amb.aziVelocity(), amb.eleVelocity(), "\n" >>>; 

    repeat (Math.random2(5, 20)) {
        <<<amb.azimuth(), amb.elevation()>>>;
        100::ms => now;
    }
}