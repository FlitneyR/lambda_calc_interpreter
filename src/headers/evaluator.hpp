#pragma once

#include <concepts>
#include <unordered_map>

#include "util.hpp"
#include "ast.hpp"

namespace LambdaCalc
{

class evaluation_error : public std::runtime_error
{
public:
    evaluation_error(std::string what) :
        std::runtime_error(what)
    {}
};

}
