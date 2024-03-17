#ifndef PROG_TOK_H
#define PROG_TOK_H

#include <cassert>

#include "loc.h"
#include "values/magic.cpp"

namespace H {

class Tok {
public:
    enum class Tag {
        #define CODE(t, str) t,
            H_KEY(CODE)
            H_LIT(CODE)
            H_TOK(CODE)
        #undef CODE
        #define CODE(t, str, prec_l, prec_r) t,
            H_OP(CODE)
        #undef CODE
        };
        
    enum class Prec {
        Error,          // If lookahead isn't a valid operator.Bottom, // Starting Prec is lowest. Shift, Add, Mul, Cast,
        Bottom,         // Starting Prec is lowest.
        Comma,
        Assignment,
        Conditional,
        LogicalOR,
        LogicalAND,
        BitwiseOR,
        BitwiseXOR,
        BitwiseAND,
        Equality,
        Relational,
        Shift,
        Additive,
        Multiplicative,
        Unary,
        Postfix,
        Top
    };

    Tok() {}
    Tok(Loc loc, Tag tag, std::string& token_type)
        : loc_(loc)
        , tag_(tag)
        , token_type_(token_type)
    {}
    Tok(Loc loc, const std::string& str, std::string& token_type)
        : loc_(loc)
        , tag_(Tag::M_Id)
        , str_(str)
        , token_type_(token_type)
    {}
    Tok(Loc loc, Tag tag, const std::string& str, std::string& token_type)
        : loc_(loc)
        , tag_(tag)
        , str_(str)
        , token_type_(token_type)
    {}
    Tok(Loc loc, Tag tag, const std::string& str, std::string& token_type, uint64_t val)
        : loc_(loc)
        , tag_(tag)
        , str_(str)
        , value_(val)
        , token_type_(token_type)
    {}

    Loc loc() const { return loc_; }
    Tag tag() const { return tag_; }
    std::string token_type() const { return token_type_; }
    uint64_t value() const { return value_; }
    bool isa(Tag tag) const { return tag == tag_; }
    const std::string& str() const { /*assert(isa(Tag::M_Id)); TODO*/ return str_; }

    static const char* tag2str(Tok::Tag);
    static const char* prec2str(Tok::Prec);
    static Prec tag2prec_l(Tag);
    static Prec tag2prec_r(Tag);

    private:
        Loc loc_;
        Tag tag_;
        std::string str_;
        uint64_t value_;
        std::string token_type_;

};

std::ostream& operator<<(std::ostream&, const Tok&);

}

#endif
