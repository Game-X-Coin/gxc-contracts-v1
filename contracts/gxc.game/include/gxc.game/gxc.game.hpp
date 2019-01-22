/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <gxclib/game.hpp>

using namespace eosio;

namespace gxc { namespace game {

class [[eosio::contract("gxc.game")]] contract : public eosio::contract {
public:
   using eosio::contract::contract;

   [[eosio::action]] void setgame (name account_name, bool is_game);

private:
   struct [[eosio::table, eosio::contract("gxc.game")]] gamerow {
      name account_name;

      uint64_t primary_key()const { return account_name.value; }

      EOSLIB_SERIALIZE(gamerow, (account_name))
   };

   typedef multi_index<"game"_n, gamerow> gametable;
};

} }
