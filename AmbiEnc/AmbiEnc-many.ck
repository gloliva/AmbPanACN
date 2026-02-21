/*
    AmbiEnc3-example.ck

    Generate N number of voices with random pannings.

    Requirements: Patch and Range must be installed via Chump
        ```
        $ chump install Patch
        $ chump install Range
        ```

    How to run (from AmbiEnc3 directory):
        ```
        $ chuck --chugin:./AmbiEnc3.chug --dac:<DEVICE_FOR_AMBISONICS> --out:<NUM_OUTS_NEEDED_FOR_ORDER> tests/AmbiEnc3-example.ck
        ```
*/

@import "Range"
@import "Patch"

// @import "../scratch/AmbiEnc3.ck"

// Read user input
ConsoleInput in;
StringTokenizer tok;



0.5 => dac.gain;

// Rec.auto();


// Prompt for voices
in.prompt("How many voices to create (e.g. 8). Stops working ~20 voices: ") => now;
int numVoices;
string userInput;


while (in.more()) {
    tok.set(in.getLine());

    if (tok.more()) {
        tok.next() => userInput;
        userInput.toInt() => numVoices;
    }
}

// Error checking
if (numVoices < 1) {
    cherr <= "Invalid number of voices provided: \"" <= userInput <= "\"." <= IO.nl();
    me.exit();
}


AmbiEnc3 pans[0];
SawOsc oscs[0];

// Patch patchesA[0];
// Range rangesA[0];
SinOsc lfosA[0];

// Patch patchesE[0];
// Range rangesE[0];
SinOsc lfosGain[0];


UGen mix[64];

repeat (numVoices) {
    // Osc and Amb
    oscs << new SawOsc(Math.mtof(Math.random2(30, 91)));
    pans << new AmbiEnc3(64);
    // pans << new AmbiEnc3(order, mix);

    // LFOs
    lfosA << new SinOsc(Math.random2f(0.05, 2.));
    lfosGain << new SinOsc(Math.random2f(0.001, 0.1));
}

UGen buffer[64];
for (UGen buf : buffer) {
    0.1 => buf.gain;
}


// oscs[0] => pans[0] => dac;
// for (1 => int i; i < numVoices; i++) {
for (0 => int i; i < numVoices; i++) {
    // Connect to dac
    oscs[i] => pans[i] => buffer;
    // oscs[i] => pans[i];
    0. => oscs[i].gain;

    lfosA[i] => blackhole;
    // lfosE[i] => blackhole;
}

buffer => dac;
// mix => dac;


fun void moveAround() {
    while (true) {
        for (int i; i < pans.size(); i++) {
            Std.scalef(lfosA[i].last(), -1., 1., -pi, pi) => pans[i].azimuth;
        }

        for (int i; i < pans.size(); i++) {
            Std.scalef(lfosA[i].last(), -1., 1., -pi, pi) => pans[i].elevation;
        }

        for (pans.size() -1 => int i; i >= 0; i--) {
            Std.scalef(lfosGain[i].last(), -1., 1., 0, 0.05) => oscs[i].gain;
        }

        10::ms => now;
    }
}

spork ~ moveAround();


1 => int running;
KBHit kb;

chout <= IO.nl() <= "Press ESC to quit." <= IO.nl();
while (running) {
    kb => now;

    while (kb.more()) {
         if (kb.getchar() == 27) {
            0 => running;
        }
    }
}
