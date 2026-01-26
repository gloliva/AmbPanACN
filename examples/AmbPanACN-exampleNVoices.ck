/*
    AmbPanACN-example.ck

    Generate N number of voices with random pannings.

    Requirements: Patch and Range must be installed via Chump
        ```
        $ chump install Patch
        $ chump install Range
        ```

    How to run (from AmbPanACN directory):
        ```
        $ chuck --chugin:./AmbPanACN.chug --dac:<DEVICE_FOR_AMBISONICS> --out:<NUM_OUTS_NEEDED_FOR_ORDER> tests/AmbPanACN-example.ck
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


in.prompt("How many voices to create (e.g. 8). Stops working ~20 voices: ") => now;
int numVoices;


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


AmbPanACN pans[0];
SawOsc oscs[0];

Patch patchesA[0];
Range rangesA[0];
SinOsc lfosA[0];

Patch patchesE[0];
Range rangesE[0];
SinOsc lfosE[0];


repeat (numVoices) {
    // Osc and Amb
    oscs << new SawOsc(Math.mtof(Math.random2(30, 91)));
    pans << new AmbPanACN(order);

    // LFOs
    lfosA << new SinOsc(Math.random2f(0.05, 2.));
    lfosE << new SinOsc(Math.random2f(0.05, 2.));
}


for (int i; i < numVoices; i++) {
    // Connect to dac
    oscs[i] => pans[i] => dac;
    0.5 / numVoices => oscs[i].gain;

    // Set up automation
    patchesA << new Patch(pans[i], "azimuth");
    patchesE << new Patch(pans[i], "elevation");

    rangesA << new Range(Math.random2f(-pi, pi), Math.random2f(-pi, pi));
    rangesE << new Range(Math.random2f(-pi, pi), Math.random2f(-pi, pi));

    lfosA[i] => rangesA[i] => patchesA[i] => blackhole;
    lfosE[i] => rangesE[i] => patchesE[i] => blackhole;
}


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
