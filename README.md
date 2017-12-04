# Axiom

Axiom is a node-based realtime synthesizer, designed for size-constrained environments such as PC intros in the demoscene. It is currently developed by MONAD, but is entirely open source -- feel free to use and contribute!

Features:

 - Musician-friendly (ie knobs and sliders) interface
 - Highly customizable and flexible through a node editor and custom scripting language
 - Export to replayer with no dependencies (not even the standard library)
 - Use any DAW with VSTi support for note editing

**Axiom is currently in very early development, and so no prepackaged versions are available.** Feel free to contribute issues, features, and pull requests though!

## Building

*Coming soon!*

## Usage Guide

*Coming soon!*

## Development

Axiom is written in C++ compiled with gcc. The editor uses Qt and is cross-platform, the VSTi uses VSTGUI, and the replayer is compiled with mingw-w64. The node editor uses tinycc to compile node expressions.

## License

Licensed under the MIT license. See the LICENSE file in the repository for more details.
