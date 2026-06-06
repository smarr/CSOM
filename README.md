CSOM - The Simple Object Machine implemented in C
=================================================

Introduction
------------

SOM is a minimal Smalltalk dialect used to teach VM construction at the [Hasso
Plattner Institute][SOM]. It was originally built at the University of Århus
(Denmark) where it was also used for teaching.

Currently, implementations exist for Java (SOM), C (CSOM), C++ (SOM++), and
Squeak/Pharo Smalltalk (AweSOM).

A simple SOM Hello World looks like:

```Smalltalk
Hello = (
  run = (
    'Hello World!' println.
  )
)
```

This repository contains a plain C implementation of SOM, including an
implementation of the SOM standard library and a number of examples. Please see
the [main project page][SOMst] for links to the VM implementations.


CSOM can be built with Make:

    $ make

Afterwards, the tests can be executed with:

    ./som.sh -cp Smalltalk TestSuite/TestHarness.som
   
A simple Hello World program is executed with:

    ./som.sh -cp Smalltalk Examples/Hello.som

The debug version of CSOM can be built using the `debug` target:

    $ make debug

Information on previous authors are included in the AUTHORS file. This code is
distributed under the MIT License. Please see the LICENSE file for details.
Additional documentation, detailing for instance the object model and how to
implement primitives, is available in the `doc` folder.

WebAssembly CSOM
----------------

CSOM can be compiled using GCC, Clang, but also emscripten.
With emscripten, it can run on Node.js as follows:

 - install [emscripten](http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html) and set it up for compilation.
   One easy way is using Docker, as in the .travis.yml. Another is with [emsdk](https://github.com/juj/emsdk/)
 - build with: `make emscripten`
 - run with: `node CSOM.js -cp Smalltalk Examples/Hello.som`

Build Status
------------

This repo has CI setups for GitLab CI and GitHub Actions.

[![Build Status](https://github.com/SOM-st/CSOM/actions/workflows/ci.yml/badge.svg)](https://github.com/SOM-st/CSOM/actions)

 [SOM]: http://www.hpi.uni-potsdam.de/hirschfeld/projects/som/
 [SOMst]: https://travis-ci.org/SOM-st/


      
