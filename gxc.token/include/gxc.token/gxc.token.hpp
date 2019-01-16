/**
 *  @file
 *  @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#include <gxclib/token.hpp>
#include <gxclib/game.hpp>

using namespace eosio;
using std::string;

namespace gxc { namespace token {

class [[eosio::contract("gxc.token")]] contract : public eosio::contract {
public:
   using eosio::contract::contract;

   [[eosio::action]] void create (name issuer, asset maximum_supply, binary_extension<name> type);
   [[eosio::action]] void retire (asset quantity, string memo, binary_extension<name> issuer);
   [[eosio::action]] void transfer (name from, name to, asset quantity, string memo, binary_extension<name> issuer);
   [[eosio::action]] void setadmin (name account_name, name action_name, bool is_admin);

private:
   struct [[eosio::table("token"), eosio::contract("gxc.token")]] tokenrow {
      name     type;
      symbol   sym;

      uint64_t  primary_key()const { return sym.code().raw(); }

      EOSLIB_SERIALIZE(tokenrow, (type)(sym))
   };

   struct [[eosio::table("admin"), eosio::contract("gxc.token")]] adminrow {
      name account_name;

      uint64_t primary_key()const { return account_name.value; }

      EOSLIB_SERIALIZE(adminrow, (account_name))
   };

   typedef multi_index<"token"_n, tokenrow> tokentable;
   typedef multi_index<"admin"_n, adminrow> admintable;

   inline name get_token_type(symbol_code sym_code, name issuer) {
      name type;
      db_get_i64(db_find_i64(_self.value, issuer.value, "token"_n.value, sym_code.raw()), &type, sizeof(type));
      return type;
   }

   inline bool has_admin_auth(name action_name) {
      admintable at(_self, action_name.value);
      for (auto itr = at.lower_bound(name().value); itr != at.end(); itr++) {
         if (has_auth((*itr).account_name)) return true;
      }
      return false;
   }
};

} }

