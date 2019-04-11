/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/eosio.hpp>
#include <eosio/producer_schedule.hpp>
#include "types.hpp"

namespace eosio {

inline bool operator< (const permission_level& lhs, const permission_level& rhs) {
   return std::tie(lhs.actor, lhs.permission) < std::tie(rhs.actor, rhs.permission);
}

struct permission_level_weight {
   permission_level  permission;
   uint16_t          weight;

   EOSLIB_SERIALIZE(permission_level_weight, (permission)(weight))
};

struct key_weight {
   eosio::public_key  key;
   uint16_t           weight;

   EOSLIB_SERIALIZE(key_weight, (key)(weight))
};

struct wait_weight {
   uint32_t           wait_sec;
   uint16_t           weight;

   EOSLIB_SERIALIZE(wait_weight, (wait_sec)(weight))
};

struct authority {
   uint32_t                              threshold = 1;
   std::vector<key_weight>               keys;
   std::vector<permission_level_weight>  accounts;
   std::vector<wait_weight>              waits;

   authority& add_account(name auth, name _permission = "active"_n) {
      auto p = permission_level_weight{{auth, _permission}, 1};

      auto itr = std::lower_bound(accounts.begin(), accounts.end(), p, [](const auto& e, const auto& v) {
            return e.permission < v.permission;
            });
      accounts.insert(itr, p);
      return *this;
   }

   authority& add_code(name auth) {
      return add_account(auth, "gxc.code"_n);
   }

   EOSLIB_SERIALIZE(authority, (threshold)(keys)(accounts)(waits))
};

struct block_header {
   uint32_t                                  timestamp;
   name                                      producer;
   uint16_t                                  confirmed = 0;
   capi_checksum256                          previous;
   capi_checksum256                          transaction_mroot;
   capi_checksum256                          action_mroot;
   uint32_t                                  schedule_version = 0;
   std::optional<eosio::producer_schedule>   new_producers;

   EOSLIB_SERIALIZE(block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)
                                  (schedule_version)(new_producers))
};

}
