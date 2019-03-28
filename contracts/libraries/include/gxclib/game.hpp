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

void check_is_game(name name) {
   auto it = eosio::internal_use_do_not_use::db_find_i64(game_account.value, game_account.value, "game"_n.value, basename(name).value);
   check(it != eosio::internal_use_do_not_use::db_end_i64(game_account.value, game_account.value, "game"_n.value), "not registered to game account");
}

}
