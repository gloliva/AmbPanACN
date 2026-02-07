/*
    AmbPanACN-testBasic.ck

    Test basic automation of azimuth panning.

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


// instantiate a AmbPanACN
CNoise noise => AmbPanACN amb(order) => dac;
"pink" => noise.mode;

chout <= "Dac channels: " <= dac.channels() <= IO.nl();
chout <= "Ambisonic order: " <= amb.order() <= ". Number of out channels: " <= amb.outChannels() <= IO.nl();


// Set phasor to control azimuth
Phasor phase(0.25) => Range r(0, 1, -1., 1.) => Patch p(amb, "azimuth") => blackhole;

// Wait forever
eon => now;
