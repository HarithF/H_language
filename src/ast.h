#ifndef PROG_AST_H
#define PROG_AST_H

#include <algorithm>
#include <memory>
#include <ostream>
#include <unordered_set>
#include <map>
#include <vector>
#include <iostream>

#include "loc.h"
#include "tok.h"

namespace H {


template<class T> using Ptr = std::unique_ptr<T>;
template<class T> using Ptrs = std::vector<Ptr<T>>;

template<class T, class... Args>
Ptr<T> mk(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

using Vars = std::unordered_set<std::string>;

class Declaration;
class Sema;
class Declarator;
class Specifier;
class CompoundStmt;
class SpecifierDeclarator;
class LabeledStmt;

inline std::map<std::string, LabeledStmt*> labels;



class Type {
    public:
        Type(){}
        virtual ~Type(){}

        virtual std::string str() const = 0;
        virtual bool isComplete() const = 0;
        bool isUnqualified() const { return true; }
        virtual bool isScalar() const = 0;

};

// Scalar Types
class ArithmeticType: public Type {
    public:
        ArithmeticType()
        : Type()
        {}

        virtual std::string str() const = 0;
        bool isComplete() const override { return true; }
        bool isScalar() const override { return true; }
};

class IntType: public ArithmeticType {
    public:
        IntType()
        : ArithmeticType()
        {}

        std::string str() const { return "int";}
};

class CharType: public ArithmeticType {
    public:
        CharType()
        : ArithmeticType()
        {}

        std::string str() const { return "char";}
};

class PointerType: public Type {
    public:
        PointerType(Type* pointee)
        : Type()
        , pointee_(pointee)
        {}

        std::string str() const { return "pointer to " + pointee()->str(); }
        Type* pointee() const { return pointee_; }
        bool isComplete() const override { return true; }
        bool isScalar() const override { return true; }

    private:
        Type* pointee_;

};


// Aggregated Types
class StructType: public Type {
    public:
        StructType()
        : Type()
        {}

        std::string str() const { return "struct";}
        bool isComplete() const override { return structComplete_; }
        void completed() { structComplete_ = true; }
        bool isScalar() const override { return false; }

    private:
        bool structComplete_ = false;
};

class ArrayType: public Type {
    public:
        ArrayType(Type* elementType)
        : Type()
        , elementType_(elementType)
        {}

        std::string str() const { return "array of" + elementType()->str();}
        Type* elementType() const { return elementType_; }

        bool isComplete() const override { return arrayComplete_; }
        void completed() { arrayComplete_ = true; }
        bool isScalar() const override { return false; }

    private:
        bool arrayComplete_ = false;
        Type* elementType_;
};


// Misc. Types
class FunctionType: public Type {
    public:
        FunctionType(Type* returnType)
        : Type()
        , returnType_(returnType)
        {}

        std::string str() const { return "function returning " + returnType()->str(); }
        Type* returnType() const { return returnType_; }
        bool isComplete() const override { return returnType()->isComplete(); }
        bool isScalar() const override { return false; }

    private:
        Type* returnType_;

};

class VoidType: public Type {
    public:
        VoidType()
        : Type()
        {}

    std::string str() const { return "void"; }
    bool isComplete() const override {return false; }
    bool isScalar() const override { return false; }
};

class ErrorType: public Type {
    public:
        ErrorType()
        : Type()
        {}

        std::string str() const { return "error";}
        bool isComplete() const override {return false; }
        bool isScalar() const override { return false; }
};



//! =================================================
//! ================ Root / Basic ===================
//! =================================================

class ASTNode {
    public:
        ASTNode(Loc loc)
            : loc_(loc)
        {}
        virtual ~ASTNode() {}

        Loc loc() const { return loc_; }

        void dump() const;
        virtual std::ostream& stream(std::ostream& o) const = 0;
        void check(Sema&);

    private:
        Loc loc_;
};

class Exp : public ASTNode{
public:
    Exp(Loc loc)
        : ASTNode(loc)
    {}

    virtual std::ostream& stream(std::ostream& o) const = 0;
    virtual Type* check (Sema&) = 0;


    Type* type_;
};

class Stmt : public ASTNode{
public:
    Stmt(Loc loc)
        : ASTNode(loc)
    {}

    virtual std::ostream& stream(std::ostream& o) const = 0;
    virtual void check (Sema&) = 0;
};


//! =================================================
//! ================= Specifiers ====================
//! =================================================

class Specifier : public ASTNode {
    public:
        Specifier(Loc loc, Tok tokType)
        : ASTNode(loc)
        , tokType_(tokType)
        {
            if (tokType_.str() == "struct") type_=mk<StructType>();
            else if (tokType_.str() == "int") type_= mk<IntType>();
            else if (tokType_.str() == "void") type_= mk<VoidType>();
            else if (tokType_.str() == "char") type_= mk<CharType>();
            else type_= mk<ErrorType>();
        }

        // Direct Getters
        std::string typeString() const { return tokType_.str(); }

        Type* type() const { return type_.get(); }

        // AST-Functions
        virtual std::ostream& stream(std::ostream& o) const = 0;
    
    private:
        Tok tokType_;
        Ptr<Type> type_;

};

class PrimitiveSpecifier : public Specifier {                        // Class for handling the primative specifiers (void, char, int)
    public:
        PrimitiveSpecifier(Loc loc, Tok type)
            : Specifier(loc, type)
        {}

        // AST-Functions
        std::ostream& stream(std::ostream& o) const override;
};


class StructSpecifier : public Specifier {                           // Class for handling the struct specifier
    public:
        StructSpecifier(Loc loc, Tok type)
            : Specifier(loc, type)
        {}
        StructSpecifier(Loc loc, Tok type, Tok structIdentifier)
            : Specifier(loc, type)
            , structIdentifier_(structIdentifier)
            , declarationListSet_(false)
        {}
        StructSpecifier(Loc loc, Tok type, Ptrs<SpecifierDeclarator>&& structDeclarationList)
            : Specifier(loc, type)
            , structDeclarationList_(std::move(structDeclarationList))
            , declarationListSet_(true)
        {}
        StructSpecifier(Loc loc, Tok type, Tok structIdentifier, Ptrs<SpecifierDeclarator>&& structDeclarationList)
            : Specifier(loc, type)
            , structIdentifier_(structIdentifier)
            , structDeclarationList_(std::move(structDeclarationList))
            , declarationListSet_(true)
        {}

        // Direct Getters
        Tok structIdentifier() const { return structIdentifier_; }
        std::string structIdentifierString() const { return structIdentifier_.str(); }
        bool declarationListSet() const {return declarationListSet_; }

        const Ptrs<SpecifierDeclarator>& structDeclarationList() const { return structDeclarationList_; }
        size_t num_structDeclarations() const { return structDeclarationList_.size(); }
        SpecifierDeclarator* structDeclaration(size_t i) const { return structDeclarationList_[i].get(); }

        // AST-Functions
        std::ostream& stream(std::ostream& o) const override;

    private: 
        Tok structIdentifier_;
        Ptrs<SpecifierDeclarator> structDeclarationList_;
        bool declarationListSet_;
};



//! =================================================
//! ================= Declarator ====================
//! =================================================

class Declarator : public ASTNode {
    public:
        Declarator(Loc loc, bool abstract)
            : ASTNode(loc)
            , abstract_(abstract)
        {}

        // Direct Getters
        bool abstract() { return abstract_; }
        virtual std::string name() const = 0;
        virtual const Ptrs<SpecifierDeclarator>& parameterList() const = 0;

        // AST-Functions
        virtual std::ostream& stream(std::ostream& o) const = 0;

        virtual Type* type(Type* specifierType) const = 0;

    private: 
        bool abstract_;
};

class NamedDeclarator : public Declarator {
    public:
        NamedDeclarator(Loc loc, Tok identifier, bool abstract=false)
            : Declarator(loc, abstract)
            , identifier_(identifier)
        {}

        // Direct Getters
        Tok identifier() const {return identifier_; }
        std::string name() const { return identifier_.str(); }
        const Ptrs<SpecifierDeclarator>& parameterList() const {return Ptrs<SpecifierDeclarator>(); };

        // AST-Functions
        std::ostream& stream(std::ostream& o) const;

        Type* type(Type* specifierType) const override { return specifierType; }

    private: 
        bool abstract_;
        Tok identifier_;
};

class FunctionDeclarator : public Declarator {
    public:
        FunctionDeclarator(Loc loc, Ptr<Declarator>&& declarator, Ptrs<SpecifierDeclarator>&& parameterList, bool abstract=false)
            : Declarator(loc, abstract)
            , declarator_(std::move(declarator))
            , parameterList_(std::move(parameterList))
        {}

        // Direct Getters
        Declarator* declarator() const { return declarator_.get(); }
        std::string name() const { return declarator()->name(); }

        const Ptrs<SpecifierDeclarator>& parameterList() const { return parameterList_; }
        size_t num_parameters() const { return parameterList_.size(); }
        const SpecifierDeclarator* parameter(size_t i) const { return parameterList_[i].get(); }

        // AST-Functions
        std::ostream& stream(std::ostream& o) const;

        Type* type(Type* specifierType) const override { return new FunctionType( declarator_->type(specifierType)); }        

    private: 
        Ptr<Declarator> declarator_;
        Ptrs<SpecifierDeclarator> parameterList_;
};


class PointerDeclarator : public Declarator {
    public:
        PointerDeclarator(Loc loc, Ptr<Declarator>&& declarator, bool abstract=false)
            : Declarator(loc, abstract)
            , declarator_(std::move(declarator))
        {}

        // Direct Getters
        Declarator* declarator() const { return declarator_.get(); }
        std::string name() const { if (declarator()!=nullptr) return declarator()->name(); else return "";}
        const Ptrs<SpecifierDeclarator>& parameterList() const {return declarator()->parameterList(); };

        // AST-Functions
        std::ostream& stream(std::ostream& o) const;

        Type* type(Type* specifierType) const override {
            if (declarator_ && declarator_->type(specifierType)) return new PointerType(declarator_->type(specifierType)); 
            else return new ErrorType();
        }

    private: 
        Ptr<Declarator> declarator_;
};

//! =================================================
//! ================ Statements =====================
//! =================================================

class ExpressionStmt : public Stmt {
    public:
        ExpressionStmt(Loc loc, Ptr<Exp>&& exp)
            : Stmt(loc)
            , exp_(std::move(exp))
        {}

        Exp* exp() const { return exp_.get(); }

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Ptr<Exp> exp_;
};

class EmptyReturnStmt : public Stmt {
    public:
        EmptyReturnStmt(Loc loc)
            : Stmt(loc)
        {}
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;
};

class ReturnStmt : public Stmt {
    public:
        ReturnStmt(Loc loc, Ptr<Exp>&& exp)
            : Stmt(loc)
            , exp_(std::move(exp))
        {}

        Exp* exp() const { return exp_.get(); }

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Ptr<Exp> exp_;
};

class GoToStmt : public Stmt {
    public:
        GoToStmt(Loc loc, Tok tok)
            : Stmt(loc)
            , tok_(tok)
        {}

        // Direct Getter
        Tok tok() const { return tok_; }
        std::string gotoLabel() const { return tok_.str(); } 

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Tok tok_;
};

class BreakStmt : public Stmt {
    public:
        BreakStmt(Loc loc)
            : Stmt(loc)
        {}
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;
};

class ContinueStmt : public Stmt {
    public:
        ContinueStmt(Loc loc)
            : Stmt(loc)
        {}
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;
};

class WhileStmt : public Stmt {
    public:
        WhileStmt(Loc loc, Ptr<Exp>&& condition, Ptr<Stmt>&& loop)
            : Stmt(loc)
            , condition_(std::move(condition))
            , loop_(std::move(loop))
        {}

        Exp* condition() const { return condition_.get(); }
        Stmt* loop() const { return loop_.get(); }
        
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Ptr<Exp> condition_;
        Ptr<Stmt> loop_;
};

class IfElseStmt : public Stmt {
    public:
        IfElseStmt(Loc loc, Ptr<Exp>&& condition, Ptr<Stmt>&& consequence, Ptr<Stmt>&& alternative)
            : Stmt(loc)
            , condition_(std::move(condition))
            , consequence_(std::move(consequence))
            , alternative_(std::move(alternative))
        {}

        Exp* condition() const { return condition_.get(); }
        Stmt* consequence() const { return consequence_.get(); }
        Stmt* alternative() const { return alternative_.get(); }

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Ptr<Exp> condition_;
        Ptr<Stmt> consequence_;
        Ptr<Stmt> alternative_;
};

class IfStmt : public Stmt {
    public:
        IfStmt(Loc loc, Ptr<Exp>&& condition, Ptr<Stmt>&& consequence)
            : Stmt(loc)
            , condition_(std::move(condition))
            , consequence_(std::move(consequence))
        {}

        Exp* condition() const { return condition_.get(); }
        Stmt* consequence() const { return consequence_.get(); }

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Ptr<Exp> condition_;
        Ptr<Stmt> consequence_;
};

class NullStmt : public Stmt {
    public:
        NullStmt(Loc loc)
            : Stmt(loc)
        {}
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;
};

class CompoundStmt : public Stmt {
    public:
        CompoundStmt(Loc loc, Ptrs<Stmt>&& blockItems)
            : Stmt(loc)
            , blockItems_(std::move(blockItems))
        {}

        const Ptrs<Stmt>& blockItems() const { return blockItems_; }
        size_t num_blockItems() const { return blockItems_.size(); }
        Stmt* blockItem(size_t i) const { return blockItems_[i].get(); }

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Ptrs<Stmt> blockItems_;
};

class LabeledStmt : public Stmt {
    public:
        LabeledStmt(Loc loc, Tok label, Ptr<Stmt>&& statement)
            : Stmt(loc)
            , label_(label)
            , statement_(std::move(statement))
            
        {
            if (labels.find(label.str()) != labels.end()){
                loc.err() << "Duplicate label '" << label.str() << "'!" << loc.endErr();
            } else labels[label.str()] = this;
        }

        Stmt* statement() const { return statement_.get(); }
        Tok label() const { return label_; }
        std::string labelString() const { return label_.str(); }

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Tok label_;
        Ptr<Stmt> statement_;
};


//! =================================================
//! ================= Base Stuff ====================
//! =================================================


class SpecifierDeclarator : public ASTNode {
    public:
        SpecifierDeclarator(Loc loc, Ptr<Specifier>&& specifier, Ptr<Declarator>&& declarator)
        : ASTNode(loc)
        , specifier_(std::move(specifier))
        , declarator_(std::move(declarator))
        {}

        SpecifierDeclarator(Loc loc, Ptr<Specifier>&& specifier)
        : ASTNode(loc)
        , specifier_(std::move(specifier))
        {}

        // Direct Getter
        Specifier* specifier() const { return specifier_.get(); }
        Declarator* declarator() const { return declarator_.get(); }

        // Indirect Getter
        std::string name() const { if (declarator()!=nullptr) return declarator()->name(); else return "";}
        //Type* type() const { return specifier()->type();}
        Type* type() const { if(declarator()!=nullptr) return declarator()->type(specifier()->type()); else return specifier()->type(); }
        std::string typeString() const { return specifier()->typeString();}
        const Ptrs<SpecifierDeclarator>& parameterList() {return declarator()->parameterList(); };

        // AST-Functions
        std::ostream& stream(std::ostream& o) const override;

    private:
        Ptr<Specifier> specifier_;
        Ptr<Declarator> declarator_;
};

class ExternalDeclaration : public ASTNode {
    public:
        ExternalDeclaration(Loc loc)
        : ASTNode(loc)
        {}

        ExternalDeclaration(Loc loc, Ptr<SpecifierDeclarator>&& specifierDeclarator)
        : ASTNode(loc)
        , specifierDeclarator_(std::move(specifierDeclarator))
        {}

        ExternalDeclaration(Loc loc, Ptr<SpecifierDeclarator>&& specifierDeclarator, Ptr<Stmt>&& functionBody)
        : ASTNode(loc)
        , specifierDeclarator_(std::move(specifierDeclarator))
        , functionBody_(std::move(functionBody))
        {}

        // Direct Getter
        SpecifierDeclarator* specifierDeclarator() const { return specifierDeclarator_.get(); }
        Stmt* functionBody() const { return functionBody_.get(); }

        // AST-Functions
        std::ostream& stream(std::ostream& o) const override;
        void check(Sema&);

    private:
        Ptr<SpecifierDeclarator> specifierDeclarator_;
        Ptr<Stmt> functionBody_;
};


class TranslationUnit : public ASTNode {
    public:
        TranslationUnit(Loc loc, Ptrs<ExternalDeclaration>&& external_declarations)
            : ASTNode(loc)
            , external_declarations_(std::move(external_declarations))
        {}

        // Direct Getter
        const Ptrs<ExternalDeclaration>& external_declarations() const { return external_declarations_; }
        size_t num_ext_declarations() const { return external_declarations_.size(); }
        ExternalDeclaration* external_declaration(size_t i) const { return external_declarations_[i].get(); }


        // AST-Function
        std::ostream& stream(std::ostream& o) const override;
        void check(Sema&);

    private:
        Ptrs<ExternalDeclaration> external_declarations_;
};

class Declaration : public Stmt {
    public:
        Declaration(Loc loc, Ptr<SpecifierDeclarator>&& specifierDeclarator)
            : Stmt(loc)
            , specifierDeclarator_(std::move(specifierDeclarator))
        {}
        Declaration(Loc loc)                                                // Empty Declaration for ErrDeclarations
            : Stmt(loc)
        {}

        // Direct Getter
        SpecifierDeclarator* specifierDeclarator() const { return specifierDeclarator_.get(); }

        // Indirect Getter
        std::string name() { return specifierDeclarator()->name(); }
        Type* type() { return specifierDeclarator()->type(); }
        std::string typeString() { return specifierDeclarator()->typeString(); }

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        Ptr<SpecifierDeclarator> specifierDeclarator_;
};



//! =================================================
//! ================ Expressions ====================
//! =================================================
class InfixExp : public Exp {
    public:
        InfixExp(Loc loc, Ptr<Exp>&& lhs, Tok operation, Ptr<Exp>&& rhs)
            : Exp(loc)
            , lhs_(std::move(lhs))
            , rhs_(std::move(rhs))
            , operation_(operation)
        {}

        Exp* lhs() const { return lhs_.get(); }
        Exp* rhs() const { return rhs_.get(); }
        Tok operation() const { return operation_; }
        
        
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        Type* check(Sema&) override;

    private:
        Ptr<Exp> lhs_;
        Ptr<Exp> rhs_;
        Tok operation_;
};

class TernaryExp : public Exp {
    public:
        TernaryExp(Loc loc, Ptr<Exp>&& condition, Ptr<Exp>&& consequence, Ptr<Exp>&& alternative)
            : Exp(loc)
            , condition_(std::move(condition))
            , consequence_(std::move(consequence))
            , alternative_(std::move(alternative))
        {}

        Exp* condition() const { return condition_.get(); }
        Exp* consequence() const { return consequence_.get(); }
        Exp* alternative() const { return alternative_.get(); }
        
    // AST-Functions
    std::ostream& stream(std::ostream&) const override; 
    Type* check(Sema&) override;

    private:
        Ptr<Exp> condition_;
        Ptr<Exp> consequence_;
        Ptr<Exp> alternative_;
};

class PrefixExp : public Exp {
    public:
        PrefixExp(Loc loc, Tok prefix, Ptr<Exp>&& operand)
            : Exp(loc)
            , prefix_(prefix)
            , operand_(std::move(operand))
        {}

        
        Tok prefix() const { return prefix_; }
        std::string prefixString() const { return prefix_.str(); }
        Exp* operand() const { return operand_.get(); }
        
    // AST-Functions
    std::ostream& stream(std::ostream&) const override; 
    Type* check(Sema&) override;

    private:
        Tok prefix_;
        Ptr<Exp> operand_;
};

class MemberAccessExp : public Exp {
    public:
        MemberAccessExp(Loc loc, Tok::Tag operation, Ptr<Exp>&& object, Tok member_name)
            : Exp(loc)
            , operation_(operation)
            , object_(std::move(object))
            , member_name_(member_name.str())
        {}

        
        Tok::Tag operation() const { return operation_; }
        Exp* object() const { return object_.get(); }
        std::string member_name() const { return member_name_; }
        
    // AST-Functions
    std::ostream& stream(std::ostream&) const override; 
    Type* check(Sema&) override;

    private:
        Tok::Tag operation_;
        Ptr<Exp> object_;
        std::string member_name_;
};

class ArrayExp : public Exp {
    public:
        ArrayExp(Loc loc, Ptr<Exp>&& object, Ptr<Exp>&& index)
            : Exp(loc)
            , object_(std::move(object))
            , index_(std::move(index))
        {}

        Exp* object() const { return object_.get(); }
        Exp* index() const { return index_.get(); }
        
    // AST-Functions
    std::ostream& stream(std::ostream&) const override; 
    Type* check(Sema&) override;

    private:
        Ptr<Exp> object_;
        Ptr<Exp> index_;
};

class FuncCallExp : public Exp {
public:
    FuncCallExp(Loc loc, Ptr<Exp>&& func, Ptrs<Exp>&& parameters)
        : Exp(loc)
        , func_(std::move(func))
        , parameters_(std::move(parameters))
    {}

    Exp* func() const { return func_.get(); }
    const Ptrs<Exp>& parameters() const { return parameters_; }
    size_t num_parameters() const { return parameters_.size(); }
    const Exp* parameter(size_t i) const { return parameters_[i].get(); }

    // AST-Functions
    std::ostream& stream(std::ostream&) const override; 
    Type* check(Sema&) override;

private:
    Ptr<Exp> func_;
    Ptrs<Exp> parameters_;
};

class SizeOfTypeExp : public Exp {
public:
    SizeOfTypeExp(Loc loc, Tok typeTok)
        : Exp(loc)
        , typeTok_(typeTok)
    {}

    Tok typeTok() const { return typeTok_; }
    std::string typeString() const {return typeTok_.str(); }

    // AST-Functions
    std::ostream& stream(std::ostream&) const override; 
    Type* check(Sema&) override;

private:
    Tok typeTok_;
};

class SizeOfUnaryExp : public Exp {
public:
    SizeOfUnaryExp(Loc loc, Ptr<Exp>&& exp)
        : Exp(loc)
        , exp_(std::move(exp))
    {}

    Exp* exp() const { return exp_.get(); }

    // AST-Functions
    std::ostream& stream(std::ostream&) const override; 
    Type* check(Sema&) override;

private:
    Ptr<Exp> exp_;
};

class PostfixExp : public Exp {
    public:
        PostfixExp(Loc loc, Ptr<Exp>&& operand, Tok postfix)
            : Exp(loc)
            , postfix_(postfix)
            , operand_(std::move(operand))
        {}

        
        Tok postfix() const { return postfix_; }
        std::string postfixString() const {return postfix_.str(); }
        Exp* operand() const { return operand_.get(); }
        
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        Type* check(Sema&) override;

    private:
        Tok postfix_;
        Ptr<Exp> operand_;
};


//! =================================================
//! ============== Basic Expressions ================
//! =================================================

class Identifier : public Exp {
    public:
        Identifier(Loc loc, std::string name)
            : Exp(loc)
            , name_(name)
        {}

        std::string name() const { return name_; }
        
        
        SpecifierDeclarator* specifierDeclarator() {return specifierDeclarator_;}
        void setSpecifierDeclarator(SpecifierDeclarator* ptr_specifierDeclarator) {specifierDeclarator_ = ptr_specifierDeclarator;}

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        Type* check(Sema&);

    private:
        std::string name_;
        SpecifierDeclarator* specifierDeclarator_;
};

class Integer : public Exp {
    public:
        Integer(Loc loc, int value)
            : Exp(loc)
            , value_(value)
        {}

        int value() const { return value_; }
        
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        Type* check(Sema&);

    private:
        int value_;
};

class Character : public Exp {
    public:
        Character(Loc loc, std::string value)
            : Exp(loc)
            , value_(value)
        {}

        std::string value() const { return value_; }
        
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        Type* check(Sema&);

    private:
        std::string value_;
};

class Literal : public Exp {
    public:
        Literal(Loc loc, std::string value)
            : Exp(loc)
            , value_(value)
        {}

        // Direct Getter
        std::string value() const { return value_; }
        
        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        Type* check(Sema&);

    private:
        std::string value_;
};




//! =================================================
//! ============= Err / Exp-Stmt-Decl ===============
//! =================================================


class ErrExp : public Exp {
    public:
        ErrExp(Loc loc)
            : Exp(loc)
        {}

        std::ostream& stream(std::ostream& o) const override;
        Type* check(Sema& sema) override;

    private:
        std::string name_;
};


class ErrStmt : public Stmt {
    public:
        ErrStmt(Loc loc)
            : Stmt(loc)
        {}

        // AST-Functions
        std::ostream& stream(std::ostream&) const override; 
        void check(Sema& sema) override;

    private:
        std::string name_;
    };

class ErrDecl : public ExternalDeclaration {
    public:
        ErrDecl(Loc loc)
            : ExternalDeclaration(loc)
        {}

        std::ostream& stream(std::ostream& o) const override;
};



}
#endif