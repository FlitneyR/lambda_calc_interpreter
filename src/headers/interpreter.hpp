#pragma once

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <memory>

#include "ast.hpp"

namespace LambdaCalc
{

class Interpreter
{
public:
    BindingTable bindings;
    std::unordered_set<std::string> includes;

    BindingTable run();
    BindingTable run(const BindingTable& initialBindings);
    BindingTable run(const std::unordered_set<std::string>& initialIncludes);
    BindingTable run(const BindingTable& initialBindings, const std::unordered_set<std::string>& initialIncludes);

protected:
    virtual std::string read() = 0;
    virtual void print(std::string message) = 0;
    virtual void print_error(std::string error) = 0;
    virtual bool end() = 0;
};

class StreamInterpreter : public Interpreter
{
protected:
    std::istream& input;
    std::ostream& output;
    std::ostream& error;

public:
    StreamInterpreter(
        std::istream& input,
        std::ostream& output,
        std::ostream& error
    ) : input(input),
        output(output),
        error(error)
    {}

    StreamInterpreter(std::istream& input) : StreamInterpreter(input, std::cout, std::cerr) {}

    StreamInterpreter() : StreamInterpreter(std::cin, std::cout, std::cerr) {}

protected:
    std::string read() override;
    void print(std::string message) override;
    void print_error(std::string message) override;
    bool end() override;
};

class Repl : public StreamInterpreter
{
protected:
    std::istream& input;
    std::ostream& output;
    std::ostream& error;

public:
    Repl(
        std::istream& input,
        std::ostream& output,
        std::ostream& error
    ) : input(input),
        output(output),
        error(error)
    {}

    Repl() : Repl(std::cin, std::cout, std::cerr) {}

protected:
    std::string read() override;
    void print(std::string message) override;
    void print_error(std::string message) override;
    bool end() override;
};

}