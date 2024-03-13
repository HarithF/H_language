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