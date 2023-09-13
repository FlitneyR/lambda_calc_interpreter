#include <iostream>
#include <fstream>
#include <sstream>

#include "headers/interpreter.hpp"

int main(const int argc, const char** const argv)
{
    using namespace LambdaCalc;

    bool interactiveMode = false;
    bool runMain = false;

    std::stringstream instructions;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            std::string s(argv[i]);
            if (s == "-i" || s == "--interactive") interactiveMode = true;
            if (s == "-r" || s == "--run") runMain = true;
        }
        else
            instructions << "#include " << '"' << argv[i] << '"' << std::endl;
    }

    if (runMain) instructions << "Main" << std::endl;

    StreamInterpreter includesInterpreter(instructions);
    BindingTable fileBindings = includesInterpreter.run();

    if (interactiveMode)
        Repl().run(fileBindings, includesInterpreter.includes);

    return 0;
}
