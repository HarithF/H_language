#ifndef PROG_PARSER_H
#define PROG_PARSER_H

#include "ast.h"
#include "lexer.h"
#include <vector>

namespace H {


class Parser {
public:
    Parser(const char* file, std::istream& stream, bool evaluate_parsing, bool prettyPrint, bool meme);

    void parse_prg();

private:
    Ptr<Exp> parse_exp(const char* ctxt, Tok::Prec p = Tok::Prec::Bottom );
    Ptr<Exp> parse_primary_expr(const char* ctxt);
    Ptr<Stmt> parse_stmt(const char* ctxt, bool labeledStmt=false);


    Ptrs<Exp> parse_expr_list(const char* ctxt);
    


    /// Trick to easily keep track of @p Loc%ations.
    class Tracker {
    public:
        Tracker(Parser& parser, const Pos& pos)
            : parser_(parser)
            , pos_(pos)
        {}

        operator Loc() const { return {parser_.prev_.file, pos_, parser_.prev_.finish}; }

    private:
        Parser& parser_;
        Pos pos_;
    };

    // Helpers
    void eat_rest_of_statement(bool with_semicolon);
    bool type_follows();
    bool type_follows_twoahead();
    Ptr<ErrExp> createErrExp(Tracker track, bool with_semicolon, const char* ctxt, const char* status);
    Ptr<ErrStmt> createErrStmt(Tracker track, bool with_semicolon, const char* ctxt, const char* status);
    Ptr<ErrDecl> createErrDecl(Tracker track, bool with_semicolon, const char* ctxt, const char* status);
    Ptr<Exp> parse_member_access(Parser::Tracker track, Ptr<Exp> lhs);


    // New Declarations
    Ptr<ExternalDeclaration> parse_external_declaration();
    Ptr<SpecifierDeclarator> parse_specifier_declarator();
    Ptr<Specifier> parse_specifier();
    Ptr<Declarator> parse_declarator();
    



    /// Factory method to build a @p Tracker.
    Tracker tracker() { return Tracker(*this, ahead().loc().begin); }

    /// Invoke @p Lexer to retrieve next @p Tok%en.
    Tok lex();

    void print_parsing(const char* ctxt, const char* status);
    bool evaluate_parsing() const { return evaluate_parsing_; }
    bool prettyPrint() const { return prettyPrint_; }
    bool meme() const { return meme_; }

    /// Get lookahead.
    Tok ahead() const { return ahead_; }
    Tok two_ahead() const { return two_ahead_; }

    /// If @p ahead() is a @p tag, @p lex(), and return @c true.
    bool accept(Tok::Tag tag);

    /// @p lex @p ahead() which must be a @p tag.
    /// Issue @p err%or with @p ctxt otherwise.
    bool expect(Tok::Tag tag, const char* ctxt);

    /// Consume @p ahead which must be a @p tag; @c asserts otherwise.
    Tok eat([[maybe_unused]] Tok::Tag tag) { assert(tag == ahead().tag() && "internal parser error"); return lex(); }

    /// Issue an error message of the form:
    /// <code>expected <what>, got '<tok>' while parsing <ctxt></code>
    void err(const std::string& what, const Tok& tok, const char* ctxt);

    /// Same above but uses @p ahead() as @p tok.
    void err(const std::string& what, const char* ctxt) { err(what, ahead(), ctxt); }

    Lexer lexer_;
    Loc prev_;
    Tok ahead_;
    Tok two_ahead_;
    bool evaluate_parsing_;
    bool prettyPrint_;
    bool meme_;
    bool debugDump = false;
    int prettyIndent = 1;

    bool semanticCheck = true;
};

}

#endif
