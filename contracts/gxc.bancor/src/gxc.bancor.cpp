#include <gxc.bancor/gxc.bancor.hpp>

namespace gxc {

constexpr name null_account = "gxc.null"_n;

void bancor_contract::convert(name sender, extended_asset from, extended_asset to) {
   require_auth(sender);

   configuration cfg(_self, _self.value);
   check(cfg.exists(), "contract not initialized");

   if (from.get_extended_symbol() == cfg.get().get_connected_symbol()) {
      // initialize connector or buy smart
      connectors conn(_self, to.contract.value);
      auto it = conn.find(to.quantity.symbol.code().raw());
      check(it != conn.end(), "connector not exists");

      conn.modify(it, same_payer, [&](auto& c) {
         charges chrg(_self, to.contract.value);
         auto cit = chrg.find(to.quantity.symbol.code().raw());
         auto fee = cit != chrg.end() ? cit->get_fee(from)
                  : !cfg.get().is_exempted() ? cfg.get().get_fee(from)
                  : extended_asset(0, from.get_extended_symbol());

         auto quant_after_fee = from - fee;
         check(quant_after_fee.quantity.amount > 0, "paid token not enough after charging fee");

         auto smart_issued = c.convert_to_smart(quant_after_fee, to.get_extended_symbol());
         check(smart_issued.value.quantity.amount > 0, "paid token not enough after charging fee");

         auto refund = extended_asset{int64_t(quant_after_fee.quantity.amount * (1 - smart_issued.ratio)), quant_after_fee.get_extended_symbol()};
         if (refund.quantity.amount > 0) {
            auto overcharged = int64_t(fee.quantity.amount * (1 - smart_issued.ratio));
            if (overcharged < 0) overcharged = 0;
            fee.quantity.amount -= overcharged;
            refund.quantity.amount += overcharged;
         }
         token(from.contract, _self).transfer(sender, _self, from - refund);
         if (fee.quantity.amount > 0) {
            token(from.contract, _self).transfer(_self, cfg.get().owner, fee, "conversion fee");
         }
         token(to.contract).transfer(null_account, sender, smart_issued.value);
         //print("effective_price = ", asset((from.quantity.amount - (fee.quantity.amount + refund.amount)) / smart_issued.value.quantity.amount * pow(10, smart_issued.value.quantity.symbol.precision()), quantity.symbol));
      });
   } else {
      // sell smart
      connectors conn(_self, from.contract.value);
      auto it = conn.find(from.quantity.symbol.code().raw());
      check(it != conn.end(), "connector not exists");

      conn.modify(it, same_payer, [&](auto& c) {
         auto connected_out = c.convert_from_smart(from, cfg.get().get_connected_symbol());

         charges chrg(_self, from.contract.value);
         auto cit = chrg.find(from.quantity.symbol.code().raw());
         auto fee = cit != chrg.end() ? cit->get_fee(connected_out.value)
                  : !cfg.get().is_exempted() ? cfg.get().get_fee(connected_out.value)
                  : extended_asset(0, connected_out.value.get_extended_symbol());

         auto quant_after_fee = connected_out.value - fee;
         check(quant_after_fee.quantity.amount > 0, "paid token not enough after charging fee");

         auto refund = extended_asset{int64_t(from.quantity.amount * (1 - connected_out.ratio)), from.get_extended_symbol()};
         token(from.contract, _self).transfer(sender, _self, from - refund);
         token(from.contract, _self).transfer(_self, null_account, from - refund);
         token(cfg.get().connected_contract, _self).transfer(_self, sender, quant_after_fee);
         if (fee.quantity.amount > 0) {
            token(cfg.get().connected_contract, _self).transfer(_self, cfg.get().owner, fee, "conversion fee");
         }
         //print("effective_price = ", asset(quant_after_fee.quantity.amount / (from.quantity.amount - refund.amount) * pow(10, from.quantity.symbol.precision()), connected_out.value.quantity.symbol));
      });
   }
}

void bancor_contract::init(name owner, extended_symbol connected) {
   require_auth(_self);

   configuration cfg(_self, _self.value);
   check(!cfg.exists(), "already initialized");
   cfg.set({asset{0, connected.get_symbol()}, 0, owner, connected.get_contract()}, _self);
}

void bancor_contract::connect(extended_symbol smart, extended_asset balance, double weight) {
   require_auth(_self);

   configuration cfg(_self, _self.value);
   check(cfg.exists(), "contract not initialized");
   check(cfg.get().get_connected_symbol() == balance.get_extended_symbol(), "balance should be paid by connected token");

   connectors conn(_self, smart.get_contract().value);
   auto it = conn.find(smart.get_symbol().code().raw());
   check(it == conn.end(), "existing connector");
   conn.emplace(_self, [&](auto& c) {
      c.smart = smart;
      c.balance = balance.quantity;
      c.weight = weight;
   });

   token(balance.contract, _self).transfer(cfg.get().owner, _self, balance);
}

void bancor_contract::setcharge(int16_t rate, std::optional<extended_asset> fixed, std::optional<extended_symbol> smart) {
   require_auth(_self);

   configuration cfg(_self, _self.value);
   check(cfg.exists(), "initialize contract before setting charge");
   check(!fixed || cfg.get().get_connected_symbol() == fixed->get_extended_symbol(), "represent conversion fee in connected token");

   if (!smart) {
      check(rate >= 0 && rate <= 1000, "rate needs to be in the range of 0-1000 (per mille)");
      auto it = cfg.get();
      it.rate = static_cast<uint16_t>(rate);
      if (fixed)
         it.connected = fixed->quantity;
      cfg.set(it, _self);
   } else {
      charges chrg(_self, smart->get_contract().value);
      auto it = chrg.find(smart->get_symbol().code().raw());

      if (rate == -1) {
         check(it != chrg.end(), "no charge policy to be deleted");
         chrg.erase(it);
         return;
      }

      check(rate >= 0 && rate <= 1000, "rate needs to be in the range of 0-1000 (per mille)");
      if (it == chrg.end()) {
         chrg.emplace(_self, [&](auto& c) {
            c.smart = *smart;
            c.rate = static_cast<uint16_t>(rate);
            c.connected = fixed ? fixed->quantity : asset{ 0, cfg.get().connected.symbol };
         });
      } else {
         chrg.modify(it, same_payer, [&](auto& c) {
            c.rate = static_cast<uint16_t>(rate);
            if (fixed)
               c.connected = fixed->quantity;
         });
      }
   }
}

void bancor_contract::setowner(name owner) {
   require_auth(_self);

   configuration cfg(_self, _self.value);
   check(cfg.exists(), "initialize contract before setting owner");

   auto it = cfg.get();
   it.owner = owner;
   cfg.set(it, _self);
}

} /// namespace gxc
