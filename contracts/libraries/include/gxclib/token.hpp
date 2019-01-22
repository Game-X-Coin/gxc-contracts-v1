/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/action.hpp>

#include <cmath>
#include "system.hpp"

namespace gxc { namespace token {

using namespace eosio;

constexpr name account = "gxc.token"_n;
constexpr name system_token = "token.sys"_n;
constexpr name standard_token = "token.std"_n;

struct _statsrow {
   asset    supply;     // 16
   asset    max_supply; // 32
   name     issuer;     // 40

   EOSLIB_SERIALIZE(_statsrow, (supply)(max_supply)(issuer))
};

struct _accountsrow {
   asset    balance;    // 16

   EOSLIB_SERIALIZE(_accountsrow, (balance))
};

inline long double get_float_amount(asset quantity) {
   return quantity.amount / (long double)pow(10, quantity.symbol.precision());
}

inline uint128_t extended_symbol_code(symbol_code sym_code, name account) {
   return (uint128_t)sym_code.raw() << 64 | account.value;
}

inline asset get_supply(name issuer, symbol_code sym_code) {
   name ttype;
   db_get_i64(db_find_i64(token::account.value, issuer.value, "token"_n.value, sym_code.raw()), &ttype, sizeof(ttype));

   _statsrow st;
   db_get_i64(db_find_i64(ttype.value, issuer.value, "stat"_n.value, sym_code.raw()), &st, sizeof(st));

   return st.supply;
}

inline asset get_balance(name issuer, name owner, symbol_code sym_code) {
   name ttype;
   db_get_i64(db_find_i64(token::account.value, issuer.value, "token"_n.value, sym_code.raw()), &ttype, sizeof(ttype));

   _accountsrow ac;

   switch (ttype.value) {
      case standard_token.value:
         {
            uint64_t pk;
            auto sk = extended_symbol_code(sym_code, issuer);

            auto itr = db_idx128_find_secondary(ttype.value, owner.value, "accounts"_n.value, &sk, &pk);
            eosio_assert(itr != db_idx128_end(ttype.value, owner.value, "accounts"_n.value), "not found");

            db_get_i64(db_find_i64(ttype.value, owner.value, "accounts"_n.value, pk), &ac, sizeof(ac));
         }
         break;
      default:
         db_get_i64(db_find_i64(ttype.value, owner.value, "accounts"_n.value, sym_code.raw()), &ac, sizeof(ac));
         break;
   }

   return ac.balance;
}

inline void validate(std::optional<name> account_name = std::nullopt) {
   eosio_assert(has_auth(account), "direct push action not allowed");
   if (account_name) {
      require_auth(*account_name);
   }
}

} }
