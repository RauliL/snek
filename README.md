# snek

![Build](https://github.com/RauliL/snek/workflows/Build/badge.svg)

Experimental functional programming language which combines TypeScript typing
system with Python like syntax. Still a work in progress.

## Features

- Syntax inspired by [Python].
- Typing system inspired by [TypeScript].
- Supports following builtin data types, all immutable:
  - Booleans
  - Functions
  - Numbers (separate integers and floats)
  - Strings (full Unicode support)
  - Lists
  - Records (think of dictionaries in python or objects in JavaScript)
  - Null
- [Prototype-based programming] just like in [JavaScript].
- Module system where functions, values and types can be imported from other
  files located on your file system. In the future possibly from the Internet
  as well.
- [REPL].
- Relatively easy to embed into other C++ libraries/applications.

[Python]: https://www.python.org
[TypeScript]: https://www.typescriptlang.org
[Prototype-based programming]: https://en.wikipedia.org/wiki/Prototype-based_programming
[JavaScript]: https://en.wikipedia.org/wiki/JavaScript
[REPL]: https://en.wikipedia.org/wiki/REPL

## How to compile?

You need to have [CMake] and C++17 compiler installed on your system, then do
this:

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

And tada! You can either execute `build/cli/snek` directly or install the
executable to your system by running `sudo make install`. Head over to the
`examples` directory to see what this language has to offer.

[CMake]: https://cmake.org
