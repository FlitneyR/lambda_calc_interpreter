#pragma once

#include <memory>

namespace LambdaCalc
{

/// @brief Try to dynamic cast a unique_ptr, return nullptr if it cannot be done
template<typename T, typename S>
std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<S>&& p) noexcept
{
    auto converted = std::unique_ptr<T>(dynamic_cast<T*>(p.get()));
    if (converted) p.release();
    return converted;
}

typedef std::unordered_map<
    std::string,
    std::unique_ptr<AST::Expression>
> BindingTable;

}
