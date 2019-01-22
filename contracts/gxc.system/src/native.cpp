/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.system/gxc.system.hpp>

#include "exchange_state.cpp"
#include "delegate_bandwidth.cpp"

namespace gxc { namespace system {

void contract::onblock(ignore<block_header> header) {
   require_auth(_self);

   block_timestamp timestamp;
   _ds >> timestamp;

   _gstate.last_block_num = timestamp;
}

void contract::newaccount(name creator, name name, ignore<authority> owner, ignore<authority> active) {
   if (creator != _self) {
      eosio_assert(name.length() >= 6, "a name shorter than 6 is reserved");
      eosio_assert(!has_dot(name), "user account name cannot contain dot");

      user_resources_table userres(_self, name.value);

      userres.emplace(name, [&](auto& res) {
         res.owner = name;
         res.net_weight = asset(0, system::contract::get_core_symbol());
         res.cpu_weight = asset(0, system::contract::get_core_symbol());
      });

      set_resource_limits(name.value, 0 + ram_gift_bytes, 0, 0);
   }
}

void contract::setabi(name account, const std::vector<char>& abi) {
   eosio_assert(is_admin(account), "not allowed to normal account");

   eosio::multi_index<"abihash"_n, abi_hash> table(_self, _self.value);

   auto itr = table.find(account.value);
   if (itr == table.end()) {
      table.emplace(account, [&](auto& row) {
         row.owner= account;
         sha256(const_cast<char*>(abi.data()), abi.size(), &row.hash);
      });
   } else {
      table.modify(itr, same_payer, [&](auto& row) {
         sha256(const_cast<char*>(abi.data()), abi.size(), &row.hash);
      });
   }
}

void contract::setcode(name account, uint8_t vmtype, uint8_t vmversion, const std::vector<char>& code) {
   eosio_assert(is_admin(account), "not allowed to normal account");
}

} }
