/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/eosio.hpp>

using namespace eosio;

namespace gxc {

class [[eosio::contract("gxc.game")]] game_contract : public contract {
public:
   using eosio::contract::contract;

   [[eosio::action]]
   void setgame(name name, bool activated);

   [[eosio::action]]
   void seturi(name name, std::string uri);
};

}
