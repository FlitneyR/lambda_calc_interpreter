#include <memory>
#include <string>

#include "headers/util.hpp"
#include "headers/ast.hpp"

/// Prints names of types and brackets to make the structure of the AST more clear as a debugging measure
// #define DEBUG_AST_STRUCTURE

namespace LambdaCalc::AST
{

std::unique_ptr<ApplicationExpr> ApplicationExpr::leftAppendSimpleExpr(
    std::unique_ptr<SimpleExpr> leftExpr,
    std::unique_ptr<Expression> rightExpr
) {
    if (auto appExpr = dynamic_pointer_cast<ApplicationExpr>(std::move(rightExpr)))
    {
        appExpr->left = leftAppendSimpleExpr(std::move(leftExpr), std::move(appExpr->left));
        return appExpr;
    }
    else if (auto rightSimpExpr = dynamic_pointer_cast<SimpleExpr>(std::move(rightExpr)))
        return std::make_unique<ApplicationExpr>(std::move(leftExpr), std::move(rightSimpExpr));
    else
    {
        return leftAppendSimpleExpr(
            std::move(leftExpr),
            std::make_unique<AST::BracketExpr>(std::move(rightExpr))
        );
    }
}

std::string Comment::toString() const
{ return ""; }

ApplicationExpr::ApplicationExpr(const ApplicationExpr& other)
{
    left = other.left->getExpressionCopy();
    right = dynamic_pointer_cast<SimpleExpr>(other.right->getExpressionCopy());
}

std::string Include::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " Include( "+"#include "+name+" )Include "; }
#else
{ return "#include "+name; }
#endif

std::string LetExpr::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " LetExpr( let "+binding->toString()+" in "+expr->toString()+" )LetExpr "; }
#else
{ return "let "+binding->toString()+" in "+expr->toString(); }
#endif

std::string WhereExpr::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " WhereExpr( "+expr->toString()+" where "+binding->toString()+" )WhereExpr "; }
#else
{ return expr->toString()+" where "+binding->toString(); }
#endif

std::string ApplicationExpr::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " AppExpr( "+left->toString()+" "+right->toString()+" )AppExpr "; }
#else
{ return left->toString()+" "+right->toString(); }
#endif

std::string Name::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " Name( "+name+" )Name "; }
#else
{ return name; }
#endif

std::string String::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " String( \""+str+"\" )String "; }
#else
{ return '\"'+str+'\"'; }
#endif

std::string BracketExpr::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " BracketExpr( "+expr->toString()+" )BracketExpr "; }
#else
{ return '('+expr->toString()+')'; }
#endif

std::string Binding::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " Binding( "+from.toString()+" = "+to->toString()+" )Binding "; }
#else
{ return from.toString()+" = "+to->toString(); }
#endif

std::string Mapping::toString() const
#ifdef DEBUG_AST_STRUCTURE
{ return " Mapping( "+from.toString()+" -> "+to->toString()+" )Mapping "; }
#else
{ return from.toString()+" -> "+to->toString(); }
#endif

}

#ifdef DEBUG_AST_STRUCTURE
#undef DEBUG_AST_STRUCTURE
#endif
