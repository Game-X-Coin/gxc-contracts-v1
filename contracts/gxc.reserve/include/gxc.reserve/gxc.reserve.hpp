/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/eosio.hpp>

#include <gxclib/system.hpp>
#include <gxclib/game.hpp>
#include <gxclib/token.hpp>
#include <gxclib/action.hpp>

using namespace eosio;
using gxc::get_float_amount;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.reserve")]] reserve : public eosio::contract {
public:
   using contract::contract;
   using key_value = std::pair<std::string, std::vector<int8_t>>;

   [[eosio::action]]
   void mint(extended_asset derivative, extended_asset underlying, std::vector<key_value> opts);

   [[eosio::action]]
   void claim(name owner, extended_asset value);

   struct [[eosio::table("reserve"), eosio::contract("gxc.reserve")]] currency_reserves {
      asset derivative;
      name  issuer;
      asset underlying;

      uint64_t primary_key()const { return derivative.symbol.code().raw(); }

      EOSLIB_SERIALIZE( currency_reserves, (derivative)(issuer)(underlying) )
   };

   typedef multi_index<"reserve"_n, currency_reserves> reserves;
};

}
