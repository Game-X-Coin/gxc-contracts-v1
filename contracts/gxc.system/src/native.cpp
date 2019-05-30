/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <eosio/crypto.hpp>
#include <gxc.system/gxc.system.hpp>

namespace gxc {

void system_contract::onblock(ignore<block_header> header) {
   require_auth(_self);

   block_timestamp timestamp;
   _ds >> timestamp;

   _gstate.last_block_num = timestamp;
}

void system_contract::newaccount(name creator, name name, ignore<authority> owner, ignore<authority> active) {
   require_auth(_self);

   if (creator != _self) {
      check(name.length() >= 6, "a name shorter than 6 is reserved");
      check(!has_dot(name), "user account name cannot contain dot");

      user_resources_table userres(_self, name.value);

      userres.emplace(name, [&](auto& res) {
         res.owner = name;
         res.net_weight = asset(0, system_contract::get_core_symbol());
         res.cpu_weight = asset(0, system_contract::get_core_symbol());
      });

      eosio::set_resource_limits(name, 0 + ram_gift_bytes, 0, 0);

      action({{name, active_permission}}, user_account, "payram4nick"_n, name).send();
   }
}

void system_contract::setabi(name account, const std::vector<char>& abi) {
   check(is_admin(account), "not allowed to normal account");

   eosio::multi_index<"abihash"_n, abi_hash> table(_self, _self.value);

   auto itr = table.find(account.value);
   if (itr == table.end()) {
      table.emplace(account, [&](auto& row) {
         row.owner = account;
         auto hash = eosio::sha256(const_cast<char*>(abi.data()), abi.size()).extract_as_byte_array();
         std::copy(hash.begin(), hash.end(), row.hash.begin());
      });
   } else {
      table.modify(itr, same_payer, [&](auto& row) {
         auto hash = eosio::sha256(const_cast<char*>(abi.data()), abi.size()).extract_as_byte_array();
         std::copy(hash.begin(), hash.end(), row.hash.begin());
      });
   }
}

void system_contract::setcode(name account, uint8_t vmtype, uint8_t vmversion, const std::vector<char>& code) {
   check(is_admin(account), "not allowed to normal account");
}

}
