#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include "headers/interpreter.hpp"
#include "headers/evaluator.hpp"
#include "headers/ast.hpp"

#include "parser.cpp"

namespace LambdaCalc
{

BindingTable Interpreter::run(const BindingTable& initialBindings)
{
    bindings.clear();

    for (const auto& pair : initialBindings)
        bindings[pair.first] = pair.second->getExpressionCopy();

    return run();
}

BindingTable Interpreter::run(const std::unordered_set<std::string>& initialIncludes)
{
    includes = initialIncludes;
    return run();
}

BindingTable Interpreter::run(
    const BindingTable& initialBindings,
    const std::unordered_set<std::string>& initialIncludes
) {
    bindings.clear();

    for (const auto& pair : initialBindings)
        bindings[pair.first] = pair.second->getExpressionCopy();
    
    includes = initialIncludes;

    return run();
}

BindingTable Interpreter::run()
{
    while (!end())
    {
        auto source = read();

        auto line = Parser<AST::Line>::parse(source);

        if (!line || source.length() > 0)
        {
            print_error("Unable to parse: \"" + source + "\"");
            continue;
        }

        if (auto binding = dynamic_cast<AST::Binding*>(line.get()))
        {
            if (bindings.contains(binding->from.name))
                print_error(
                    "Warning: "
                    "Shadowing binding `" + binding->from.name
                );

            bindings[binding->from.name] = binding->to->getExpressionCopy();
        }
        
        if (auto expression = dynamic_cast<AST::Expression*>(line.get()))
        try
        {
            print(expression->simplify(bindings)->toString());
        } catch (evaluation_error e)
        {
            print_error("Evaluation error: " + std::string(e.what()));
        }

        if (auto include = dynamic_cast<AST::Include*>(line.get()))
        {
            if (includes.contains(include->name))
            {
                // print_error("Include warning: Already included `"+include->name+"`");
                continue;
            }
            
            std::ifstream include_file(include->name + ".lambda");
            if (include_file.fail())
            {
                print_error("Include Error: Failed to open file: \""+include->name+".lambda\"");
                continue;
            }

            StreamInterpreter file_interpreter(include_file);
            BindingTable new_bindings = file_interpreter.run(includes);

            for (const auto& entry : new_bindings)
            {
                if (bindings.contains(entry.first))
                    print_error(
                        "Include warning: "
                        "Shadowing binding `" + entry.first +
                        "` while including " + include->name
                    );
                
                bindings[entry.first] = entry.second->getExpressionCopy();
            }

            includes = file_interpreter.includes;
            includes.insert(include->name);
        }
    }

    // we need to explicitly copy each unique_ptr's content
    BindingTable results;
    for (const auto& pair : bindings)
        results[pair.first] = pair.second->getExpressionCopy();
    return results;
}

std::string StreamInterpreter::read()
{
    std::string source;

    const static char* whitespace = " \n\r\v\t";

    while(!end())
    {
        std::string line;
        std::getline(input, line);
        source += line;

        std::size_t index = source.find_last_not_of(whitespace);
        if (source[index] != '\\') break;
        source.erase(index);
    }

    return source;
}

void StreamInterpreter::print(std::string message)
{ std::cout << message << std::endl; }

void StreamInterpreter::print_error(std::string message)
{ std::cerr << message << std::endl; }

bool StreamInterpreter::end()
{ return input.eof(); }

std::string Repl::read()
{
    output << ">>> ";
    std::string source;

    const static char* whitespace = " \n\r\v\t";

    while(!end())
    {
        std::string line;
        std::getline(input, line);
        source += line;

        std::size_t index = source.find_last_not_of(whitespace);
        if (source[index] != '\\') break;
        source.erase(index);
    }

    return source;
}

void Repl::print(std::string message)
{ output << message << "\n" << std::endl; }

void Repl::print_error(std::string message)
{ error << message << "\n" << std::endl; }

bool Repl::end()
{ return input.eof(); }

}