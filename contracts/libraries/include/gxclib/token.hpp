/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio-xt/crypto.hpp>
#include "symbol.hpp"

#include <cmath>

namespace gxc {

using namespace eosio;
using namespace eosio::internal_use_do_not_use;

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
                          xxh3_64(reinterpret_cast<const char*>(&esc), sizeof(uint128_t))),
              reinterpret_cast<void*>(&balance), sizeof(asset));
   return balance;
}

}
