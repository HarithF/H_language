#ifndef LAM_LEXER_H
#define LAM_LEXER_H

#include <cassert>
#include <istream>

#include "tok.h"

namespace H {

class Lexer {
public:
    Lexer(const char*, std::istream&);

    Loc loc() const { return loc_; }
    Tok lex();                                          ///< Get next @p Tok in stream.

private:
    Tok tok(Tok::Tag tag, std::string tag_value, std::string tag_type) {

        if(tag == Tok::Tag::C_Integer) {
            return {loc(), tag, tag_value, tag_type, std::stoull(tag_value)};
        }

        return {loc(), tag, tag_value, tag_type}; 
    }        
 
    bool eof() const { peek(); return stream_.eof(); }  


    /// In this case invoke @p next() and append to @p str_;
    template<class Pred>
    bool accept_if(Pred pred) {
        if (pred(peek())) {
            str_ += next();
            return true;
        }
        return false;
    }

    bool accept(int val) {
        return accept_if([val] (int p) { return p == val; });
    }

    /// Get next byte in @p stream_ and increase @p loc_ / @p peek_pos_.
    int next();
    void back();
    int peek() const { return stream_.peek(); }
    void eat_comments();


    Loc loc_;       ///< @p Loc%ation of the @p Tok%en we are currently constructing within @p str_,
    Pos peek_pos_;  ///< @p Pos%ition of the current @p peek().
    std::istream& stream_;
    std::string str_;
};

}

#endif
