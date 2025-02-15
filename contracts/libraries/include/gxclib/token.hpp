/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eoslib/crypto.hpp>
#include <eoslib/symbol.hpp>

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
#ifdef TARGET_TESTNET
                          fasthash64(reinterpret_cast<const char*>(&esc), sizeof(uint128_t))),
#else
                          xxh64(reinterpret_cast<const char*>(&esc), sizeof(uint128_t))),
#endif
              reinterpret_cast<void*>(&balance), sizeof(asset));
   return balance;
}

struct token_contract_mock {
   token_contract_mock(name auth) {
      auths.emplace_back(permission_level(auth, active_permission));
   }

   token_contract_mock& with(name auth) {
      auths.emplace_back(permission_level(auth, active_permission));
      return *this;
   }

   using key_value = std::pair<std::string, std::vector<int8_t>>;

   void mint(extended_asset value, std::vector<key_value> opts) {
      action_wrapper<"mint"_n, &token_contract_mock::mint>(std::move(name(token_account)), auths)
      .send(value, opts);
   }

   void transfer(name from, name to, extended_asset value, std::string memo) {
      action_wrapper<"transfer"_n, &token_contract_mock::transfer>(std::move(name(token_account)), auths)
      .send(from, to, value, memo);
   }

   void burn(extended_asset value, std::string memo) {
      action_wrapper<"burn"_n, &token_contract_mock::burn>(std::move(name(token_account)), auths)
      .send(value, memo);
   }

   std::vector<permission_level> auths;
};

}
