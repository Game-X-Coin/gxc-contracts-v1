/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/eosio.hpp>
#include <eosio/producer_schedule.hpp>

#include "types.hpp"

namespace eosio { namespace chain {

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
   uint32_t                              threshold = 0;
   std::vector<key_weight>               keys;
   std::vector<permission_level_weight>  accounts;
   std::vector<wait_weight>              waits;

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

struct newaccount {
   name creator;
   name name;
   authority owner;
   authority active;

   EOSLIB_SERIALIZE(newaccount, (creator)(name)(owner)(active))
};

} }
