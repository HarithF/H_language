A C-like toy language written in C++ which implements a subset of c99


## Usage

```
USAGE:
  H [-?|-h|--help] [-v|--version] [-t|--tokenize] [-p|--eval] [-e|--parse] [-ep|--eval-parsing] [-pp|--print-ast] [-c|--compile] [<file>]

Display usage information.

OPTIONS, ARGUMENTS:
  -?,   -h, --help          display this help and exit
  -v,   --version           display version info and exit
  -t,   --tokenize          display all tokens
  -p,   --parse             display syntactical errors while parsing if they exist
  -ep,  --eval-parsing      display the parser run through the code
  -pp,  --print-ast         display a pretty printed version of the source code
  -c,   --compile           compiles the given source code
  <file>                    Input file.

  Hint: use '-' as file to read from stdin
```

Use ```build_llvm.sh``` to install the appropriate version of LLVM to run the project (Currently not fully implemented. The Program only compiles to an AST without emitting LLVM or other lower level code)

Full description of the [C99 specs](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf).
