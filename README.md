A C-like toy language written in C++ which implements a subset of c99


## Usage

```
USAGE:
  H [-?|-h|--help] [-v|--version] [-d|--dump] [-e|--eval] [<file>]

Display usage information.

OPTIONS, ARGUMENTS:
  -?,   -h, --help          display this help and exit
  -v,   --version           display version info and exit
  -t,   --tokenize          display all tokens
  -p,   --parse             display syntactical errors while parsing if they exist
  -ep,  --eval-parsing      display the parser run through the code
  <file>                    Input file.

  Hint: use '-' as file to read from stdin
```

Full description of the [C99 specs](http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf).
