#include <memory>
#include <concepts>

#include "headers/parser.hpp"
#include "headers/util.hpp"
#include "headers/ast.hpp"

namespace LambdaCalc
{

template<Parseable P>
std::unique_ptr<P> Parser<P>::parse(std::string& source)
{
    const char* c_source = source.c_str();
    auto result = parse(c_source);
    remove_leading_whitespace(c_source);

    if (&*c_source == &*source.end())
    {
        source = std::string(c_source);
        return result;
    }

    return nullptr;
}

template<Parseable P>
std::unique_ptr<P> Parser<P>::parse(const char*& source)
{
    const char* copy = source;

    auto result = P::parse(copy);
    if (result == nullptr) return nullptr;

    source = copy;
    return result;
}

void remove_leading_whitespace(const char*& source)
{
    const static std::string whitespace = " \t\r\n\v";
    while (*source != 0)
    {
        bool is_whitespace = false;
        for (const auto& c : whitespace)
            is_whitespace |= *source == c;
        
        if (is_whitespace) source++;
        else return;
    }
}

bool match_exact_string(const char*& source, const std::string& comparison)
{
    for (int i = 0; i < comparison.length(); i++)
        if (source[i] != comparison[i]) return false;
    
    source += comparison.length();
    return true;
}

std::unique_ptr<AST::Line> AST::Line::parse(const char*& source)
{
    if (auto binding = Parser<AST::Binding>::parse(source))
        return binding;

    if (auto whereExpr = Parser<AST::WhereExpr>::parse(source))
        return whereExpr;
    
    if (auto expression = Parser<AST::Expression>::parse(source))
        return expression;
    
    if (auto comment = Parser<AST::Comment>::parse(source))
        return comment;
    
    if (auto include = Parser<AST::Include>::parse(source))
        return include;
    
    return nullptr;
}

std::unique_ptr<AST::Include> AST::Include::parse(const char*& source)
{
    remove_leading_whitespace(source);
    if (!match_exact_string(source, "#include")) return nullptr;

    auto name = Parser<AST::String>::parse(source);
    if (!name) return nullptr;

    return std::make_unique<AST::Include>(name->str);
}

std::unique_ptr<AST::Comment> AST::Comment::parse(const char*& source)
{
    remove_leading_whitespace(source);

    if (*source == '\0') return std::make_unique<Comment>();

    if (!match_exact_string(source, "//")) return nullptr;

    while (*source != '\0' && *source != '\n') source++;

    return std::make_unique<Comment>();
}

std::unique_ptr<AST::Expression> AST::Expression::parse(const char*& source)
{
    if (auto letExpr = Parser<AST::LetExpr>::parse(source))
        return letExpr;
    
    if (auto mapping = Parser<AST::Mapping>::parse(source))
        return mapping;

    auto simpleExpr = Parser<AST::SimpleExpr>::parse(source);
    if (!simpleExpr) return nullptr;

    auto rightExpr = Parser<AST::Expression>::parse(source);
    std::unique_ptr<AST::Expression> expr = rightExpr
        ? AST::ApplicationExpr::leftAppendSimpleExpr(std::move(simpleExpr), std::move(rightExpr))
        : dynamic_pointer_cast<AST::Expression>(std::move(simpleExpr));

    return expr;
}

std::unique_ptr<AST::WhereExpr> AST::WhereExpr::parse(const char*& source)
{
    auto expression = Parser<AST::Expression>::parse(source);

    remove_leading_whitespace(source);
    if (!match_exact_string(source, "where")) return nullptr;

    do {
        auto binding = Parser<AST::Binding>::parse(source);
        if (!binding) return nullptr;

        expression = std::make_unique<AST::WhereExpr>(std::move(expression), std::move(binding));

        remove_leading_whitespace(source);
    } while (match_exact_string(source, ","));

    return dynamic_pointer_cast<AST::WhereExpr>(std::move(expression));
}

std::unique_ptr<AST::Expression> AST::WhereExpr::getInnerMostExpression() const
{
    if (auto whereExpr = dynamic_cast<AST::WhereExpr*>(expr.get()))
        return whereExpr->getInnerMostExpression();
    else return expr->getExpressionCopy();
}

void AST::WhereExpr::setInnerMostExpression(std::unique_ptr<AST::Expression> newExpr)
{
    if (auto whereExpr = dynamic_cast<AST::WhereExpr*>(expr.get()))
        whereExpr->setInnerMostExpression(std::move(newExpr));
    else expr = std::move(newExpr);
}

std::unique_ptr<AST::LetExpr> AST::LetExpr::parse(const char*& source)
{
    remove_leading_whitespace(source);
    if (!match_exact_string(source, "let")) return nullptr;

    auto binding = Parser<AST::Binding>::parse(source);
    if (!binding) return nullptr;

    remove_leading_whitespace(source);
    if (!match_exact_string(source, "in")) return nullptr;

    auto expr = Parser<AST::Expression>::parse(source);
    if (!expr) return nullptr;

    return std::make_unique<AST::LetExpr>(std::move(binding), std::move(expr));
}

std::unique_ptr<AST::SimpleExpr> AST::SimpleExpr::parse(const char*& source)
{
    if (auto name = Parser<AST::Name>::parse(source))
        return name;

    if (auto string = Parser<AST::String>::parse(source))
        return string;
    
    if (auto bracketExpr = Parser<AST::BracketExpr>::parse(source))
        return bracketExpr;
    
    return nullptr;
}

std::unique_ptr<AST::Name> AST::Name::parse(const char*& source)
{
    remove_leading_whitespace(source);

    const static std::string valid_name_chars(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "_:"
    );

    const char* name_begin = source;
    while (*source != 0)
    {
        bool is_valid_name_char = false;

        for (const auto& c : valid_name_chars)
        if (*source == c)
        {
            is_valid_name_char = true;
            break;
        }

        if (!is_valid_name_char) break;
        source++;
    }
    const char* name_end = source;

    if (name_begin == name_end) return nullptr;

    std::string name(name_begin, name_end);

    if (keywords.contains(name)) return nullptr;

    return std::make_unique<AST::Name>(name);
}

std::unique_ptr<AST::String> AST::String::parse(const char*& source)
{
    remove_leading_whitespace(source);
    if (!match_exact_string(source, "\"")) return nullptr;

    const char* str_begin = source;
    while (*source != '\"')
    {
        if (*source == 0) return nullptr;
        source++;
    }
    const char* str_end = source;

    if (!match_exact_string(source, "\"")) return nullptr;

    std::string str(str_begin, str_end);
    return std::make_unique<AST::String>(str);
}

std::unique_ptr<AST::BracketExpr> AST::BracketExpr::parse(const char*& source)
{
    remove_leading_whitespace(source);

    if (match_exact_string(source, "$"))
        return std::make_unique<AST::BracketExpr>(Parser<AST::Expression>::parse(source));

    if (!match_exact_string(source, "(")) return nullptr;

    auto expr = Parser<Expression>::parse(source);
    if (!expr) return nullptr;

    remove_leading_whitespace(source);
    if (!match_exact_string(source, ")")) return nullptr;

    return std::make_unique<AST::BracketExpr>(std::move(expr));
}

std::unique_ptr<AST::Binding> AST::Binding::parse(const char*& source)
{
    auto name = Parser<AST::Name>::parse(source);
    if (!name) return nullptr;

    remove_leading_whitespace(source);
    if (!match_exact_string(source, "=")) return nullptr;

    std::unique_ptr<AST::Expression> expr = Parser<AST::WhereExpr>::parse(source);
    if (!expr) expr = Parser<AST::Expression>::parse(source);
    if (!expr) return nullptr;

    return std::make_unique<AST::Binding>(*name, std::move(expr));
}

std::unique_ptr<AST::Mapping> AST::Mapping::parse(const char*& source)
{
    auto name = Parser<AST::Name>::parse(source);
    if (!name) return nullptr;

    remove_leading_whitespace(source);
    if (!match_exact_string(source, "->")) return nullptr;

    auto expr = Parser<AST::Expression>::parse(source);
    if (!expr) return nullptr;

    return std::make_unique<AST::Mapping>(*name, std::move(expr));
}

}
