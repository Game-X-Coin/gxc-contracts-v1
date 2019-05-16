#include <gxc.htlc/gxc.htlc.hpp>

#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

namespace gxc {

void htlc_contract::newcontract(name owner, string contract_name, std::variant<name, checksum160> recipient, extended_asset value, checksum256 hashlock, time_point_sec timelock) {
   require_auth(owner);

   htlcs idx(_self, owner.value);
   check(idx.find(htlc::hash(contract_name)) == idx.end(), "existing contract name");
   check(timelock > current_time_point(), "the expiration time should be in the future");

   idx.emplace(owner, [&](auto& lck) {
      lck.contract_name = contract_name;
      lck.recipient = recipient;
      lck.value = value;
      lck.hashlock = hashlock;
      lck.timelock = timelock;
   });

   transfer_action("gxc.token"_n, {{_self, "active"_n}}).send(owner, _self, value, "CREATED BY " + owner.to_string() + (contract_name.size() ? ", " : "") + contract_name);
}

void htlc_contract::withdraw(name owner, string contract_name, checksum256 preimage) {
   htlcs idx(_self, owner.value);
   const auto& it = idx.get(htlc::hash(contract_name));

   // `preimage` works as a key here.
   //require_auth(it.recipient);

   auto data = preimage.extract_as_byte_array();
   auto hash = eosio::sha256(reinterpret_cast<const char*>(data.data()), data.size());
   check(memcmp((const void*)it.hashlock.data(), (const void*)hash.data(), 32) == 0, "invalid preimage");

   if (std::holds_alternative<name>(it.recipient))
      transfer_action("gxc.token"_n, {{_self, "active"_n}}).send(_self, std::get<name>(it.recipient), it.value, "FROM " + owner.to_string() + (contract_name.size() ? ", " : "") + contract_name);
   else
      transfer_action("gxc.token"_n, {{_self, "active"_n}}).send(_self, "gxc.vault"_n, it.value, "FROM " + owner.to_string() + (contract_name.size() ? ", " : "") + contract_name);

   idx.erase(it);
}

void htlc_contract::refund(name owner, string contract_name) {
   require_auth(owner);

   htlcs idx(_self, owner.value);
   const auto& it = idx.get(htlc::hash(contract_name));

   check(it.timelock < current_time_point(), "contract not expired");

   if (std::holds_alternative<name>(it.recipient))
      transfer_action("gxc.token"_n, {{_self, "active"_n}}).send(_self, owner, it.value, "REFUNDED FROM " + std::get<name>(it.recipient).to_string() + (contract_name.size() ? ", " : "") + contract_name);
   else
      transfer_action("gxc.token"_n, {{_self, "active"_n}}).send(_self, owner, it.value, "");

   idx.erase(it);
}

}
