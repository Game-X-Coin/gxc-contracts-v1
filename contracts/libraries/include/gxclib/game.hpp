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
      name        name; /**< Game account name */
      std::string uri;  /**< Game metadata */

      uint64_t primary_key()const { return name.value; }

      EOSLIB_SERIALIZE(game, (name)(uri))
   };

   typedef eosio::multi_index<"game"_n, game> games;

   /**
    * Verifies that @ref name has game auth.
    * Game auth is required to mint game token.
    *
    * @brief Verifies that @ref name has game auth.
    * @param name - name of the account to be verified
    */
   bool has_gauth(name name) {
      games gms(game_account, game_account.value);
      return gms.find(basename(name).value) != gms.end();
   }

   /**
    * Verifies that @ref name has game auth. Fails if not found.
    * Game auth is required to mint game token.
    *
    * @brief Verifies that @ref name has game auth.
    * @param name - name of the account to be verified
    */
   void require_gauth(name name) {
      check(has_gauth(name), "not registered to game account");
   }
}
