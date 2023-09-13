#include <concepts>
#include <sstream>
#include <unordered_map>

#include "headers/evaluator.hpp"
#include "headers/util.hpp"
#include "headers/ast.hpp"

namespace LambdaCalc
{

using namespace AST;

std::unique_ptr<AST::Expression> AST::Expression::simplify(
    const BindingTable& bindings
) const { return getExpressionCopy(); }

std::unique_ptr<AST::Expression> AST::LetExpr::simplify(
    const BindingTable& bindings
) const {
    return expr
        ->getExpressionCopy()
        ->substitute(binding->from.name, *binding->to->simplify(bindings))
        ->simplify(bindings);
}

std::unique_ptr<Expression> AST::LetExpr::substitute(
    std::string name,
    const Expression& expr
) const {
    if (name == binding->from.name) return getExpressionCopy();

    auto new_binding_expr = binding->to->substitute(name, expr);
    auto new_expr = this->expr->substitute(name, expr);

    return std::make_unique<AST::LetExpr>(
        std::make_unique<AST::Binding>(binding->from, std::move(new_binding_expr)),
        std::move(new_expr)
    );
}

std::unique_ptr<AST::Expression> AST::WhereExpr::simplify(
    const BindingTable& bindings
) const {
    return expr
        ->getExpressionCopy()
        ->substitute(binding->from.name, *binding->to->getExpressionCopy())
        ->simplify(bindings);
}

std::unique_ptr<Expression> AST::WhereExpr::substitute(
    std::string name,
    const Expression& expr
) const {
    if (name == binding->from.name) return getExpressionCopy();

    auto new_binding_expr = binding->to->substitute(name, expr);
    auto new_expr = this->expr->substitute(name, expr);

    return std::make_unique<AST::WhereExpr>(
        std::move(new_expr),
        std::make_unique<AST::Binding>(binding->from, std::move(new_binding_expr))
    );
}

std::unique_ptr<Expression> AST::Mapping::substitute(
    std::string name,
    const Expression& expr
) const {
    if (name == from.name) return getExpressionCopy();
    else return std::make_unique<AST::Mapping>(
        from,
        to->substitute(name, expr)
    );
}

std::unique_ptr<AST::Expression> AST::Name::simplify(
    const BindingTable& bindings
) const {
    try {
        return bindings.at(name)->simplify(bindings);
    } catch (std::out_of_range e)
    { throw evaluation_error("Cannot evaluate `"+name+"`, it is not defined."); }
}

std::unique_ptr<Expression> AST::Name::substitute(
    std::string name,
    const Expression& expr
) const {
    if (this->name == name) return expr.getExpressionCopy();
    else return getExpressionCopy();
}

std::unique_ptr<AST::Expression> AST::BracketExpr::simplify(
    const BindingTable& bindings
) const { return expr->simplify(bindings); }

std::unique_ptr<Expression> AST::BracketExpr::substitute(
    std::string name,
    const Expression& expr
) const {
    return this->expr->substitute(name, expr);
}

std::unique_ptr<AST::Expression> AST::ApplicationExpr::simplify(
    const BindingTable& bindings
) const {
    auto _left = left->simplify(bindings);

    if (auto mapping = dynamic_cast<Mapping*>(_left.get()))
    {
        return mapping->to->substitute(mapping->from.name, *right)->simplify(bindings);
    }
    else if (auto _left_string = dynamic_cast<String*>(_left.get()))
    {
        auto _right = right->simplify(bindings);

        if (auto _right_string = dynamic_cast<String*>(_right.get()))
            return std::make_unique<String>(_left_string->str + _right_string->str);
        
        throw evaluation_error(
            "Left side of application expression must not be a string "
            "unless right side is also a string in " + toString() +
            " where Left side is "+ _left->toString() +", and Right side is " + _right->toString());
    }
    else return _left;
}

std::unique_ptr<Expression> AST::ApplicationExpr::substitute(
    std::string name,
    const Expression& expr
) const {
    auto _left = left->substitute(name, expr);
    auto _right = right->substitute(name, expr);

    auto simple_right = dynamic_cast<SimpleExpr*>(_right.get())
        ? dynamic_pointer_cast<SimpleExpr>(std::move(_right))
        : std::make_unique<BracketExpr>(std::move(_right));

    return std::make_unique<AST::ApplicationExpr>(std::move(_left), std::move(simple_right));
}

}
