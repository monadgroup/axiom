# Axiom [![Build Status](https://travis-ci.org/monadgroup/axiom.svg?branch=master)](https://travis-ci.org/monadgroup/axiom)

![Picture of a synth built in Axiom](axiom.png)

> A synth built in the current version of Axiom

Axiom is an extremely flexible node-based realtime audio synthesizer. It was originally designed for size-constrained environments such as PC intros in the demoscene, but is entirely open source and is becoming an excellent free tool for any musician.

Features:

 - Musician-friendly (ie knobs and sliders) interface
 - Highly customizable and flexible through a node editor and Maxim, a custom scripting language
 - Export to replayer with no dependencies (not even the standard library)
 - Use any DAW with VSTi support for note editing and automation

There are currently pre-packaged versions available for Windows and macOS (alpha, let us know of any issues) on [the Releases page](https://github.com/monadgroup/axiom/releases). Stay tuned for Linux builds!

**[Usage Guide](https://github.com/monadgroup/axiom/wiki/Usage-Guide) · [Example Projects](https://github.com/monadgroup/axiom/tree/master/examples) · [Downloads & Release Notes](https://github.com/monadgroup/axiom/releases)**

## Backends

Axiom currently supports the following audio backends:

 - Standalone editor - doesn't require a DAW or host, allowing experimentation with the editor. MIDI can be input from a MIDI device, or by pressing keys on a regular computer keyboard.
 - VST2 - runs in a VST host as an instrument or effect, with support for side-chaining and multiple inputs/outputs.
 - _Other backends such as VST3 are planned_

## Building

Axiom is built with CMake. The build process depends on Cargo, Qt 5.10+, LLVM 6, and the VST 2 SDK (for the VST2 backend), so make sure those are installed and setup correctly.

Once Cargo, Qt, LLVM, and the VST SDK are installed, go to the directory where you'd like to build Axiom to. Then run the following command:

```
cmake ../path/to/source -DVST2_SDK_ROOT=/path/to/vst/sdk
```

If you want to build it statically-linked, pass the `AXIOM_STATIC_LINK` flag:

```
cmake ../path/to/source -DAXIOM_STATIC_LINK=ON -DVST2_SDK_ROOT=/path/to/vst/sdk
```

CMake will setup files necessary for building. If this fails, make sure you've got Cargo, Qt, LLVM, and the VST SDK installed correctly. Once complete, you can choose which backend to build:

### VST2 Instrument & VST2 Effect

* To build the VST2 instrument backend, use the following command. Make sure you provided a path to the VST SDK in the command above.
```
cmake --build ./ --target axiom_vst2_instrument
```

* You can also build the VST2 effect with the `axiom_vst2_effect` target.
```
cmake --build ./ --target axiom_vst2_effect
```

### Standalone

* To build the standalone version as an executable, use the following command. The standalone optionally depends on PortAudio and PortMidi: without PortAudio nodes will not be simulated and audio will not be output, without PortMidi MIDI devices cannot be used for input.


```
cmake --build ./ --target axiom_standalone
```

## Development

Axiom is comprised of several components:

 - The VST Editor, written with Qt and the VST SDK. This is the only part the user directly interacts with, and must be
   OS-independent. 
 - The Maxim language compiler and runtime, written in Rust with LLVM and statically linked into the editor.
 - The replayer, _coming soon_.

## License

Licensed under the MIT license. See the LICENSE file in the repository for more details.
