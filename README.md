# AmbPanACN

AmbPanACN is an ambisonics chugin (i.e. ChucK plugin) that supports up to 7th order.
It uses ACN ordering and SN3D normalization.

## Installation

Run the appropriate `makefile` for your system (i.e. mac, win, linux).

```code
    $ make mac
```

## How to Run

You will most likely need to explicitly set the output audio device and the number of channels you need. To find this information, run:

```code
    # Example chuck probe output
    $ chuck --probe

    [chuck]: [CoreAudio] driver found 9 audio device(s)...
    [chuck]:
    [chuck]: ------( audio device: 1 )------
    [chuck]: device name = "My Ambisonics Decoder Device"
    [chuck]: probe [success]...
    [chuck]: # output channels = 64
    [chuck]: # input channels  = 64
    [chuck]: # duplex Channels = 64
    [chuck]: default output = NO
    [chuck]: default input = NO
    [chuck]: natively supported data formats:
    [chuck]:   32-bit float
    [chuck]: supported sample rates:
    [chuck]:   8000 Hz
    [chuck]:   16000 Hz
    [chuck]:   44100 Hz
    [chuck]:   48000 Hz
    [chuck]:   88200 Hz
    [chuck]:   96000 Hz
    [chuck]:   176400 Hz
    [chuck]:   192000 Hz
    [chuck]:   352800 Hz
    [chuck]:   384000 Hz
    [chuck]:   705600 Hz
    [chuck]:   768000 Hz
    ...
```

When you have the device number, you can run chuck programs like this:

```code
    $ chuck --chugin:./AmbPanACN.chug --dac:<DEVICE_FOR_AMBISONICS> --out:<NUM_OUTS_NEEDED_FOR_ORDER> program.ck
```

For example, if your device number is `4` and you are working in 5th order ambisonics (which needs 36 outputs), run:

```code
    $ chuck --chugin:./AmbPanACN.chug --dac:4 --out:36 program.ck
```

## How To Use AmbPanACN

`AmbPanACN` is a UGen that can be connected like any other:

```code
    // Send a sine oscillator into a 5th order ambisonics panner, and then to the DAC
    SinOsc osc(440.) => AmbPanACN pan(5) => dac;
```

The azimuth and elevation can be set like so:

```code
    pi / 2. => pan.azimuth;
    pi / 4. => pan.elevation;

    // Set both azimuth and elevation
    pan.set(pi / 2., pi / 4.);
```

For automating these values, one approach is to use the `Patch` and `Range` classes, which can be downloaded with `chump`.

```code
    @import "Range"
    @import "Patch"

    // Ambisonics panner
    SinOsc osc(440.) => AmbPanACN pan(5) => dac;

    // Automate azimuth with a SinOsc LFO
    SinOsc lfo(0.1) => Range r(pi / 2., -pi / 2.) => Patch p(amb2, "azimuth") => blackhole;
```
