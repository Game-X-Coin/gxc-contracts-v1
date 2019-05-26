#include <gxc.htlc/gxc.htlc.hpp>

#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <eoslib/hex.hpp>

namespace gxc {

constexpr auto vault_account = "gxc.vault"_n.value;

void htlc_contract::newcontract(name owner, string contract_name, std::variant<name, checksum160> recipient, extended_asset value, checksum256 hashlock, time_point_sec timelock) {
   require_auth(owner);

   htlcs idx(_self, owner.value);
   check(idx.find(htlc::hash(contract_name)) == idx.end(), "existing contract name");

   configs cfg(_self, _self.value);
   auto it = cfg.find(config::hash(value));

   bool constrained = owner.value != vault_account && it != cfg.end();

   auto min_amount = (constrained) ? it->min_amount : extended_asset(0, extended_symbol(value.quantity.symbol, value.contract));
   auto min_duration = (constrained) ? it->min_duration : 0;

   check(value >= min_amount, "specified amount is not enough");
   check(timelock >= current_time_point() + microseconds(static_cast<int64_t>(min_duration * 1000)), "the expiration time should be in the future");

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

   if (std::holds_alternative<name>(it.recipient)) {
      transfer_action("gxc.token"_n, {{_self, "active"_n}}).send(_self, owner, it.value, "REFUNDED FROM " + std::get<name>(it.recipient).to_string() + (contract_name.size() ? ", " : "") + contract_name);
   } else {
      auto bytes = std::get<checksum160>(it.recipient).extract_as_byte_array();
      auto str_recipient = to_hex(reinterpret_cast<const char*>(bytes.data()), bytes.size());
      transfer_action("gxc.token"_n, {{_self, "active"_n}}).send(_self, owner, it.value, "REFUNDED FROM " + str_recipient + (contract_name.size() ? ", " : "") + contract_name);
   }

   idx.erase(it);
}

void htlc_contract::setconfig(extended_asset min_amount, uint32_t min_duration) {
   require_auth(min_amount.contract);

   check(min_amount.quantity.symbol.is_valid(), "invalid symbol name `" + min_amount.quantity.symbol.code().to_string() + "`");
   check(min_amount.quantity.is_valid(), "invalid quantity");
   check(min_amount.quantity.amount >= 0, "must not be negative quantity");
   
   configs idx(_self, _self.value);
   auto it = idx.find(config::hash(min_amount));

   if (it != idx.end()) {
      idx.modify(it, same_payer, [&](auto& c) {
         c.min_amount = min_amount;
         c.min_duration = min_duration;
      });
   } else {
      idx.emplace(min_amount.contract, [&](auto& c) {
         c.min_amount = min_amount;
         c.min_duration = min_duration;
      });
   }
}

}
