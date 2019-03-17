/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/name.hpp>
#include <eosiolib/db.h>
#include "action.hpp"

namespace gxc {

using eosio::name;

constexpr name game_account = "gxc.game"_n;

inline bool check_is_game(name name) {
   auto it = db_find_i64(game_account.value, game_account.value, "game"_n.value, basename(name).value);
   return it != db_end_i64(game_account.value, game_account.value, "game"_n.value);
}

}
