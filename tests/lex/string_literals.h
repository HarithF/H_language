// Valid examples
""                      // empty string
"h"                     // single character
"\n"                    // valid escape sequence
"\n h \t"               // many escape sequences
"\\\\\n\\a\\"           // many many escape sequences
"hello"                 // multi char
"hello world"           // space
" 'c' "                 // single quotes
"this is a /*test*/"    // string with comment -> should not be treated as comment

// invalid examples
"\x"            // invalid escape sequence
"\\\ "          // invalid escape sequence
" hello world   // missing end quotation marks
