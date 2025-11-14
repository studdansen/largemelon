# largemelon

This is a single-header library for building parsers using the
[Ragel](http://www.colm.net/open-source/ragel/) finite-state machine (lexer)
generator and the [Lemon](https://sqlite.org/src/doc/trunk/doc/lemon.html)
LALR(1) parser generator.
It is implemented with nothing more than the standard C and C++ libraries.

This repo also provides package modules for locating and/or building the Ragel
and Lemon software tools, for which [largemelon.hpp](largemelon.hpp) provides a
bridge.

This may mature into a framework. For now, it is a collection of helper data
types, functions, and classes that can be reused between parsers.

## To Do

Port this to pure C. Lemon and Ragel output C, not strictly C++.
This would (potentially vastly) increase its applicability.

Should this be under the MIT License like
[dedap](https://github.com/studdansen/dedap)?
