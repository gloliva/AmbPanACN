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
    "Gregg Oliva"
    "Zac Dulkin",
] => pkg.authors;

"https://github.com/gloliva/Chambisonics/tree/master" => pkg.homepage;
"https://github.com/gloliva/Chambisonics/tree/master" => pkg.repository;

"Ambisonics tools for ChucK" => pkg.description;
"MIT" => pkg.license;

["ambisonics", "spatial audio", "encoders", "decoders"] => pkg.keywords;

// generate a package-definition json file,
// this will be stored in (./AwesomeEffect/package.json)
"./chump" => pkg.generatePackageDefinition;

// Now we need to define a specific PackageVersion and all the associated files and metadata
PackageVersion ver("Chambisonics", VERSION);

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
    ver.addFile("AmbiPan/AmbiPan.chug");
    ver.addFile("AmbiEnc/AmbiEnc.chug");
    ver.addFile("AmbiBin/AmbiBin.chug");
}

// add example files, this will be stored in the package's `_examples` directory.
// TODO: add examples here

// zip up all our files into AwesomeEffect.zip, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./chump", "HashMap", "https://ccrma.stanford.edu/~gloliva/chambisonics/releases/" + VERSION + "/Chambisonics-" + os + ".zip");

// Generate a version definition json file, stores this in "AwesomeEffect/<VerNo>/version-<os>.json"
ver.generateVersionDefinition("Chambisonics-" + os, "./chump");
