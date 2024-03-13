#include <cstring>
#include <iostream>
#include <fstream>

#include "lexer.h"
#include "parser.h"

using namespace H;

static const auto usage =
"Usage: h [options] file\n"
"\n"
"Options:\n"
"\t-h,\t--help\t\tdisplay this help and exit\n"
"\t-v,\t--version\tdisplay version info and exit\n"
"\t-l,\t--tokenize\tdisplay all tokens\n"
"\t-ep,\t--eval-parsing\tdisplay the parser run through the code\n"
"\t-p,\t--parse\t\tdisplay syntactical errors while parsing if they exist\n"
"\nHint: use '-' as file to read from stdin.\n"
;

static const auto version = "H compiler 0.1\n";

int main(int argc, char** argv) {
    try {
        bool tokenize = false;
        bool eval_parsing = false;
        bool parse = false;
        const char* file = nullptr;

        

        for (int i = 1; i != argc; ++i) {
            if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
                std::cerr << usage;
                return EXIT_SUCCESS;
            } else if (strcmp("-v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) {
                std::cerr << version;
                return EXIT_SUCCESS;
            } else if (strcmp("-t", argv[i]) == 0 || strcmp("--tokenize", argv[i]) == 0) {
                tokenize = true;
            } else if (strcmp("-ep", argv[i]) == 0 || strcmp("--eval-parsing", argv[i]) == 0) {
                eval_parsing = true;
            } else if (strcmp("-p", argv[i]) == 0 || strcmp("--parse", argv[i]) == 0) {
                parse = true;
            } else if (file == nullptr) {
                file = argv[i];
            } else {
                throw std::logic_error("multiple input files given");
            }
        }
        //
        if (file == nullptr)
            throw std::logic_error("no input file given");


        if(tokenize) {
            if (strcmp("-", file) == 0) {

                Lexer lex("<stdin>", std::cin);

                H::Tok t;
                do {
                    t = lex.lex();
                    std::cout << "in main: " << t << std::endl;

                } while(strcmp(Tok::tag2str(t.tag()), "<eof>") != 0);
            } else {
                std::ifstream ifs(file);
                Lexer lex(file, ifs);

                H::Tok t;
                do {
                    t = lex.lex();
                    if(t.tag() != Tok::Tag::M_EoF) std::cout << t.loc() << ": " << t << std::endl;
                } while(strcmp(Tok::tag2str(t.tag()), "<eof>") != 0);
            }

            if (num_errors != 0) {
            std::cerr << num_errors << " error(s) encountered" << std::endl;
            return EXIT_FAILURE;
            }

        }
        else if ((parse||eval_parsing)) {
            if (strcmp("-", file) == 0) {
                //Parser parser("<stdin>", std::cin, eval_parsing);
                //parser.parse_prg();
            } else {
                //std::ifstream ifs(file);
                //Parser parser(file, ifs, eval_parsing);
                //parser.parse_prg();
            }


        }
        else {
            // compile
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        std::cerr << usage;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "error: unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
