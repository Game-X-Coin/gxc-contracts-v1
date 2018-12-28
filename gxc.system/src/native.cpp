/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.system/gxc.system.hpp>
#include <gxclib/name.hpp>

#include "exchange_state.cpp"
#include "delegate_bandwidth.cpp"

namespace gxc { namespace system {

void contract::onblock(ignore<block_header> header) {
   require_auth(_self);

   block_timestamp timestamp;
   _ds >> timestamp;

   _gstate.last_block_num = timestamp;
}

void contract::newaccount(name creator, name newacnt, ignore<authority> owner, ignore<authority> active) {
   if (creator != _self) {
      eosio_assert(newacnt.length() >= 6, "the names shorter than 6 are reserved");
      eosio_assert(has_dot(newacnt), "user name cannot contain dot");

      user_resources_table userres(_self, newacnt.value);

      userres.emplace(newacnt, [&](auto& res) {
         res.owner = newacnt;
         res.net_weight = asset(0, system::contract::get_core_symbol());
         res.cpu_weight = asset(0, system::contract::get_core_symbol());
      });

      set_resource_limits(newacnt.value, 0, 0, 0);
   }
}

void contract::setabi(name account, const std::vector<char>& abi) {
   eosio_assert(!starts_with(account, "gxc."), "not allowed to normal account");

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
   eosio_assert(!starts_with(account, "gxc."), "not allowed to normal account");
}

} }
