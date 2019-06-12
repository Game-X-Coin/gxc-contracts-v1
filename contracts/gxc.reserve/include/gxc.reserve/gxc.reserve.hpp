/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

#include <gxclib/system.hpp>
#include <gxclib/game.hpp>
#include <gxclib/action.hpp>

using namespace eosio;

namespace gxc {

class [[eosio::contract("gxc.reserve")]] reserve : public eosio::contract {
public:
   using contract::contract;
   using key_value = std::pair<std::string, std::vector<int8_t>>;

   [[eosio::action]]
   void mint(extended_asset derivative, extended_asset underlying, std::vector<key_value> opts);

   struct [[eosio::table("reserve"), eosio::contract("gxc.reserve")]] currency_reserves {
      extended_asset derivative;
      asset underlying;
      double rate;

      uint64_t primary_key()const { return derivative.quantity.symbol.code().raw(); }

      EOSLIB_SERIALIZE( currency_reserves, (derivative)(underlying)(rate) )
   };

   typedef multi_index<"reserve"_n, currency_reserves> reserves;

#ifdef TARGET_TESTNET
   [[eosio::action]]
   void migrate(extended_symbol derivative);
#endif
};

}
