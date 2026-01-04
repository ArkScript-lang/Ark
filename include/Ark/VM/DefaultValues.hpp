/*
 * @file DefaultValues.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Defines default values to be used by the VM
 * @date 2025-08-10
 *
 * @copyright Copyright (c) 2025-2026
 *
 */

#ifndef ARK_VM_DEFAULTVALUES_HPP
#define ARK_VM_DEFAULTVALUES_HPP

#include <Ark/VM/Value.hpp>

namespace Ark
{
    /// ArkScript Nil value
    const auto Nil = Value(ValueType::Nil);
    /// ArkScript False value
    const auto False = Value(ValueType::False);
    /// ArkScript True value
    const auto True = Value(ValueType::True);
}

#endif  // ARK_VM_DEFAULTVALUES_HPP
