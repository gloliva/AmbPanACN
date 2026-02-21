# AmbiPan

AmbiPan is an ambisonics panner chugin (i.e. ChucK plugin) that supports up to 7th order.
It uses ACN ordering and SN3D normalization.

## Installation

Run the appropriate `makefile` for your system (i.e. mac, win, linux).

```bash
$ make mac
```

## How to Run

You will most likely need to explicitly set the output audio device and the number of channels you need. To find this information, run:

```bash
# Example chuck probe output
$ chuck --probe

[chuck]: [CoreAudio] driver found 9 audio device(s)...
[chuck]:
[chuck]: ------( audio device: 1 )------
[chuck]: device name = "My Ambisonics Decoder Device"
[chuck]: probe [success]...
[chuck]: output channels = 64
[chuck]: input channels  = 64
[chuck]: duplex Channels = 64
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

```bash
$ chuck --chugin:./AmbiPan.chug --dac:<DEVICE_FOR_AMBISONICS> --out:<NUM_OUTS_NEEDED_FOR_ORDER> program.ck
```

For example, if your device number is `4` and you are working in 5th order ambisonics (which needs 36 outputs), run:

```bash
$ chuck --chugin:./AmbiPan.chug --dac:4 --out:36 program.ck
```

## How To Use AmbiPan

`AmbiPan` is a UGen that can be connected like any other:

```java
// Send a sine oscillator into a 5th order ambisonics panner, and then to the DAC
SinOsc osc(440.) => AmbiPan pan(5) => dac;
```

The azimuth and elevation can be set like so:

```java
pi / 2. => pan.azimuth;
pi / 4. => pan.elevation;

// Set both azimuth and elevation
pan.set(pi / 2., pi / 4.);
```

For automating these values, one approach is to use the `Patch` and `Range` classes, which can be downloaded with `chump`.

```java
@import "Range"
@import "Patch"

// Ambisonics panner
SinOsc osc(440.) => AmbiPan pan(5) => dac;

// Automate azimuth with a SinOsc LFO
SinOsc lfo(0.1) => Range r(pi / 2., -pi / 2.) => Patch p(pan, "azimuth") => blackhole;
```

A larger example with three voices can be viewed in `examples/AmbiPan-example3Voices.ck`.

### Caveats

Due to how ChucK currently handles creating multichannel UGens, and in order to have only 1 ambisonics panner class (as opposed to `AmbiPan1`, `AmbiPan2`, ..., `AmbiPan7`), `AmbiPan` is a 64 channel UGen regardless of order (however, it only does the calculations for the order that is set). This can limit the number of concurrent voices; if you need a large number of concurrent voices, it is recommended to use the `AmbiEnc` encoders instead, which are a fixed order.

When connecting an `AmbiPan` object to the DAC, if the DAC is the exact number of channels as is needed by the ambisonics order, or if the remaining channels are uneeded/unused (e.g. output device supports 64 channels, but you are only using the first 36 for 5th order ambisonics), then you can connect the panner to the DAC with the `=>` operator:

```java
AmbiPan pan(5) => dac;
```

However, if you are using less than 64 channels (i.e. less than 7th order) for a panner and you intend to use the remaining channels for something else, you will want to connect each channel of the panner to the DAC manually. An example of this would be your DAC is 45 channels, and you are using the first 36 channels for a 5th order decoder, and the remaining 9 channels for a 2nd order decoder. You can connect channels manually like this:

```java
// Define 5th order and 2nd order panners
AmbiPan pan5(5);
AmbiPan pan2(2);

// Connect the 5th order panner (36 channels) to DAC channels 0 - 35
for (int c; c < pan5.outChannels(); c++) {
    pan5.chan(c) => dac.chan(c);
}

// Connect the 2nd order panner (9 channels) to DAC channels 36 - 44
for (36 => int c; c < pan2.outChannels(); c++) {
    pan2.chan(c) => dac.chan(c);
}
```

This ensures that only the used channels of the panner are connected to the DAC. Technically, all unused channels output a value of 0 each tick, but it is still advised to connect the channels manually in this situation.
