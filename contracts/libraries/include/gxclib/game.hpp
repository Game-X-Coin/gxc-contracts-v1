/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include "action.hpp"

namespace gxc {

using eosio::name;
using eosio::check;

constexpr name game_account = "gxc.game"_n;

   struct [[eosio::table, eosio::contract("gxc.game")]] game {
      name        name;
      std::string uri;

      uint64_t primary_key()const { return name.value; }

      EOSLIB_SERIALIZE(game, (name)(uri))
   };

   typedef eosio::multi_index<"game"_n, game> games;

   void check_is_game(name name) {
      games gms(game_account, game_account.value);
      auto it = gms.find(basename(name).value);

      check(it != gms.end(), "not registered to game account");
   }

}
