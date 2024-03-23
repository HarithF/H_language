#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#include "parser.h"

#include <fstream>
#include <iostream>
#include <cstring>

namespace H {

    //! =======================================================================
    //! ================================ BASIC ================================
    //! =======================================================================

    Parser::Parser(const char* file, std::istream& stream, bool evaluate_parsing, bool prettyPrint)
        : lexer_(file, stream)
        , prev_(lexer_.loc())
        , ahead_(lexer_.lex())
        , two_ahead_(lexer_.lex())
        , evaluate_parsing_(evaluate_parsing)
        , prettyPrint_(prettyPrint)

    {}

    Tok Parser::lex() {
        //std::cout << "ahead: " << ahead() << " | two_ahead: " << two_ahead() << std::endl;
        auto result = ahead();
        ahead_ = two_ahead();
        two_ahead_ = lexer_.lex();
        return result;
    }

    bool Parser::accept(Tok::Tag tag) {
        if (tag != ahead().tag()) return false;
        lex();
        return true;
    }

    bool Parser::expect(Tok::Tag tag, const char* ctxt) {
        if (ahead().tag() == tag) {
            lex();
            return true;
        }

        err(std::string("'") + Tok::tag2str(tag) + std::string("'"), ctxt);
        return false;
    }

    void Parser::err(const std::string& what, const Tok& tok, const char* ctxt) {
        tok.loc().err() << "expected " << what << ", got '" << tok << "' while parsing " << ctxt << "\033[0m" << std::endl;
    }

    void Parser::print_parsing(const char* ctxt, const char* status = ""){
        if (evaluate_parsing()){

            std::cout << ahead().loc().begin;
            if (strcmp(ctxt, "program") == 0) {if (strcmp(status, "stopping") == 0) std::cout << std::endl;}
            else if (strcmp(status, "starting") == 0) {
                std::cout << std::string(prettyIndent, '\t') << ">>> " << ctxt << std::endl;
                prettyIndent += 1;
            }
            else if (strcmp(status, "stopping") == 0) {
                prettyIndent -= 1;
                std::cout << std::string(prettyIndent, '\t') << "<<< " << ctxt << std::endl;
            }
            else {
                std::cout << std::string(prettyIndent, '\t') << "- " << ctxt << std::endl;
            }

        }
    }



    //! ===========================================================================
    //! ================================ MAIN LOOP ================================
    //! ===========================================================================

    void Parser::parse_prg() {
        Tracker track = tracker();
        if (ahead().tag() == Tok::Tag::M_EoF) {
            err(std::string("a non-empty file"), "program");
            return;
        }

        Ptrs<ExternalDeclaration> externalDeclarations;

        while (ahead().tag() != Tok::Tag::M_EoF) {
            // program = list of external definitions (variable declarations and function definitions)? 
            if (evaluate_parsing_) std::cout << std::endl;
            externalDeclarations.emplace_back(parse_external_declaration());
        }

        expect(Tok::Tag::M_EoF, "program");

        //Ptr<AST> ast = mk<AST>(std::move(externalDeclarations));
        Ptr<TranslationUnit> translationUnit = mk<TranslationUnit>(track, std::move(externalDeclarations));


        if (prettyPrint() && num_errors==0) translationUnit->dump();

        Sema sema = Sema();

        if (prettyPrint() && num_errors==0) translationUnit->dump();
        if (semanticCheck && num_errors==0) translationUnit->check(sema);
   



        return;// exp;
    }

    //! ====================================================================================================
    //! ========================================= HELPER FUNCTIONS =========================================
    //! ====================================================================================================

    void Parser::eat_rest_of_statement(bool with_semicolon){
        while (ahead().tag() != Tok::Tag::P_Semicolon && ahead().tag() != Tok::Tag::M_EoF) lex();
        if (with_semicolon && ahead().tag() == Tok::Tag::P_Semicolon) lex();
    }

    bool Parser::type_follows(){
        return (ahead().tag()==Tok::Tag::K_void || ahead().tag()==Tok::Tag::K_int || ahead().tag()==Tok::Tag::K_char || ahead().tag()==Tok::Tag::K_struct);
    }

    bool Parser::type_follows_twoahead(){
        return (two_ahead().tag()==Tok::Tag::K_void || two_ahead().tag()==Tok::Tag::K_int || two_ahead().tag()==Tok::Tag::K_char || two_ahead().tag()==Tok::Tag::K_struct);
    }

    Ptrs<Exp> Parser::parse_expr_list(const char* ctxt){
        UNUSED(ctxt);
        Ptrs<Exp> parameters;
        Ptrs<Exp> error;
        lex();                                                      //opening paranthesis
        if (ahead().tag() != Tok::Tag::D_Parenthesis_R){
            do {
                if(ahead().tag()==Tok::Tag::D_Parenthesis_R) {
                    err("expression", "expression list");
                    eat_rest_of_statement(false);
                    return error;
                }
                print_parsing("expression in list", "starting");
                parameters.emplace_back(parse_exp("expression list"));
                print_parsing("expression in list", "stopping");
            } while (accept(Tok::Tag::P_Comma));
        }
        expect(Tok::Tag::D_Parenthesis_R, "expression list");       //closing paranthesis
        return parameters;
    }

    Ptr<ErrExp> Parser::createErrExp(Tracker track, bool with_semicolon, const char* ctxt, const char* status){
        auto errExp = mk<ErrExp>(track);
        eat_rest_of_statement(with_semicolon);
        if (strcmp(ctxt, "")!=0) print_parsing(ctxt, status);
        //errstmt->dump();
        return errExp;
    }

    Ptr<ErrStmt> Parser::createErrStmt(Tracker track, bool with_semicolon, const char* ctxt, const char* status){
        auto errStmt = mk<ErrStmt>(track);
        eat_rest_of_statement(with_semicolon);
        if (strcmp(ctxt, "")!=0) print_parsing(ctxt, status);
        //errstmt->dump();
        return errStmt;
    }

    Ptr<ErrDecl> Parser::createErrDecl(Tracker track, bool with_semicolon, const char* ctxt, const char* status){
        auto errdecl = mk<ErrDecl>(track);
        eat_rest_of_statement(with_semicolon);
        if (strcmp(ctxt, "")!=0) print_parsing(ctxt, status);
        //errdecl->dump();
        return errdecl;
    }
                        
    //! =========================================================================================
    //! ====================================== DECLARATION NEW ======================================
    //! =========================================================================================

    Ptr<ExternalDeclaration> Parser::parse_external_declaration(){
        Tracker track = tracker();                

        Ptr<SpecifierDeclarator> specifierDeclarator = parse_specifier_declarator();
        
        if (specifierDeclarator==nullptr) {
            auto error = mk<ErrDecl>(track);
            eat_rest_of_statement(true);
            return error;
        }

        if (ahead().tag() == Tok::Tag::D_Brace_L){
            Ptr<Stmt> functionBody = parse_stmt("external declaration");
            return mk<ExternalDeclaration>(track, std::move(specifierDeclarator), std::move(functionBody));
        }
        if (!expect(Tok::Tag::P_Semicolon, "external declaration")) eat_rest_of_statement(true);
        return mk<ExternalDeclaration>(track, std::move(specifierDeclarator));
    }

    Ptr<SpecifierDeclarator> Parser::parse_specifier_declarator(bool inside_paramlist){  
        Tracker track = tracker();  
        Ptr<Specifier> specifier = parse_specifier();
        
        if (specifier == nullptr) return nullptr;

        Ptr<Declarator> declarator = parse_declarator(inside_paramlist);
        if (declarator==nullptr) return mk<SpecifierDeclarator>(track, std::move(specifier));
        else return mk<SpecifierDeclarator>(track, std::move(specifier), std::move(declarator));
    }

    Ptr<Specifier> Parser::parse_specifier(){                                                   // type specifier
        Tracker track = tracker();  
        while (ahead().tag() == Tok::Tag::P_Semicolon) lex();                                   // eat away useless declarations with only a semicolon
        
        if (ahead().tag() == Tok::Tag::K_char || ahead().tag() == Tok::Tag::K_int || ahead().tag() == Tok::Tag::K_void) return mk<PrimitiveSpecifier>(track, lex());

        if (ahead().tag() == Tok::Tag::K_struct){
            Tok struct_specifier = lex();
            
            bool identifier_set = false;
            bool structDeclList_set = false;
            Ptrs<SpecifierDeclarator> structDeclarationList;
            Tok struct_identifier;

            if (ahead().tag() == Tok::Tag::M_Id) {
                struct_identifier = lex(); 
                identifier_set=true;
                print_parsing("struct identifier set");
            }

            if (identifier_set == false && ahead().tag()!=Tok::Tag::D_Brace_L){
                err("struct declaration list", "struct declaration");
                eat_rest_of_statement(false);
                return nullptr;                                             //TODO: Error 
            } else if (ahead().tag()==Tok::Tag::D_Brace_L) {
                print_parsing("struct-declaration-list", "starting");
                lex();
                structDeclList_set=true;
                if (ahead().tag() != Tok::Tag::D_Brace_R){
                    do
                    {
                        if (ahead().tag()==Tok::Tag::M_EoF){break;}
                        structDeclarationList.emplace_back(parse_specifier_declarator());                   // only declaration (not function definition) allowed. //TODO: check in semantic analysis? hard to catch here i guess... 
                        expect(Tok::Tag::P_Semicolon, "parameter list");
                    } while (ahead().tag()!=Tok::Tag::D_Brace_R);
                }

                expect(Tok::Tag::D_Brace_R, "struct declaration");
                print_parsing("struct-declaration-list", "stopping");
            }
            
            if (identifier_set && structDeclList_set)   return mk<StructSpecifier>(track, struct_specifier, struct_identifier, std::move(structDeclarationList));
            if (identifier_set && !structDeclList_set)  return mk<StructSpecifier>(track, struct_specifier, struct_identifier);
            if (!identifier_set && structDeclList_set)  return mk<StructSpecifier>(track, struct_specifier, std::move(structDeclarationList));
            return mk<StructSpecifier>(track, struct_specifier);
        }
        err("type specifier", "");
        return nullptr;
    }

    Ptr<Declarator> Parser::parse_declarator(bool inside_paramlist){
        Tracker track = tracker();
        Ptr<Declarator> declarator;

        if (ahead().tag() == Tok::Tag::P_Multiplication) {                  // Pointer Declarator
            lex();
            Ptr<Declarator> decl = parse_declarator();
            declarator = mk<PointerDeclarator>(track, std::move(decl));
        } else if (ahead().tag() == Tok::Tag::D_Parenthesis_L) {            // Paranthesized Declarator
            lex();
            if(type_follows() && inside_paramlist) goto parsing_paramlist;
            declarator = parse_declarator();
            expect(Tok::Tag::D_Parenthesis_R, "declarator");
        } else if (ahead().tag() == Tok::Tag::M_Id) declarator = mk<NamedDeclarator>(track, lex());  // Named Declarator
        else declarator=nullptr;

        if (ahead().tag() == Tok::Tag::D_Parenthesis_L) {                   // Parameter List
            lex();

parsing_paramlist:
            Ptrs<SpecifierDeclarator> paramList;
            do {
                if (type_follows()) paramList.emplace_back(parse_specifier_declarator(true));
                else {
                    err("type specifier","parameter declaration");
                    do lex(); while (ahead().tag() != Tok::Tag::P_Comma && ahead().tag() != Tok::Tag::M_EoF && ahead().tag() != Tok::Tag::D_Parenthesis_R);
                }
            } while (accept(Tok::Tag::P_Comma));
            
            expect(Tok::Tag::D_Parenthesis_R, "parameter list");
            return mk<FunctionDeclarator>(track, std::move(declarator), std::move(paramList));
        }
            
        return declarator;

    }


    //! =======================================================================================
    //! ====================================== Statement ======================================
    //! =======================================================================================

    Ptr<Stmt> Parser::parse_stmt(const char* ctxt, bool labeledStmt, bool nonblock){
        auto track = tracker();
        Ptr<Stmt> stmt;

        if(strcmp(ctxt, "") != 0) print_parsing(ctxt, "starting");
    
        switch (ahead().tag()) {
            case Tok::Tag::K_void:
            case Tok::Tag::K_char:
            case Tok::Tag::K_int: 
            case Tok::Tag::K_struct:{
                if (labeledStmt) {
                    err("statement", "labeled statement");
                    eat_rest_of_statement(true);
                    break;
                }
                if (nonblock) {
                    err("statement", "if or while statement");
                    eat_rest_of_statement(true);
                    break;
                }
                Ptr<SpecifierDeclarator> specifierDeclarator = parse_specifier_declarator();
                if (specifierDeclarator==nullptr) return nullptr;
                expect(Tok::Tag::P_Semicolon, "declaration");
                return mk<Declaration>(track, std::move(specifierDeclarator));
            }

            // Labeled Statement (First: identifier)
            case Tok::Tag::M_Id: {
                if (two_ahead().tag() == Tok::Tag::P_Colon) {           // labeled statement
                    auto label = lex(); 
                    lex();                                              //colon wegknuspern
                    if(ahead().tag() == Tok::Tag::D_Brace_R) {
                        err("expression", "labeled statement");
                        return createErrStmt(track, true, "expression", "labeled statement");
                    } else {                                            
                        auto labeled_statement = parse_stmt("labeled statement", true);
                        auto labeledStmt = mk<LabeledStmt>(track, label, std::move(labeled_statement));
                        return labeledStmt;
                    }
                } else {
                    goto expressionstatement;
                }
                break;
            }

            // compound statement (First: brace left)
            case Tok::Tag::D_Brace_L: {
                lex();
                print_parsing("compound statement list", "starting");

                Ptrs<Stmt> blockItems;

                while (ahead().tag() != Tok::Tag::D_Brace_R && ahead().tag()!=Tok::Tag::M_EoF){
                    blockItems.emplace_back(parse_stmt(""));
                }
                expect(Tok::Tag::D_Brace_R, "compound statement");
                print_parsing("compound statement list", "stopping");
                
                auto compoundStmt = mk<CompoundStmt>(track, std::move(blockItems));
                return compoundStmt;
            }
                
            // null statement (First: semicolon)
            case Tok::Tag::P_Semicolon: {
                lex(); 
                print_parsing("null statement"); 
                auto nullStmt = mk<NullStmt>(track);
                return nullStmt;
            }

            // selection statement (First: if)
            case Tok::Tag::K_if: {
                lex();
                expect(Tok::Tag::D_Parenthesis_L, "if statement");
                auto condition = parse_exp("if condition");
                expect(Tok::Tag::D_Parenthesis_R, "if statement");
                auto consequence = parse_stmt("if consequence", false, true);
                if (ahead().tag() == Tok::Tag::K_else) {
                    lex();
                    auto alternative = parse_stmt("if alternative (else)");
                    auto ifElseStmt = mk<IfElseStmt>(track, std::move(condition), std::move(consequence), std::move(alternative));
                    return ifElseStmt;
                }

                auto ifStmt = mk<IfStmt>(track, std::move(condition), std::move(consequence));
                return ifStmt;
            }
                
            // iteration statement (First: while)
            case Tok::Tag::K_while: {
                lex();
                print_parsing("while loop", "starting");
                if(!expect(Tok::Tag::D_Parenthesis_L, "while loop")) return createErrStmt(track, true, "while loop","stopping");
                auto condition = parse_exp("while loop");
                if(!expect(Tok::Tag::D_Parenthesis_R, "while loop")) return createErrStmt(track, true, "while loop","stopping");
                auto loop = parse_stmt("while loop", nonblock=true); //do something...

                auto whilestmt = mk<WhileStmt>(track, std::move(condition), std::move(loop));
                return whilestmt;
            }
                
            // jump statements (First: goto, continue, break, return)
            case Tok::Tag::K_continue: {
                lex(); 
                print_parsing("continue statement"); 
                if(!expect(Tok::Tag::P_Semicolon, "continue")) return createErrStmt(track, true, "", ""); 
                auto continuestmt = mk<ContinueStmt>(track); 
                return continuestmt;
            }
            case Tok::Tag::K_break: {
                lex(); 
                print_parsing("break statement"); 
                if(!expect(Tok::Tag::P_Semicolon, "break")) return createErrStmt(track, true, "", ""); 
                auto breakstmt = mk<BreakStmt>(track); 
                return breakstmt;
            }
            case Tok::Tag::K_goto: {
                lex();
                Tok label;
                if (ahead().tag() == Tok::Tag::M_Id) label = lex();
                else if(!expect(Tok::Tag::M_Id, "goto statement")) return createErrStmt(track, true, "", "");
                print_parsing("goto statement");
                if (!expect(Tok::Tag::P_Semicolon, "goto statement")) return createErrStmt(track, true, "", "");
                
                auto gotostmt = mk<GoToStmt>(track, label);
                return gotostmt;
            }

            case Tok::Tag::K_return: {
                lex();
                if (ahead().tag() == Tok::Tag::P_Semicolon){
                    lex();
                    auto emptyreturnstmt = mk<EmptyReturnStmt>(track);
                    return emptyreturnstmt;
                } else {
                    auto return_exp = parse_exp("return statement");
                    print_parsing("return statement");
                    if (!expect(Tok::Tag::P_Semicolon, "return statement")) return createErrStmt(track, true, "", "");
                    auto returnstmt = mk<ReturnStmt>(track, std::move(return_exp));
                    return returnstmt;
                }
                break;
            }
            
            default: {
expressionstatement:
                // expression-statement (First: expression) - fallback
                print_parsing("expression statement", "starting");
                auto exp = parse_exp("expression statement");
                

                
                if (!expect(Tok::Tag::P_Semicolon, "expression statement")) return createErrStmt(track, true, "expression statement", "stopping");

                print_parsing("expression statement", "stopping");
                auto expstmt = mk<ExpressionStmt>(track, std::move(exp));
                return expstmt;
            }
        }
        if(strcmp(ctxt, "") != 0) print_parsing(ctxt, "stopping");
        return stmt;
    }



    //! ========================================================================================
    //! ====================================== Expression ======================================
    //! ========================================================================================

    Ptr<Exp> Parser::parse_member_access(Tracker track, Ptr<Exp> lhs) {
        auto operation = lex().tag();
        if (ahead().tag() != Tok::Tag::M_Id) {
            expect(Tok::Tag::M_Id, "member access");
            eat_rest_of_statement(false);
            return mk<ErrExp>(track);                     //TODO: Return correct ptr
        }
        auto member_name = lex();
        //std::cout << "Member name: " << member_name.str() << " | Operator: " << Tok::tag2str(tag) << std::endl;
        return mk<MemberAccessExp>(track, operation, std::move(lhs), member_name);  // ToDo: Create (Exp-)Node with lhs (union or struct we want to access), tag (method) and member_name
    }

    // Main Loop
    Ptr<Exp> Parser::parse_exp(const char* ctxt, Tok::Prec p) {
        auto track = tracker();
        //std::cout << "Parse Expression: \t\t" << Tok::tag2str(ahead().tag()) << " " << ahead().str() << " | " << ctxt << std::endl;
        auto lhs = parse_primary_expr(ctxt);            // Just start
        //std::cout << "Left-Hand side parsed" << std::endl;
        //std::cout << "Parse Expression: \t\t" << Tok::tag2str(ahead().tag()) << " " << ahead().str() << " | " << ctxt << std::endl << std::endl;
        
        while (true) {
            switch (ahead().tag()) {
                // This assumes that all postfix expressions bind strongest.

                // Brackets (array subscripting)
                case Tok::Tag::D_Bracket_L:{
                    print_parsing("array subscripting", "starting");
                    lex();
                    auto index = parse_exp("array subscription");
                    expect(Tok::Tag::D_Bracket_R, "array subscription");
                    print_parsing("array subscripting", "stopping");

                    lhs = mk<ArrayExp>(track, std::move(lhs), std::move(index));
                    continue;
                }
                    
                // Dot (member access) and Right Arrow (member access through pointer ==> a->b is the same as (*a).b)
                case Tok::Tag::P_Arrow_R:
                case Tok::Tag::P_Dot: {
                    lhs = parse_member_access(track, std::move(lhs));
                    continue;
                }
                    
                // func() (function call)
                case Tok::Tag::D_Parenthesis_L: {
                    print_parsing("function call", "starting");
                    Ptrs<Exp> parameters = parse_expr_list("function call");
                    print_parsing("function call", "stopping");                   

                    lhs = mk<FuncCallExp>(track, std::move(lhs), std::move(parameters));
                    continue;
                }

                // var++, var-- (postfix increment and decrement)
                case Tok::Tag::P_Increment:
                case Tok::Tag::P_Decrement: {
                    lhs = mk<PostfixExp>(track, std::move(lhs), lex());
                    continue; 
                }      
                
                default: break;
            }

            // If operator in lookahead has less left precedence: reduce.
            // If lookahead isn't a valid infix operator, we will see Prec::Error.
            // This is less than all other prec levels.
            auto ahead_tag = ahead().tag();
            auto q = Tok::tag2prec_l(ahead_tag);
            if (q < p) break;      
            if (ahead_tag == Tok::Tag::P_Inline_If){ // Ternary Expression
                lex();
                Ptr<Exp> consequence, alternative;
                
                if (ahead().tag() == Tok::Tag::P_Semicolon || ahead().tag()==Tok::Tag::M_EoF) {
                        err("expression", "alternative of ternary expression");
                        lhs = createErrExp(track, false, "", "");
                } else {
                    consequence = parse_exp("consequence of a ternary expression");
                    if (!expect(Tok::Tag::P_Colon, "ternary expression")) lhs = createErrExp(track, false, "", "");
                    else {
                        if (ahead().tag() == Tok::Tag::P_Semicolon || ahead().tag()==Tok::Tag::M_EoF) {
                            err("expression", "alternative of ternary expression");
                            lhs = createErrExp(track, false, "", "");
                        } else alternative = parse_exp("alternative of a ternary expression", Tok::Prec::Conditional);
                    }
                }
                lhs = mk<TernaryExp>(track, std::move(lhs), std::move(consequence), std::move(alternative));
                
            } else { // Binary Expression
                auto operation = lex();
                //std::cout << Tok::tag2str(ahead().tag()) << std::endl;
                auto rhs = parse_exp("right-hand side of a binary expression", Tok::tag2prec_r(operation.tag()));
                
                auto infixExp = mk<InfixExp>(track, std::move(lhs), operation, std::move(rhs));
                lhs = std::move(infixExp);
            }
        }
        return lhs;
    }

    Ptr<Exp> Parser::parse_primary_expr(const char* ctxt) {
        //std::cout << "Parse Primary Expression: \t" << Tok::tag2str(ahead().tag()) << " " << ahead().str() << " | " << ctxt << std::endl;
        auto track = tracker();
        
        switch (ahead().tag()) {
            // see c reference 6.5.3 Unary Operators
            case Tok::Tag::P_Bitwise_And:               // Address-of       (&var)
            case Tok::Tag::P_Multiplication:            // Dereference      (*var)
            case Tok::Tag::P_Logical_Not:               // Logical Not      (!var)
            case Tok::Tag::P_Bitwise_Not:               // Bitwise Not      (~var)
            case Tok::Tag::P_Addition:                  // Unary Plus       (+var)
            case Tok::Tag::P_Substraction:              // Unary Minus      (-var (= -1 * var))   
            case Tok::Tag::P_Increment:                 // Prefix increment (++var)
            case Tok::Tag::P_Decrement:{                // Prefix decrement (--var)   
                auto prefix = lex();
                auto rhs = parse_exp("right-hand side of a unary expression", Tok::Prec::Unary);    // This assumes that all prefix expressions bind with precedence level "Unary".
                auto prefixExp = mk<PrefixExp>(track, prefix, std::move(rhs));
                return prefixExp;
            }           

            // "sizeof int" <-> "sizeof x" 
            // sizeof unary-expression <-> sizeof ( type-name )
            case Tok::Tag::K_sizeof: { // TODO: return expression that represents a sizeof expression // TODO: 2-look-ahead
                lex();
                if (two_ahead().tag() == Tok::Tag::K_const || two_ahead().tag() == Tok::Tag::K_char || two_ahead().tag() == Tok::Tag::K_int){
                    if (!expect(Tok::Tag::D_Parenthesis_L, "sizeof (type)")) return createErrExp(track, false, "", "");
                    Tok typetok = lex(); 
                    if (!expect(Tok::Tag::D_Parenthesis_R, "sizeof (type)")) return createErrExp(track, false, "", "");
                    return mk<SizeOfTypeExp>(track, typetok);
                } else {
                    auto sizeOfUnaryExp = parse_exp("sizeof unary-expression", Tok::Prec::Unary);
                    return mk<SizeOfUnaryExp>(track, std::move(sizeOfUnaryExp));
                }
            } 

            case Tok::Tag::C_Integer:{
                auto integerNode = mk<Integer>(track, lex().value());
                return integerNode;
            }
            case Tok::Tag::C_Character:{
                std::string dd = lex().str();
                // std::cout << "CHAR: " << dd << std::endl;
                auto characterNode = mk<Character>(track, dd); 
                return characterNode;
            }
            case Tok::Tag::M_Id: {
                auto identifierNode = mk<Identifier>(track, lex().str());
                return identifierNode;
            }
            case Tok::Tag::S_Literal: {
                std::string s = lex().str();
                // std::cout << "LITERAL: " << s << std::endl;
                auto literalNode = mk<Literal>(track, s);
                return literalNode;
            }
            case Tok::Tag::D_Parenthesis_L: {
                lex();
                auto res = parse_exp("parenthesized expression");
                if (!expect(Tok::Tag::D_Parenthesis_R, "parenthesized expression")) return createErrExp(track, false, "","");
                return res;
            }
            default: {
                err("expression", ctxt);
                auto errExp = createErrExp(track, false, "","");
                return errExp;
            }                           
                
        }
    }
}