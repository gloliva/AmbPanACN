@import "Chumpinate"

// get os from command line args
if (me.args() != 1) {
    chout <= "Please specify an os to package (mac, win, linux)." <= IO.nl();
    me.exit();
}

me.arg(0) => string os;

if (!(os == "mac" | os == "win" | os == "linux")) {
    chout <= "Invalid os \"" <= os <= "\". Must be one of: (mac, win, linux)."
    me.exit();
}

// instantiate a Chumpinate package
Package pkg("AmbiPan");

// add our metadata here
"0.1.0" => string VERSION;
[
    "Zac Dulkin",
    "Gregg Oliva"
] => pkg.authors;

"https://github.com/gloliva/AmbiPan/tree/master" => pkg.homepage;
"https://github.com/gloliva/AmbiPan/tree/master" => pkg.repository;

"An ambisonics panner that supports up to 7th order." => pkg.description;
"MIT" => pkg.license;

["ambisonics", "spatial audio", "encoder"] => pkg.keywords;

// generate a package-definition json file,
// this will be stored in (./AwesomeEffect/package.json)
"./chump" => pkg.generatePackageDefinition;

// Now we need to define a specific PackageVersion and all the associated files and metadata
PackageVersion ver("AmbiPan", VERSION);

// what is the oldest version of ChucK this package will run on?
"1.5.5.0" => ver.languageVersionMin;

// Because this is a ChuGin, which is a complied binary,
// this package must be built for specific operating systems and CPU architectures.
if (os == "mac") {
    "mac" => ver.os;
    "universal" => ver.arch;
} else {
    os => ver.os;
    "x86_64" => ver.arch;
}

// add our package's files
if (os == "windows") {
    ver.addFile("x64/Release/AmbiPan.chug");
} else {
    ver.addFile("AmbiPan.chug");
}

// add example files, this will be stored in the package's `_examples` directory.
ver.addExampleFile("examples/AmbiPan-example3Voices.ck");
ver.addExampleFile("examples/AmbiPan-exampleNVoices.ck");

// zip up all our files into AwesomeEffect.zip, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./chump", "HashMap", "https://ccrma.stanford.edu/~gloliva/ambisonics/releases/" + VERSION + "/AmbiPan-" + os + ".zip");

// Generate a version definition json file, stores this in "AwesomeEffect/<VerNo>/version-<os>.json"
ver.generateVersionDefinition("AmbiPan-" + os, "./chump");
