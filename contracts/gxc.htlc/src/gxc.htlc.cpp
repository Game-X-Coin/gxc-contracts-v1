#include <gxc.htlc/gxc.htlc.hpp>

#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

using std::string;

namespace gxc {

void htlc_contract::on_transfer(name from, name to, asset quantity, string memo) {
   if (from == _self) return;

   htlcs idx(_self, from.value);
   auto it = idx.get(name(memo).value);

   check(!it.activated, "contract is already activated");
   check(it.value == extended_asset{quantity, get_first_receiver()}, "token amount not match: " + it.value.quantity.to_string() + "@" + it.value.contract.to_string());

   idx.modify(it, same_payer, [&](auto& lck) {
      lck.activated = true;
   });
}

void htlc_contract::newcontract(name owner, name contract_name, name recipient, extended_asset value, checksum256 hashlock, time_point_sec timelock) {
   require_auth(owner);

   htlcs idx(_self, owner.value);
   check(idx.find(contract_name.value) == idx.end(), "existing contract name");
   check(timelock > current_time_point(), "the expiration time should be in the future");

   idx.emplace(owner, [&](auto& lck) {
      lck.contract_name = contract_name;
      lck.recipient = recipient;
      lck.value = value;
      lck.hashlock = hashlock;
      lck.timelock = timelock;
   });
}

void htlc_contract::withdraw(name owner, name contract_name, checksum256 preimage) {
   htlcs idx(_self, owner.value);
   auto it = idx.get(contract_name.value);
   check(it.activated, "contract not activated");

   // `preimage` works as a key here.
   //require_auth(it.recipient);

   auto data = preimage.extract_as_byte_array();
   auto hash = eosio::sha256(reinterpret_cast<const char*>(data.data()), data.size());
   check(memcmp((const void*)it.hashlock.data(), (const void*)hash.data(), 32) == 0, "invalid preimage");

   transfer_action(it.value.contract, {{_self, "active"_n}}).send(_self, it.recipient, it.value.quantity, "");

   idx.erase(it);
}

void htlc_contract::cancel(name owner, name contract_name) {
   require_auth(owner);

   htlcs idx(_self, owner.value);
   auto it = idx.get(contract_name.value);

   check(it.timelock < current_time_point(), "contract not expired");

   if (it.activated)
      transfer_action(it.value.contract, {{_self, "active"_n}}).send(_self, owner, it.value.quantity, "");

   idx.erase(it);
}

}
