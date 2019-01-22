/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/name.hpp>
#include <eosiolib/db.h>

namespace gxc { namespace game {

using eosio::name;

constexpr name account = "gxc.game"_n;

inline bool is_game(name account_name) {
   auto itr = db_find_i64(account.value, account.value, "game"_n.value, account_name.value);
   return itr != db_end_i64(account.value, account.value, "game"_n.value);
}

} }
