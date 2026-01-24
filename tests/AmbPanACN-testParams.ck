/*
    AmbPanACN-testParams.ck

    Test basic parameter setting and retrieving.

    How to run (from AmbPanACN directory):
        ```
        $ chuck --chugin:./AmbPanACN.chug tests/AmbPanACN-testParams.ck
        ```
*/

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
AmbPanACN amb(order);

// Set azimuth and elevation
pi / 2. => amb.azimuth;
pi / 4. => amb.elevation;

// Print out parameters
<<< "Order:", amb.order() >>>;
<<< "Out channels:", amb.outChannels() >>>;
<<< "Total channels:", amb.channels() >>>;
<<< "Azimuth:", amb.azimuth() >>>;
<<< "Elevation:", amb.elevation() >>>;
