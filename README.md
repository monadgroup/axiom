# Axiom

![A basic sine-wave synth](axiom.png)

> A basic sine-wave synth built in the current work-in-progress version of Axiom

Axiom is a node-based realtime synthesizer, designed for size-constrained environments such as PC intros in the demoscene. It is currently developed by MONAD, but is entirely open source -- feel free to use and contribute!

Features:

 - Musician-friendly (ie knobs and sliders) interface
 - Highly customizable and flexible through a node editor and custom scripting language, named Maxim
 - Export to replayer with no dependencies (not even the standard library)
 - Use any DAW with VSTi support for note editing and automation

**Axiom is currently in very early development, and so no prepackaged versions are available.** Feel free to contribute issues, features, and pull requests though!

## Building

Axiom is built with CMake. The build process depends on Qt 5.10 and LLVM 5.0.1, so make sure those are installed and setup correctly.

Once Qt and LLVM are installed, go to the directory where you'd like to build Axiom to. Then run the following command:

```
cmake ../path/to/source
```

CMake will setup files necessary for building. If this fails, make sure you've got Qt and LLVM installed correctly. Once complete, run the following command from your output directory to build the Axiom VSTi.

```
cmake --build ./ --target axiom_editor
```

This will build the Axiom VSTi as a shared library into the current folder. Once complete, follow the instructions for your DAW to install the VST.

Alternatively, you can build the standalone version, as an executable. Currently, the interface is entirely useable in this version, however you won't actually be able to make sounds. To build the standalone version, use the following command:

```
cmake --build ./ --target axiom_standalone
```

## Usage Guide

*Coming soon!*

## Development

Axiom is comprised of several components:

 - The VST Editor, written with Qt and the VST SDK. This is the only part the user directly interacts with, and must be
   OS-independent.
 - The Maxim language compiler and runtime, written with LLVM and statically linked into the editor.
 - The replayer, written in size-optimised C++. Due to it's reliance on Maxim snippets, this must be compiled with
   Clang. The replayer provides a function to fill a buffer, use with the API of your choice.

## License

Licensed under the MIT license. See the LICENSE file in the repository for more details.
