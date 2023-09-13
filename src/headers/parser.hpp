#pragma once

#include <memory>
#include <concepts>

#include "ast.hpp"

namespace LambdaCalc
{

/// @brief A type that can be parsed
template<typename P>
concept Parseable = requires (const char*& str) {
    { P::parse(str) } -> std::same_as<std::unique_ptr<P>>;
};

/// @brief A helper class for parsing classes that implement the Parseable concept
template<Parseable P>
class Parser
{
public:
    /// @brief The same as parse(const char*& source), but with the added assurance
    ///        that the entire source string has been parsed, if nullptr is not returned
    static std::unique_ptr<P> parse(std::string& source);

    /// @brief Parse a P and update the pointer to the remaining source code.
    ///        But if no P is parsed, i.e. the return value is nullptr, then
    ///        source is guaranteed to be unchanged.
    static std::unique_ptr<P> parse(const char*& source);
};

/// @brief Remove all whitespace characters at the start of a string
void remove_leading_whitespace(const char*& source);

}