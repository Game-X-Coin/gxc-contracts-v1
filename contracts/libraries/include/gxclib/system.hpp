/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/symbol.hpp>

namespace gxc { namespace system {

using eosio::name;
using eosio::symbol;

constexpr name account = "gxc"_n;

constexpr name owner_permission = "owner"_n;
constexpr name active_permission = "active"_n;
constexpr name code_permission = "gxc.code"_n;

constexpr symbol core_symbol = symbol("GXC", 4);

} }
