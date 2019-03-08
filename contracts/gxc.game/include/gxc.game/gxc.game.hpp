/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>

using namespace eosio;

namespace gxc {

class [[eosio::contract("gxc.game")]] game_contract : public contract {
public:
   using eosio::contract::contract;

   [[eosio::action]]
   void setgame(name name, bool activated);

   [[eosio::action]]
   void seturi(name name, std::string uri);

private:
   struct [[eosio::table, eosio::contract("gxc.game")]] game {
      name        name;
      std::string uri;

      uint64_t primary_key()const { return name.value; }

      EOSLIB_SERIALIZE(game, (name)(uri))
   };

   typedef multi_index<"game"_n, game> games;
};

}
