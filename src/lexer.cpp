#include "lexer.h"
#include "values/c_keywords.cpp"

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cctype>

using namespace std;


namespace H {

Lexer::Lexer(const char* filename, std::istream& stream)
    : loc_{filename, {1, 1}, {1, 1}}
    , peek_pos_({1, 1})
    , stream_(stream)
{
    if (!stream_) throw std::runtime_error("stream is bad");
}

int Lexer::next() {
    loc_.finish = peek_pos_;
    int c = stream_.get();

    if (c == '\n') {
        ++peek_pos_.row;
        peek_pos_.col = 1;
    } else {
        ++peek_pos_.col;
    }

    return c;
}

void Lexer::back() {
    stream_.unget();
}

Tok Lexer::lex() {
    vector<int> escape_sequences = {'\'', '"', '?', '\\', 'a', 'b', /*'e',*/ 'f', 'n', 'r', 't', 'v'};
    while (true) {
        loc_.begin = peek_pos_;
        str_.clear();

        if (eof()) return tok(Tok::Tag::M_EoF, std::string("eof"), std::string("M_EoF"));
        if (accept_if(::isspace)) continue;
        
         // Lex comments
        if (accept('/')) {
            if (accept('*')) {
                eat_comments();
                continue;
            }
            if (accept('/')) {
                while (!eof() && peek() != '\n' && peek() != '\r') next();
                continue;
            }
            if (accept('=')) return tok(Tok::Tag::P_Division_Assign, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Division, str_ , std::string("punctuator"));

            
        }

       
        // lex punctuators
        {
        // Single ,
        if (accept(',')) return tok(Tok::Tag::P_Comma, str_, std::string("punctuator"));

        // single :
        if (accept(':')) return tok(Tok::Tag::P_Colon, str_, std::string("punctuator"));
        
        // Single ;
        if (accept(';')) return tok(Tok::Tag::P_Semicolon, str_, std::string("punctuator"));

        // Single (
        if (accept('(')) return tok(Tok::Tag::D_Parenthesis_L, str_, std::string("punctuator"));
        
        // Single )
        if (accept(')')) return tok(Tok::Tag::D_Parenthesis_R, str_, std::string("punctuator"));
        
        // Single [
        if (accept('[')) return tok(Tok::Tag::D_Bracket_L, str_, std::string("punctuator"));
        
        // Single ]
        if (accept(']')) return tok(Tok::Tag::D_Bracket_R, str_, std::string("punctuator"));
        
        // Single {
        if (accept('{')) return tok(Tok::Tag::D_Brace_L, str_, std::string("punctuator"));
        
        // Single }
        if (accept('}')) return tok(Tok::Tag::D_Brace_R, str_, std::string("punctuator"));
        
        // Single ?
        if (accept('?')) return tok(Tok::Tag::P_Inline_If, str_, std::string("punctuator"));

        // Single ~        
        if (accept('~')) return tok(Tok::Tag::P_Bitwise_Not, str_ , std::string("punctuator"));


        // Starting with .    
        if (accept('.')) {
            if(accept('.')) {
                if(accept('.')) {
                    // Three dots
                    return tok(Tok::Tag::P_Tripple_Dot, str_ , std::string("punctuator"));
                }
                // Two Dots
                back();
            }
            // One or two dots
            return tok(Tok::Tag::P_Dot, "." , std::string("punctuator"));
        }

        // Starting with -        
        if (accept('-')){
            if (accept('-')) return tok(Tok::Tag::P_Decrement, str_ , std::string("punctuator"));
            if (accept('=')) return tok(Tok::Tag::P_Substraction_Assign, str_ , std::string("punctuator"));
            if (accept('>')) return tok(Tok::Tag::P_Arrow_R, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Substraction, str_ , std::string("punctuator"));
        }
        
        // Starting with #        
        if (accept('#')){
            if (accept('#')) return tok(Tok::Tag::P_Preprocessor_Concat, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Preprocessor_Stringize, str_ , std::string("punctuator"));
        } 

        // Starting with =        
        if (accept('=')){
            if (accept('=')) return tok(Tok::Tag::P_Equal, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Assign, str_ , std::string("punctuator"));
        }
        
        // Starting with !        
        if (accept('!')){
            if (accept('=')) return tok(Tok::Tag::P_Unequal, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Logical_Not, str_ , std::string("punctuator"));
        }

        // Starting with +        
        if (accept('+')){
            if (accept('+')) return tok(Tok::Tag::P_Increment, str_ , std::string("punctuator"));
            if (accept('=')) return tok(Tok::Tag::P_Addition_Assign, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Addition, str_ , std::string("punctuator"));
        }

        // Starting with *        
        if (accept('*')){
            if (accept('=')) return tok(Tok::Tag::P_Multiplication_Assign, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Multiplication, str_ , std::string("punctuator"));
        }

        // Starting with %        
        if (accept('%')){
            if (accept('=')) return tok(Tok::Tag::P_Modulo_Assign, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Modulo, str_ , std::string("punctuator"));
        }

        // Starting with &        
        if (accept('&')){
            if (accept('&')) return tok(Tok::Tag::P_Logical_And, str_ , std::string("punctuator"));
            if (accept('=')) return tok(Tok::Tag::P_Bitwise_And_Assign, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Bitwise_And, str_ , std::string("punctuator"));
        }

        // Starting with <        
        if (accept('<')){
            if (accept('=')) return tok(Tok::Tag::P_Less_Equal, str_ , std::string("punctuator"));
            if (accept('<')) {
                if (accept('=')) return tok(Tok::Tag::P_Bitwise_Shift_L_Assign, str_ , std::string("punctuator"));
                return tok(Tok::Tag::P_Bitwise_Shift_L, str_ , std::string("punctuator"));
            }
            return tok(Tok::Tag::P_Less, str_ , std::string("punctuator"));
        }

        // Starting with >        
        if (accept('>')){
            if (accept('=')) return tok(Tok::Tag::P_Greater_Equal, str_ , std::string("punctuator"));
            if (accept('>')) {
                if (accept('=')) return tok(Tok::Tag::P_Bitwise_Shift_R_Assign, str_ , std::string("punctuator"));
                return tok(Tok::Tag::P_Bitwise_Shift_R, str_ , std::string("punctuator"));
            }
            return tok(Tok::Tag::P_Greater, str_ , std::string("punctuator"));
        }

        // Starting with |        
        if (accept('|')){
            if (accept('|')) return tok(Tok::Tag::P_Logical_Or, str_ , std::string("punctuator"));
            if (accept('=')) return tok(Tok::Tag::P_Bitwise_Or_Assign, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Bitwise_Or, str_ , std::string("punctuator"));
        }

        // Starting with ^        
        if (accept('^')){
            if (accept('=')) return tok(Tok::Tag::P_Bitwise_Xor_Assign, str_ , std::string("punctuator"));
            return tok(Tok::Tag::P_Bitwise_Xor, str_ , std::string("punctuator"));
        }
        }

        // lex constants   
        // integer constants
        if(accept('0')) {
            if(accept_if(::isdigit)) {
                // Error
                loc_.err() << "0 may not be followed by another digit." << loc_.endErr();
                //TODO next() ?
                continue;
            }

            if(accept_if(::isalpha)) {
                // Error
                loc_.err() << "numbers may not be followed by a letter." << loc_.endErr();
                //TODO next() ?
                continue;
            }
            else {
                // 0
                return tok(Tok::Tag::C_Integer, str_, "constant");
            }
        }

        if(accept_if(::isdigit)) {
            // munch all numbers away
            while (accept_if(::isdigit)) {}

            try {std::stoull(str_);} catch(out_of_range &e) {
                loc_.err() << "Integer constants must be less than 2^64." << loc_.endErr();
                continue;
            }
            return tok(Tok::Tag::C_Integer, str_, "constant");
        }

        // character constants
        if(accept('\'')) {

            // munch until ' or eof o  cntrl or space
            bool backslashMunched = false;
            while(accept_if([this, backslashMunched](int i){return !((i == '\'' && !backslashMunched) || iscntrl(i) || eof());})) { 
                backslashMunched = str_.back() == '\\' ? !backslashMunched : false;
            }
            bool closing = accept('\'');

            //cout << "The munching is done." << endl;
            
            // last is not a ' -> not closing
            if(!closing) {
                loc_.err() << "Character constant without closing quote." << loc_.endErr();
                continue;
            }

            // if len 2 -> empty 
            if(str_.length() == 2) {
                loc_.err() << "Character constant may not be empty." << loc_.endErr();
                continue;
            }

            // 'a'
            // len 3, return 2nd
            if(str_.length() == 3) {

                return tok(Tok::Tag::C_Character, str_, "constant");
            }
            
            // '\n'
            // len 4 && pos 2 == \ -> Check if valid escape sequence, else invalid escape sequence
            if(str_.length() == 4 && str_[1] == '\\') {
                bool valid = std::find(escape_sequences.begin(), escape_sequences.end(), str_[2]) != escape_sequences.end();

                if(valid) {

                    return tok(Tok::Tag::C_Character, str_, "constant");
                } else {
                    loc_.err() << "Character constant with invalid escape sequence.." << loc_.endErr();
                    continue;
                }
            }

            // Too long!
            loc_.err() << "Character constant contains too many characters!" << loc_.endErr();
            continue;
        }

        // lex string literals
        if(accept('"')) {
            // munch until " or eof or cntrl or space
            bool backslashMunched = false;
            bool invalidEscapeSequenceRead = false;
            while(accept_if([this, backslashMunched](int i){return !((i == '\"' && !backslashMunched) || (iscntrl(i) && i != '\f') || eof());})) {
                if(backslashMunched) {
                    bool valid = std::find(escape_sequences.begin(), escape_sequences.end(), str_.back()) != escape_sequences.end();
                    if(!valid) invalidEscapeSequenceRead = true;
                }
                backslashMunched = str_.back() == '\\' ? !backslashMunched : false;
            }
            bool closing = accept('\"');


            if(invalidEscapeSequenceRead) {
                loc_.err() << "String literal contains an invalid escape sequence." << loc_.endErr();
                continue;
            }
            if(!closing) {
                loc_.err() << "String literal without closing quotation marks." << loc_.endErr();
                continue;
            }

            return tok(Tok::Tag::S_Literal, str_, "string-literal");

        }

        // lex identifier or keyword
        if (accept_if([](int i) { return i == '_' || isalpha(i); })) {
            while (accept_if([](int i) { return i == '_' || isalpha(i) || isdigit(i); })) {}
            
            // Try to match a KEYWORD
            map<string, Tok::Tag>::iterator it = c_keywords.begin();
            while(it != c_keywords.end()) {
                string token_str = it->first;
                if(str_.compare(token_str) == 0) {
                    //cout << token_str << " is a keyword (really!)" << endl;
                    return tok(it->second, std::string(str_), std::string("keyword"));
                }
                it++;
            }
            
            //cout << str_ << " is an identifier (really!)" << endl;
            return tok(Tok::Tag::M_Id, str_.c_str(), std::string("identifier"));
        }

        loc_.err() << "invalid input char: '" << (char) peek() << "'" << loc_.endErr();
        next();
    }
}

void Lexer::eat_comments() {
    while (true) {
        while (!eof() && peek() != '*') next();
        if (eof()) {
            loc_.err() << "non-terminated multiline comment" << loc_.endErr();
            return;
        }
        next();
        if (accept('/')) break;
    }
}

}
