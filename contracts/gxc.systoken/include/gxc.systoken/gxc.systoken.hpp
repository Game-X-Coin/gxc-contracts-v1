/**
 *  @file
 *  @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp> 
#include <eosiolib/binary_extension.hpp>
#include <eosiolib/singleton.hpp>

#include <gxclib/token.hpp>
#include <gxclib/game.hpp>

using namespace eosio;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.systoken")]] systoken : public contract  {
public:
   using contract::contract;

   [[eosio::action]] void create (name issuer, asset maximum_supply);
   void issue (name to, asset quantity, string memo, name issuer);
   void retire (asset quantity, string memo, name issuer);
   [[eosio::action]] void transfer (name from, name to, asset quantity, string memo, name issuer);

private:
   struct [[eosio::table("accounts"), eosio::contract("gxc.systoken")]] accountsrow {
      asset balance;

      uint64_t  primary_key()const { return balance.symbol.code().raw(); }

      EOSLIB_SERIALIZE(accountsrow, (balance));
   };

   struct [[eosio::table("stat"), eosio::contract("gxc.systoken")]] statsrow {
      asset supply;
      asset max_supply;
      name  issuer;

      uint64_t primary_key()const { return supply.symbol.code().raw(); }

      EOSLIB_SERIALIZE(statsrow, (supply)(max_supply)(issuer))
   };

   typedef eosio::multi_index<"accounts"_n, accountsrow>  accountstable;
   typedef eosio::multi_index<"stat"_n, statsrow > statstable;

   void sub_balance(name owner, asset value);
   void add_balance(name owner, asset value);

   inline void _check_args(asset quantity, std::optional<string> memo = std::nullopt) {
      eosio_assert(quantity.symbol.is_valid(), "invalid symbol name");
      eosio_assert(quantity.is_valid(), "invalid quantity");
      eosio_assert(quantity.amount > 0, "must be positive quantity");
      eosio_assert(!memo || (*memo).size() <= 256, "memo has more than 256 bytes");
   }
};

}
