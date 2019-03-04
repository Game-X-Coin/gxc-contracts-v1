/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/action.hpp>
#include "symbol.hpp"
#include "fasthash.h"

#include <cmath>

namespace gxc { 

using namespace eosio;

constexpr name token_account = "gxc.token"_n;

inline double get_float_amount(asset quantity) {
   return quantity.amount / (double)pow(10, quantity.symbol.precision());
}

asset get_supply(name issuer, symbol_code sym_code) {
   asset supply;
   db_get_i64(db_find_i64(token_account.value, issuer.value, "stat"_n.value, sym_code.raw()),
              reinterpret_cast<void*>(&supply), sizeof(asset));
   return supply;
}

asset get_balance(name owner, name issuer, symbol_code sym_code) {
   asset balance;
   auto esc = extended_symbol_code(sym_code, issuer);
   db_get_i64(db_find_i64(token_account.value, issuer.value, "accounts"_n.value,
                          fasthash64(reinterpret_cast<const void*>(&esc), sizeof(uint128_t), "accounts"_n.value)),
              reinterpret_cast<void*>(&balance), sizeof(asset));
   return balance;
}

}
