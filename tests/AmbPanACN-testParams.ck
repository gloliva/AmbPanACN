/*
    AmbPanACN-testParams.ck

    Test basic parameter setting and retrieving.

    How to run (from AmbPanACN directory):
        ```
        $ chuck --chugin:./AmbPanACN.chug tests/AmbPanACN-testParams.ck
        ```
*/

// Define assert function
fun int assert(float actual, float expected) {
    if (actual != expected) {
        cherr <= "FAILED, " <= actual <= " != " <= expected <= IO.nl();
        return 0;
    }

    return 1;
}

// Read user input
ConsoleInput in;
StringTokenizer tok;

// Prompt for ambisonics order
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

// Prompt for updateSamples
in.prompt("Number of samples per update (e.g. a power of 2, like 64): ") => now;
int updatePeriod;


while (in.more()) {
    tok.set(in.getLine());

    if (tok.more()) {
        tok.next() => userInput;
        userInput.toInt() => updatePeriod;
    }
}

// Error checking
if (updatePeriod < 1) {
    cherr <= "Invalid sample period provided: \"" <= userInput <= "\". Must be greater than 0." <= IO.nl();
    me.exit();
}
chout <= IO.nl();

// instantiate a AmbPanACN
AmbPanACN amb(order, updatePeriod, AmbPanACN.RADIANS);

chout <= "Printing out amb.help() function" <= IO.nl();
amb.help();
chout <= IO.nl();


// Set azimuth and elevation
pi / 2. => amb.azimuth;
pi / 4. => amb.elevation;


// Test and print out parameters
chout <= "Printing out amb parameters" <= IO.nl();
assert(amb.order(), order);
<<< "    Order:", amb.order() >>>;

assert(amb.outChannels(), Math.pow(order+1, 2));
<<< "    Out channels:", amb.outChannels() >>>;

assert(amb.channels(), 64);
<<< "    Total channels:", amb.channels() >>>;

assert(amb.azimuth(), pi / 2.);
<<< "    Azimuth:", amb.azimuth() >>>;

assert(amb.elevation(), pi / 4.);
<<< "    Elevation:", amb.elevation() >>>;

assert(amb.updatePeriod(), updatePeriod);
<<< "    Update period:", amb.updatePeriod() >>>;

// Test static variables
assert(amb.NORMALIZED, 0);
<<< "    Normalized static variable:", AmbPanACN.NORMALIZED >>>;

assert(amb.RADIANS, 1);
<<< "    Radians static variable:", AmbPanACN.RADIANS >>>;

// Test return values for pan
amb.pan(pi, pi / 2.) => vec2 ret;
assert(ret.x, pi);
assert(ret.y, pi / 2.);

// Change order and test
for (1 => int order; order < 8; order++) {
    order => amb.order;
    assert(amb.order(), order);
    assert(amb.outChannels(), Math.pow(order+1, 2));
}
