# A compile-time brainfuck interpreter

A C++ compile-time brainfuck interpreter written in a purely functional style using template metaprogramming.

### Implementation

The memory model consists of instruction pointer and an array (a tape) of an unbounded number (in all directions) of byte cells initialized to zero. IO commands operate on two streams of bytes for input and output. When a program tries to read from an empty input stream, an error is being raised.

The main engine of the interpreter is `bfcore.hpp` file, which is fully functional and working module itself. However `bfutils.hpp` file provides a few more templates making the intepreter more usable.

Possible improvements include implementing compile-time strings.

### Usage

Both program source code and user input are provided by preprocessor macros (`PROGRAM_CODE` and `PROGRAM_INPUT` respectively) and can be either hardcoded in `main.cpp` file or passed during compilation using compiler's option `-D` (`/D` for MSVC) on the command line. Though program's output could be also printed at compile time via an error message I chose a cleaner solution and left it for an executable produced by a compiler.

