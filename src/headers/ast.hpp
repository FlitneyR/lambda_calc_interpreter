#pragma once

#include <memory>
#include <string>
#include <unordered_set>

#include "parser.hpp"
#include "util.hpp"

namespace LambdaCalc::AST
{

class Line;
class Binding;
class Expression;
class Mapping;
class ApplicationExpr;
class SimpleExpr;
class Name;
class String;
class BracketExpr;

const std::unordered_set<std::string> keywords {
    "let",
    "in",
    "where"
};

class Line
{
public:
    static std::unique_ptr<Line> parse(const char*& source);

    Line() {}
    virtual ~Line() = default;

    virtual std::string toString() const = 0;

private:
};

template<std::derived_from<Line> T>
std::ostream& operator<<(std::ostream& os, const T& t)
{ return os << t.toString(); }

class Include : public Line
{
public:
    static std::unique_ptr<Include> parse(const char*& source);

    std::string name;

    Include(std::string name) : name(name) {}

    std::string toString() const override;
};

class Comment : public Line
{
public:
    static std::unique_ptr<Comment> parse(const char*& source);

    Comment() {}

    std::string toString() const override;
};

class Expression : public Line
{
public:
    static std::unique_ptr<Expression> parse(const char*& source);

    Expression() {}

    /// @return A base class unique_ptr to a copy of any concrete expression
    virtual std::unique_ptr<Expression> getExpressionCopy() const = 0;

    /// @brief Simplify this expression. Only call this function when evaluating
    /// @return A unique_ptr to a simplified copy of this expression
    virtual std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const;

    /// @brief Substitute a name with an expression in this expression
    /// @return A unique_ptr to a copy of this expression, with the
    ///         substitution made. 
    virtual std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const = 0;
};

/// @brief An expression that defines a local binding for use only in the expression.
///        Note that these are one time replacements.
///        If they refer to names in the bindings, that name will not be recognised
class WhereExpr : public Expression
{
public:
    std::unique_ptr<Expression> expr;
    std::unique_ptr<Binding> binding;

    WhereExpr() {}
    WhereExpr(
        std::unique_ptr<Expression> expr,
        std::unique_ptr<Binding> binding
    ) : expr(std::move(expr)),
        binding(std::move(binding))
    {}

    WhereExpr(const WhereExpr& other) :
        expr(other.expr->getExpressionCopy()),
        binding(std::make_unique<Binding>(*other.binding))
    {}

    std::unique_ptr<Expression> getExpressionCopy() const override 
    { return std::make_unique<WhereExpr>(*this); }
    
    static std::unique_ptr<WhereExpr> parse(const char*& source); 

    std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const override;

    std::string toString() const override;

    std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const override;
};

/// @brief Similar to a WhereExpr, but these bindings are simplified ahead of time.
///        Useful if you want to precalculate something to avoid calculating it twice.
///        But use with caution, as these can lead to infinite recursion if you are not careful
class LetExpr : public Expression
{
public:
    std::unique_ptr<Binding> binding;
    std::unique_ptr<Expression> expr;

    LetExpr() {}
    LetExpr(
        std::unique_ptr<Binding> binding,
        std::unique_ptr<Expression> expr
    ) : binding(std::move(binding)),
        expr(std::move(expr))
    {}

    LetExpr(const LetExpr& other) :
        binding(std::make_unique<Binding>(*other.binding)),
        expr(other.expr->getExpressionCopy())
    {}

    std::unique_ptr<Expression> getExpressionCopy() const override 
    { return std::make_unique<LetExpr>(*this); }

    static std::unique_ptr<LetExpr> parse(const char*& source);

    std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const override;

    std::string toString() const override;

    std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const override;
};

class ApplicationExpr : public Expression
{
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<SimpleExpr> right;

    ApplicationExpr() {}
    ApplicationExpr(
        std::unique_ptr<Expression> left,
        std::unique_ptr<SimpleExpr> right
    ) : left(std::move(left)),
        right(std::move(right))
    {}

    ApplicationExpr(const ApplicationExpr& other);

    ApplicationExpr& operator=(const ApplicationExpr& other);

    /// @brief Add a simple expression left of an application expression chain
    static std::unique_ptr<ApplicationExpr> leftAppendSimpleExpr(
        std::unique_ptr<SimpleExpr> leftSimpExpr,
        std::unique_ptr<Expression> appExpr
    );

    std::string toString() const override;

    std::unique_ptr<Expression> getExpressionCopy() const override 
    { return std::make_unique<ApplicationExpr>(*this); }

    std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const override;

    std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const override;
};

class SimpleExpr : public Expression
{
public:
    static std::unique_ptr<SimpleExpr> parse(const char*& source);

    SimpleExpr() {}
};

class Name : public SimpleExpr
{
public:
    std::string name;

    Name() {}
    Name(std::string name) : name(name) {}
    Name(const Name& other) : name(other.name) {}

    std::string toString() const override;

    std::unique_ptr<Expression> getExpressionCopy() const override 
    { return std::make_unique<Name>(*this); }

    static std::unique_ptr<Name> parse(const char*& source);

    std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const override;

    std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const override;
};

class String : public SimpleExpr
{
public:
    std::string str;

    String() {}
    String(std::string str) : str(str) {}
    String(const String& other) : str(other.str) {}

    std::string toString() const override;

    std::unique_ptr<Expression> getExpressionCopy() const override 
    { return std::make_unique<String>(*this); }

    static std::unique_ptr<String> parse(const char*& source);

    std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const override { return getExpressionCopy(); }

    std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const override { return getExpressionCopy(); };
};

class BracketExpr : public SimpleExpr
{
public:
    std::unique_ptr<Expression> expr;

    BracketExpr() {}
    BracketExpr(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}
    BracketExpr(const BracketExpr& other) :
        expr(other.expr->getExpressionCopy())
    {}

    std::string toString() const override;

    std::unique_ptr<Expression> getExpressionCopy() const override 
    { return std::make_unique<BracketExpr>(*this); }

    static std::unique_ptr<BracketExpr> parse(const char*& source);

    std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const override;

    std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const override;
};

class Binding : public Line
{
public:
    Name from;
    std::unique_ptr<Expression> to;

    Binding() {}
    Binding(
        Name from,
        std::unique_ptr<Expression> to
    ) : from(from),
        to(std::move(to))
    {}
    Binding(const Binding& other) :
        from(other.from),
        to(other.to->getExpressionCopy())
    {}

    std::string toString() const override;

    static std::unique_ptr<Binding> parse(const char*& source);
};

class Mapping : public Expression
{
public:
    Name from;
    std::unique_ptr<Expression> to;

    Mapping() {}
    Mapping(
        Name from,
        std::unique_ptr<Expression> to
    ) : from(from),
        to(std::move(to))
    {}

    Mapping(const Mapping& other) :
        from(other.from),
        to(other.to->getExpressionCopy())
    {}

    std::string toString() const override;
    
    std::unique_ptr<Expression> getExpressionCopy() const override 
    { return std::make_unique<Mapping>(*this); }

    static std::unique_ptr<Mapping> parse(const char*& source);

    std::unique_ptr<Expression> simplify(
        const BindingTable& bindings
    ) const override { return getExpressionCopy(); }

    std::unique_ptr<Expression> substitute(
        std::string name,
        const Expression& expr
    ) const override;
};

}