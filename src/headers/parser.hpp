#pragma once

#include <memory>
#include <concepts>

#include "ast.hpp"

namespace LambdaCalc
{

template<typename P>
concept Parseable = requires (const char*& str) {
    { P::parse(str) } -> std::same_as<std::unique_ptr<P>>;
};

template<Parseable P>
class Parser
{
public:
    static std::unique_ptr<P> parse(std::string& source);
    static std::unique_ptr<P> parse(const char*& source);
};

void remove_leading_whitespace(const char*& source);

}