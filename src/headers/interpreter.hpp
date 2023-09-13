#pragma once

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <memory>

#include "ast.hpp"

namespace LambdaCalc
{

/// @brief An interface for a lambda-calculus interpreter.
class Interpreter
{
public:
    BindingTable bindings;
    std::unordered_set<std::string> includes;

    /// @brief Runs the interpreter
    /// @param initialBindings The variables already bound in the enclosing scope
    /// @param initialIncludes The files already included, which should not be included again
    /// @return The new variables bound while running the interpreter.
    BindingTable run(
        const BindingTable* const initialBindings = nullptr,
        const std::unordered_set<std::string>* const initialIncludes = nullptr
    );

protected:

    /// @brief Read a line of code to interpret
    virtual std::string read() = 0;

    /// @brief Print a successful result of an expression
    virtual void print(std::string message) = 0;
    
    /// @brief Print an error message
    virtual void print_error(std::string error) = 0;

    /// @brief Determine if the end of the code has been reached
    /// @return True if there is no more code to interpret
    virtual bool end() = 0;
};

/// @brief 
class StreamInterpreter : public Interpreter
{
protected:
    std::istream& input;
    std::ostream& output;
    std::ostream& error;

public:
    StreamInterpreter(
        std::istream& input = std::cin,
        std::ostream& output = std::cout,
        std::ostream& error = std::cerr
    ) : input(input),
        output(output),
        error(error)
    {}

protected:
    std::string read() override;
    void print(std::string message) override;
    void print_error(std::string message) override;
    bool end() override;
};

class Repl : public StreamInterpreter
{
public:
    Repl(
        std::istream& input = std::cin,
        std::ostream& output = std::cout,
        std::ostream& error = std::cerr
    ) : StreamInterpreter(input, output, error) {}

protected:
    std::string read() override;
    void print(std::string message) override;
    void print_error(std::string message) override;
};

}