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


void TranslationUnit::check(Sema &sema) {
    for (size_t i = 0; i < num_ext_declarations(); i++) {
        external_declaration(i)->check(sema);
    }     
}

//! =================================================
//! ================ Declarations ===================
//! =================================================

void ExternalDeclaration::check(Sema &sema) {
    if (specifierDeclarator() == nullptr) return;
    auto typeString = specifierDeclarator()->typeString();
    //auto type = specifierDeclarator()->type();
    auto name = specifierDeclarator()->name();
    //bool definingStruct = false;  

    if (typeString=="struct"){
        StructSpecifier* structSpecif = dynamic_cast<StructSpecifier*>(specifierDeclarator()->specifier());
        std::string structIdentifier = structSpecif->structIdentifierString();
        
        //if (structSpecif->num_structDeclarations() == 0) loc().err() << "Struct has no members!" << loc().endErr();

        if (sema.structDefined(structIdentifier) && structSpecif->declarationListSet()) loc().err() << "Redeclaration of struct " << structIdentifier << "!" << loc().endErr();
        if (!sema.structDefined(structIdentifier) && !structSpecif->declarationListSet() && name!="") loc().err() << "Storage size of '" << name << "' unknown!" << loc().endErr();

        if (!sema.structDefined(structIdentifier) && structSpecif->declarationListSet()) sema.addStructDefinition(structSpecif);
        if (!structSpecif->declarationListSet() && name=="") loc().err() << "External declarations should declare at least one declarator!" << loc().endErr();
        if (!sema.structDefined(structIdentifier) && name=="") loc().err() << "Unnamed struct that defines no instances!" << loc().endErr();
    }

    if (specifierDeclarator()->declarator() == nullptr && specifierDeclarator()->typeString()!="struct"){
        loc().err() << "External declarations should declare at least one declarator!" << loc().endErr();
        return;
    }

    sema.addDeclaration(specifierDeclarator());                                            //  Declaration to the current scope

    
    if (functionBody()!=nullptr) {                                              // If Declaration is in fact a Function Definition
        sema.external_declaration(this);
        
        CompoundStmt* compoundFunctionBody = dynamic_cast<CompoundStmt*>(functionBody());
        const Ptrs<SpecifierDeclarator>& paramList = specifierDeclarator()->parameterList();

        for (size_t i = 0; i < paramList.size(); i++)
        {
            auto param = paramList[i].get();
            if (param->name() == "" && param->typeString()!="void") param->loc().err() << "Parameter name ommitted!" << param->loc().endErr(); 
        }
        
        sema.external_declaration(this);                                        // Remember the current  

        compoundFunctionBody->check(sema);
        
        sema.external_declaration(nullptr);
    }


}


//! =================================================
//! ================ Statements =====================
//! =================================================

void Declaration::check(Sema &sema) {
    sema.addDeclaration(specifierDeclarator());                                            //  Declaration to the current scope
    if (specifierDeclarator()->typeString()=="struct") sema.addStructDefinition(dynamic_cast<StructSpecifier*>(specifierDeclarator()->specifier()));
}

void ExpressionStmt::check(Sema &sema) {
    exp()->check(sema);
}

void EmptyReturnStmt::check(Sema &sema) {UNUSED(sema);}

void ReturnStmt::check(Sema &sema) {
    Type* returnType = exp()->check(sema);
    auto function = sema.external_declaration();
    auto specDecl = function->specifierDeclarator();
    if(specDecl){
        auto specDeclType = specDecl->type();
        if (specDeclType && dynamic_cast<FunctionType*>(specDeclType)){
            FunctionType* functionType = dynamic_cast<FunctionType*>(specDeclType);
            if (functionType->returnType()) {
                Type* functionReturnType = functionType->returnType();

                if (returnType && functionReturnType->str() != returnType->str()) 
                    loc().err() << "Wrong return type (got type " << returnType->str() << ", expected type "<< functionReturnType->str() << ")!" << loc().endErr();
            }
        }
    }
}

void GoToStmt::check(Sema &sema) {
    if (sema.lookupLabel(gotoLabel())==nullptr) loc().err() << "Label " << gotoLabel() << " not declared!" << loc().endErr();
}

void BreakStmt::check(Sema &sema) {
    if (sema.loop() == nullptr) loc().err() << "'Break' outside of loop!" << loc().endErr();
}

void ContinueStmt::check(Sema &sema) {
    if (sema.loop() == nullptr) loc().err() << "'Continue' outside of loop!" << loc().endErr();
}

void WhileStmt::check(Sema &sema) {
    Type* conditionType = condition()->check(sema);
    if (!conditionType->isScalar()) {
        loc().err() << "Condition of While-Statement has to be scalar!" << loc().endErr();
    }
    auto oldLoop = sema.loop();
    sema.setLoop(this);
    loop()->check(sema);
    sema.setLoop(oldLoop);
}

void IfElseStmt::check(Sema &sema) {
    Type* conditionType = condition()->check(sema);
    if (!conditionType->isScalar()) {
        loc().err() << "Condition of If-Statement has to be scalar!" << loc().endErr();
    }
    consequence()->check(sema);
    alternative()->check(sema);
}

void IfStmt::check(Sema &sema) {
    Type* conditionType = condition()->check(sema);
    if (!conditionType->isScalar()) {
        loc().err() << "Condition of If-Statement has to be scalar!" << loc().endErr();
    }
    consequence()->check(sema);
}

void NullStmt::check(Sema &sema) {UNUSED(sema);}

void CompoundStmt::check(Sema &sema) {
    //std::cout << "=============== Start of new scope ===============";
    sema.push();                                                                                    // Start new scope

    if (sema.external_declaration() != nullptr && sema.size()==2) {                                 // Add Parameters only to function body environment
        const Ptrs<SpecifierDeclarator>& paramList = sema.external_declaration()->specifierDeclarator()->parameterList();
        for (size_t i = 0; i < paramList.size(); i++) {
            auto param = paramList[i].get();
            sema.addDeclaration(param);
        }
    }
    
    for(size_t ind=0; ind<num_blockItems(); ind++) {
        blockItem(ind)->check(sema);
    }
    //std::cout << "=============== End of new scope ===============";

    sema.pop(); // End new scope
    //std::cout << std::endl;
}

void LabeledStmt::check(Sema &sema) {UNUSED(sema);}

void ErrStmt::check(Sema &sema) {UNUSED(sema);}


//! =================================================
//! ============= Infix Expressions =================
//! =================================================

Type* InfixExp::check(Sema &sema) {
    Type* lhs_type = lhs()->check(sema);
    // std::cout << "LHS: " << lhs_type->str() << std::endl;
    Type* rhs_type = rhs()->check(sema);
    // std::cout << "RHS: " << rhs_type->str() << std::endl;


    if (lhs_type->str() == "error" || rhs_type->str() =="error") return type_ = new ErrorType();

    // Logical Operators
    if (operation().isa(Tok::Tag::P_Less) || 
        operation().isa(Tok::Tag::P_Less_Equal) || 
        operation().isa(Tok::Tag::P_Greater) || 
        operation().isa(Tok::Tag::P_Greater_Equal) ||
        operation().isa(Tok::Tag::P_Equal) ||
        operation().isa(Tok::Tag::P_Unequal)) {
            if ((dynamic_cast<ArithmeticType*>(lhs_type) && dynamic_cast<ArithmeticType*>(rhs_type)) || lhs_type->str()==rhs_type->str()) {
                return type_ = new IntType();
        }    
    }

    // Logical And/Or
    if (operation().isa(Tok::Tag::P_Logical_And) || operation().isa(Tok::Tag::P_Logical_Or)) {
        if (lhs_type->isScalar() && rhs_type->isScalar()) return type_ = new IntType();
    }
    
    //Assignment
    if (operation().isa(Tok::Tag::P_Assign)) {
        
        if (!dynamic_cast<Identifier*>(lhs()) && !dynamic_cast<PrefixExp*>(lhs()) && !dynamic_cast<ArrayExp*>(lhs()) && !dynamic_cast<MemberAccessExp*>(lhs())){
            operation().loc().err() << "LHS of assignment must be a modifiable lvalue!" << loc().endErr();
        } else if (lhs_type->str() != rhs_type->str()) {
            operation().loc().err() << "Incompatible Types for operand '" << operation().str() << "' (" << lhs_type->str() << "<->" << rhs_type->str() <<")!" << loc().endErr();
        }
        return type_ = lhs_type;
    }

    // Addition
    if (operation().isa(Tok::Tag::P_Addition)){
        if (dynamic_cast<ArithmeticType*>(lhs_type) && dynamic_cast<ArithmeticType*>(rhs_type)) return type_ = new IntType();
        else if (dynamic_cast<PointerType*>(lhs_type) && dynamic_cast<PointerType*>(lhs_type)->pointee()->isComplete() && dynamic_cast<const ArithmeticType*>(rhs_type)) return type_ = lhs_type;
        else if (dynamic_cast<PointerType*>(rhs_type) && dynamic_cast<PointerType*>(rhs_type)->pointee()->isComplete() && dynamic_cast<const ArithmeticType*>(lhs_type)) return type_ = rhs_type;

    }

    //Substraction
    if (operation().isa(Tok::Tag::P_Substraction)){
        if (dynamic_cast<ArithmeticType*>(lhs_type) && dynamic_cast<ArithmeticType*>(rhs_type)) return type_ = new IntType();
        else if (dynamic_cast<PointerType*>(lhs_type) && dynamic_cast<PointerType*>(lhs_type)->pointee()->isComplete() && dynamic_cast<PointerType*>(rhs_type) && dynamic_cast<PointerType*>(rhs_type)->pointee()->isComplete() && lhs_type->str()==rhs_type->str()) return type_ = new IntType();
        else if (dynamic_cast<PointerType*>(lhs_type) && dynamic_cast<PointerType*>(lhs_type)->pointee()->isComplete() && dynamic_cast<const IntType*>(rhs_type)) return type_ = lhs_type;
    }

    // Multiplication and Division
    if (operation().isa(Tok::Tag::P_Multiplication) || operation().isa(Tok::Tag::P_Division)){
        if (dynamic_cast<ArithmeticType*>(lhs_type) && dynamic_cast<ArithmeticType*>(rhs_type)) return type_ = new IntType(); 
    }

    operation().loc().err() << "Incompatible Types for operand '" << operation().str() << "' (" << lhs_type->str() << "<->" << rhs_type->str() <<")!" << loc().endErr();
    return type_ = sema.error_type();
}

Type* TernaryExp::check(Sema &sema) {
    //auto conditionType = condition()->check(sema);
    auto consequenceType = consequence()->check(sema);
    auto alternativeType = alternative()->check(sema);

    if (consequenceType->str() != alternativeType->str()) {
        loc().err() << "Mismatch of type in consequence and alternative of ternary expression (" << consequenceType->str() << "<->" << alternativeType->str() << ")" << loc().endErr();
        return new ErrorType();
    }
    return consequenceType;
}

Type* PrefixExp::check(Sema &sema) {
    
    auto opType = operand()->check(sema);

    if (prefix().isa(Tok::Tag::P_Multiplication)) { 
        if (dynamic_cast<PointerType*>(opType)) {
            auto operandType = dynamic_cast<PointerType*>(opType);
            return type_ = operandType->pointee();
        } else {
            loc().err() << "Invalid type argument of unary '*' (have " << opType->str() << ")" << loc().endErr();
            return new ErrorType();
        }
    }
    if (prefix().isa(Tok::Tag::P_Bitwise_And)) { 
        return type_ = new PointerType(opType);
    }
    if (prefix().isa(Tok::Tag::P_Addition) || prefix().isa(Tok::Tag::P_Substraction)){
        if (dynamic_cast<ArithmeticType*>(opType)) return type_ = opType;
        else {
            loc().err() << "Invalid type argument of unary '"<< prefix() <<"' (have " << opType->str() << ")" << loc().endErr();
            return new ErrorType();
        }
    }
    if (prefix().isa(Tok::Tag::P_Logical_Not)){
        if (opType->isScalar()) return type_ = new IntType();
        else {
            loc().err() << "Invalid type argument of unary '"<< prefix() <<"' (have " << opType->str() << ")" << loc().endErr();
            return new ErrorType();
        }
    }
    if (prefix().isa(Tok::Tag::P_Bitwise_Not)){
        if (dynamic_cast<IntType*>(opType)) return type_ = opType;
        else {
            loc().err() << "Invalid type argument of unary '"<< prefix() <<"' (have " << opType->str() << ")" << loc().endErr();
            return new ErrorType();
        }
    }

    return type_ = opType;
}

Type* MemberAccessExp::check(Sema &sema) {
    
    // TODO: Check that member_name is actually part of the object
    auto objectType = object()->check(sema);

    if (dynamic_cast<ErrorType*>(objectType)) return type_ = new ErrorType();

    // Correct operation for correct type
    if (dynamic_cast<PointerType*>(objectType) && operation()==Tok::Tag::P_Dot){
        loc().err() << "Object with pointer type must use '->' operation to access members!" << loc().endErr();
    } else if (!dynamic_cast<PointerType*>(objectType) && operation()==Tok::Tag::P_Arrow_R) {
        loc().err() << "Object with object type must use '.' operation to access members!" << loc().endErr();
    }

    if (dynamic_cast<Identifier*>(object())) {
        auto obj = dynamic_cast<Identifier*>(object());
        StructSpecifier* structSpecif = dynamic_cast<StructSpecifier*>(sema.lookup(obj->name())->specifier());
        std::string structIdent = structSpecif->structIdentifierString();
        SpecifierDeclarator* member = sema.lookupMember(structIdent, member_name());

        if (!sema.structDefined(structIdent)) loc().err() << "'" <<  obj->name() << "' is not a struct!" << loc().endErr();
        else if (member == nullptr) loc().err() << "'struct " <<  structIdent << "' has no member named '" << member_name() << "'!" << loc().endErr();
        else return type_ = member->type();
    }
        
    return type_ = new ErrorType();
}


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
