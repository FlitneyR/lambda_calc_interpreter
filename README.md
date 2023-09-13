# Lambda Calc Repl

A simple lambda calculus repl interpreter.

## Basic Language Grammar

$$
\begin{align}

line ::=&\ binding\ |\ expr\\
binding ::=&\ name = expr\\
        &|\ name\ =\ expr\ where\ binding^+\\
expr ::=&\ name\ \rightarrow\ expr\\
        &|\ simpleExpr\\\
        &|\ appExpr\\
        &|\ `let`\ binding\ `in`\ expr\\
appExpr ::=&\ simpleExpr\\
        &|\ appExpr\ simpleExpr\\
simpleExpr ::=&\ name\ |\ "string"\ |\ (expr)\ |\ \$\ expr\\

\end{align}
$$

A $name$ is defined as any alphanumeric string, but can also contain `:` and `_`.\
A $string$ is defined as any string that does not contain `"`.

`"//"` at the start of a line denotes a comment.\
`#include "file/path/to/module/name"` tells the interpreter to read the bindings, and run the code in `file/path/to/module/name.lambda`.

The interpreter will parse each line up to `\n`, unless the line ends `\`, then it will read the next line as well.

So booleans might be implemented like this:
```
/// Definition of True and False
Bool::True    = a -> b -> a
Bool::False   = a -> b -> b

/// Definition of And and Or for Bool
Bool::And     = a -> b -> a b False
Bool::Or      = a -> b -> a True b

/// Defintion of Print for Bool
Bool::Print = bool -> bool "True" "False"

/// Some tests
Bool::Print (And True True)   // (And True True) "True" "False"
Bool::Print (And True False)
Bool::Print (Or True True)
Bool::Print (Or True False)
```

## Getting Started

To compile the project, go to the root of the project and run `make bin/main`, you will need `make` and `clang++` to be installed.

To run the example main program, move to the `lambda` folder, and run `../bin/main -r main`. The program should output `"[0, 1, 1, 2, 3, 5, 8, 13]"`, the first 8 numbers of the Fibonacci sequence.

### Arguments

The program accepts a number of arguments, or none at all. Any argument that begins with `-` will be treated as a modifying argument, all other arguments will be treated as a path to a `.lambda` file containing source code (***without the `.lambda` extension***). You can include as many source files as you like, but they will overwrite each other if they define variables with the same name. Also, a file can only be included once, so if you include a file twice, the second include will be ignored - this applies to all code, not just command line arguments.

The `-i` / `--interactive` argument runs the Read-Execute-Print-Loop (repl) after including the specified source files. This lets you input code and see the result output to the screen in realtime.

The `-r` / `--run` argument will run the `Main` function of the program you provide. Remember, if you provide multiple files which each define their own `Main` function, this will run the last one.
