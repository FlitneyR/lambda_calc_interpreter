#include <iostream>
#include <fstream>
#include <sstream>

#include "headers/interpreter.hpp"

int main(const int argc, const char** const argv)
{
    using namespace LambdaCalc;

    // Parse command line arguments

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
        else // Add running the file to the initial program string
            instructions << "#include " << '"' << argv[i] << '"' << std::endl;
    }

    // Add call to run Main, if requested
    if (runMain) instructions << "Main" << std::endl;

    // Run included files
    StreamInterpreter includesInterpreter(instructions);
    BindingTable fileBindings = includesInterpreter.run();

    // Start interactive repl, if requested
    if (interactiveMode)
        Repl().run(fileBindings, includesInterpreter.includes);

    return 0;
}
