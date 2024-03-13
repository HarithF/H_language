#include "tok.h"

namespace H {

const char* Tok::tag2str(Tag tag) {
    switch (tag) {
        #define CODE(t, str) \
            case Tag::t: return str;
            H_KEY(CODE)
            H_LIT(CODE)
            H_TOK(CODE)
        #undef CODE
        #define CODE(t, str, prec_l, prec_r) \
            case Tag::t: return str;
            H_OP(CODE)
        #undef CODE
        default: assert(false && "<UNKNOWN TOKEN TYPE! See: tok.cpp>"); // nullptr shutup warning
    }
}

const char* Tok::prec2str(Prec prec) {
    switch (prec) {
        case Tok::Prec::Error: return "Error";
        case Tok::Prec::Bottom: return "Bottom";
        case Tok::Prec::Comma: return "Comma";
        case Tok::Prec::Assignment: return "Assignment";
        case Tok::Prec::Conditional: return "Conditional";
        case Tok::Prec::LogicalOR: return "LogicalOR";
        case Tok::Prec::LogicalAND: return "LogicalAND";
        case Tok::Prec::BitwiseOR: return "BitwiseOR";
        case Tok::Prec::BitwiseXOR: return "BitwiseXOR";
        case Tok::Prec::BitwiseAND: return "BitwiseAND";
        case Tok::Prec::Equality: return "Equality";
        case Tok::Prec::Relational: return "Relational";
        case Tok::Prec::Shift: return "Shift";
        case Tok::Prec::Additive: return "Additive";
        case Tok::Prec::Multiplicative: return "Multiplicative";
        case Tok::Prec::Unary: return "Unary";
        case Tok::Prec::Postfix: return "Postfix";
        case Tok::Prec::Top: return "Top";
        default: assert(false && "<UNKOWN PRECEDENT! See:tok.cpp>");
    }
}

Tok::Prec Tok::tag2prec_l(Tag tag) {
    switch (tag) {
        #define CODE(t, str, prec_l, prec_r) \
            case Tag::t: return Prec::prec_l;
            H_OP(CODE)
        #undef CODE
        default: return Prec::Error;
    }
}

Tok::Prec Tok::tag2prec_r(Tag tag) {
    switch (tag) {
        #define CODE(t, str, prec_l, prec_r) \
            case Tag::t: return Prec::prec_r;
            H_OP(CODE)
        #undef CODE
        default: return Prec::Error;
    }
}




std::ostream& operator<<(std::ostream& o, const Tok& tok) {

    //if(tok.tag() == Tok::Tag::M_EoF) return o;
    return o << tok.token_type() << " " << tok.str();
    
    //if (true/*tok.isa(Tok::Tag::M_Id)*/) return o << tok.str(); //TODO
    //return o << Tok::tag2str(tok.tag());
    
}

}
