/*
    AmbPanACN-testMany.ck

    Test azimuth and elevation automation of three voices. Once running, you can enable/disable each individual voice by pressing
    1, 2, or 3. Press ESC to quit.

    Requirements: Patch and Range must be installed via Chump
        ```
        $ chump install Patch
        $ chump install Range
        ```

    How to run (from AmbPanACN directory):
        ```
        $ chuck --chugin:./AmbPanACN.chug --dac:<DEVICE_FOR_AMBISONICS> --out:<NUM_OUTS_NEEDED_FOR_ORDER> tests/AmbPanACN-testAzimuth.ck
        ```
*/

@import "Range"
@import "Patch"

// Read user input
ConsoleInput in;
StringTokenizer tok;

in.prompt("Ambisonic order to test (e.g. 1 - 7): ") => now;
string userInput;
int order;


while (in.more()) {
    tok.set(in.getLine());

    if (tok.more()) {
        tok.next() => userInput;
        userInput.toInt() => order;
    }
}

// Error checking
if (order < 1 || order > 7) {
    cherr <= "Invalid order provided: \"" <= userInput <= "\". Order must be between 1 and 7" <= IO.nl();
    me.exit();
}


// instantiate AmbPanACNs
PulseOsc osc(Math.mtof(60)) => AmbPanACN amb(order) => dac;
SawOsc osc2(Math.mtof(63)) => AmbPanACN amb2(order) => dac;
SawOsc osc3(Math.mtof(70)) => AmbPanACN amb3(order) => dac;

// Set gains
0.33 => osc.gain;
0.33 => osc2.gain;
0.33 => osc3.gain;


// Handle automation
// Amb1
Phasor phase(0.5) => Range r(0, 1, -pi, pi) => Patch p(amb, "azimuth") => blackhole;

// Amb2
Phasor phase2(0.8) => Range r2(0, 1, pi, -pi) => Patch p2(amb2, "azimuth") => blackhole;
SinOsc lfo(0.1) => Range r3(-1, 1, pi, -pi) => Patch p3(amb2, "elevation") => blackhole;

// Amb3
TriOsc lfo2(0.25) => Range r4(-1, 1, -pi / 2., pi / 2.) => Patch p4(amb3, "azimuth") => blackhole;


// User interaction
chout <= "Press 1, 2, or 3 to toggle each voice On/Off." <= IO.nl();
chout <= "Press ESC to quit." <= IO.nl() <= IO.nl();

[1, 1, 1] @=> int voiceStatus[];
1 => int running;
KBHit kb;


fun void toggleVoice(int idx, UGen osc) {
    if (voiceStatus[idx] == 1) {
        chout <= "Turning off voice " <= idx + 1 <= IO.nl();
        0 => voiceStatus[idx];
        0. => osc.gain;
    } else {
        chout <= "Turning on voice " <= idx + 1 <= IO.nl();
        1 => voiceStatus[idx];
        0.33 => osc.gain;
    }
}


while (running) {
    kb => now;

    while (kb.more()) {
        kb.getchar() => int key;
        if (key == "1".charAt(0)) {
            toggleVoice(0, osc);
        } else if (key == "2".charAt(0)) {
            toggleVoice(1, osc2);
        } else if (key == "3".charAt(0)) {
            toggleVoice(2, osc3);
        } else if (key == 27) {
            0 => running;
        }
    }
}
