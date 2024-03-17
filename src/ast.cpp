#include "ast.h"
#include <typeinfo>
#include <iostream>

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

namespace H {


int indentlvl = 0;
void newIndent(int i = 0)
{   
    putchar('\n');
    for (i = 0; i < indentlvl; i++)
        putchar('\t');
}

void newIndentDumpBlockItem(Stmt* blockItem)
{   
    if (dynamic_cast<LabeledStmt*>(blockItem)) putchar('\n');
    else newIndent();
    blockItem->dump();
}

void newIndentDumpStmt(Stmt* statement, int temporaryIndentAdjust = 0)
{   
    indentlvl = indentlvl+temporaryIndentAdjust;
    putchar('\n');
    if (dynamic_cast<LabeledStmt*>(statement)) statement->dump();
    else {
        for (int i = 0; i < indentlvl; i++) putchar('\t');
        statement->dump();
    }
    indentlvl = indentlvl-temporaryIndentAdjust;
}



//! ========================================================================================================
//! ================= Semantic Analysis ====================================================================
//! ========================================================================================================



//! ========================================================================================================
//! ================= Stream/Dump ==========================================================================
//! ========================================================================================================


void ASTNode::dump() const {
    stream(std::cout);
}


//! =================================================
//! ================= Specifiers ====================
//! =================================================

std::ostream& PrimitiveSpecifier::stream(std::ostream& o) const {
    return o << typeString();
}

std::ostream& StructSpecifier::stream(std::ostream& o) const {
    o << "struct";
    if (structIdentifierString()!="") o << " " << structIdentifierString();
    
    if (declarationListSet()) {
        if (num_structDeclarations()==0) {
            o << " {}";
        } else {
            newIndent();
            o << "{";
            newIndent(++indentlvl);
            for (size_t i = 0; i < num_structDeclarations(); i++)
            {
                structDeclaration(i)->dump();
                o << ";";
                if (i+1<num_structDeclarations()) newIndent();
            }
            newIndent(--indentlvl);
            o<<"}";
        }
    }
        
    return o;
}


//! =================================================
//! ================= Declarator ====================
//! =================================================

std::ostream& NamedDeclarator::stream(std::ostream& o) const {
    return o << name();
}

std::ostream& FunctionDeclarator::stream(std::ostream& o) const {
    o << "(";
    if (declarator()!=nullptr){
        declarator()->dump();
            o << "(";
    }
    
    for (size_t i = 0; i < num_parameters(); i++) {
        parameter(i)->dump();
        if (i+1 < num_parameters()) o << ", ";
    }
    if (declarator()!=nullptr) o<<")";
    return o << ")";
}

std::ostream& PointerDeclarator::stream(std::ostream& o) const {
    o << "(*";
    if (declarator() != nullptr){
        declarator()->dump();
    }
    return o<<")";
}

//! =================================================
//! ================= Base Stuff ====================
//! =================================================

std::ostream& SpecifierDeclarator::stream(std::ostream& o) const {
    specifier()->dump();
    if (declarator() != nullptr){
        o<<" ";
        declarator()->dump();
    } 
    return o;
}

std::ostream& ExternalDeclaration::stream(std::ostream& o) const {
    if (!specifierDeclarator()) return o<<"error";
    specifierDeclarator()->dump();
    if (functionBody() != nullptr){
        o<<"\n";
        functionBody()->dump();
    } else {
        o << ";";
    }
    return o;
}

std::ostream& TranslationUnit::stream(std::ostream& o) const {
    
    for (size_t i = 0; i < num_ext_declarations(); i++) {
        external_declaration(i)->dump();
        newIndent();
        if (i+1<num_ext_declarations()) newIndent();
    }        
    return o;
}

std::ostream& Declaration::stream(std::ostream& o) const {
    specifierDeclarator()->dump();    
    return o<<";";
}




//! =================================================
//! ================ Statements =====================
//! =================================================

std::ostream& ExpressionStmt::stream(std::ostream& o) const {
    exp()->dump();
    return o<<";";
}

std::ostream& EmptyReturnStmt::stream(std::ostream& o) const {
    return o<<"return;";
}

std::ostream& ReturnStmt::stream(std::ostream& o) const {
    o << "return ";
    exp()->dump();
    return o<<";";
}

std::ostream& GoToStmt::stream(std::ostream& o) const {
    return o << "goto " << gotoLabel() << ";";
}

std::ostream& BreakStmt::stream(std::ostream& o) const {
    return o<<"break;";
}

std::ostream& ContinueStmt::stream(std::ostream& o) const {
    return o<<"continue;";
}

std::ostream& WhileStmt::stream(std::ostream& o) const {
    o<<"while (";
    condition()->dump();
    o<<")";
    if (dynamic_cast<CompoundStmt*>(loop())) {
        o << " ";
        loop()->dump();
    } else {
        indentlvl++;
        newIndentDumpStmt(loop());
        indentlvl--;
    }
    return o;
}

std::ostream& IfElseStmt::stream(std::ostream& o) const {
    o<<"if (";
    condition()->dump();
    o<<")";
    if (dynamic_cast<CompoundStmt*>(consequence())) {
        o << " ";
        consequence()->dump();
        o << " ";
    } else {
        newIndentDumpStmt(consequence(), +1);
        newIndent();
    }
    o<<"else";
    if (dynamic_cast<IfStmt*>(alternative()) || dynamic_cast<IfElseStmt*>(alternative()) || dynamic_cast<CompoundStmt*>(alternative())) {
        o<<" ";
        alternative()->dump();
    } else newIndentDumpStmt(alternative(), +1);
    return o;
}

std::ostream& IfStmt::stream(std::ostream& o) const {
    o<<"if (";
    condition()->dump();
    o<<")";
    if (dynamic_cast<CompoundStmt*>(consequence())) consequence()->dump();
    else newIndentDumpStmt(consequence(), +1);
    return o;
}

std::ostream& NullStmt::stream(std::ostream& o) const {
    return o << ";";
}

std::ostream& CompoundStmt::stream(std::ostream& o) const {
    o<<("{"); 
    indentlvl++;

    for(size_t ind=0; ind<num_blockItems(); ind++) {
        newIndentDumpBlockItem(blockItem(ind));
    }
    indentlvl--; 
    newIndent();
    return o<<"}";
}

std::ostream& LabeledStmt::stream(std::ostream& o) const {
    int oldIndentLvl = indentlvl;
    o << labelString() << ":";
    indentlvl = oldIndentLvl; 
    newIndentDumpStmt(statement());
    return o;
}



//! =================================================
//! ================ Expressions ====================
//! =================================================
std::ostream& InfixExp::stream(std::ostream& o) const {
    o << "(";
    lhs()->dump();
    o << " " << operation().str() << " ";
    rhs()->dump();
    o << ")";
    return o;
}

std::ostream& TernaryExp::stream(std::ostream& o) const {
    o << "(";
    condition()->dump();
    o << " ? ";
    consequence()->dump();
    o << " : ";
    alternative()->dump();
    o << ")";
    return o;
}

std::ostream& PrefixExp::stream(std::ostream& o) const {
    o <<"(" << prefixString() ;
    operand()->dump();
    return o<<")";
}

std::ostream& MemberAccessExp::stream(std::ostream& o) const {
    o << "(";
    object()->dump();
    return o << Tok::tag2str(operation()) << member_name() << ")";
}

std::ostream& ArrayExp::stream(std::ostream& o) const {
    o << "(";
    object()->dump();
    o << "[";
    index()->dump();
    return o << "]" << ")";
}

std::ostream& FuncCallExp::stream(std::ostream& o) const {
    o<<("(");
    func()->dump();
    o<<"(";
    size_t ind = 0;
    if (num_parameters()>0){
        //auto p = parameter(0);

        do {
            //auto p = parameter(ind);
            parameter(ind)->dump();
            ind++;
        } while (ind < num_parameters() && o<<", ");
    }
    return o<<"))";
}

std::ostream& SizeOfTypeExp::stream(std::ostream& o) const {
    return o << "(sizeof(" << typeString() << "))";
}

std::ostream& SizeOfUnaryExp::stream(std::ostream& o) const {
    o << "(sizeof ";
    exp()->dump();
    return o<<")";
}

std::ostream& PostfixExp::stream(std::ostream& o) const {
    o <<"(" ;
    operand()->dump();
    return o<< postfixString() <<")";
}

//! =================================================
//! ============== Basic Expressions ================
//! =================================================

std::ostream& Identifier::stream(std::ostream& o) const {
    return o << name();
}

std::ostream& Integer::stream(std::ostream& o) const {
    return o << value();
}

std::ostream& Character::stream(std::ostream& o) const {
    return o << value();
}

std::ostream& Literal::stream(std::ostream& o) const {
    return o << value();
}


//! =================================================
//! ============= Error Exp/Stmt/Decl ===============
//! =================================================
std::ostream& ErrExp::stream(std::ostream& o) const {
    return o << "<errorExp>";
}

std::ostream& ErrStmt::stream(std::ostream& o) const {
    return o << "<errorStmt>";
}

std::ostream& ErrDecl::stream(std::ostream& o) const {
    return o << "<errorDecl>";
}

}
