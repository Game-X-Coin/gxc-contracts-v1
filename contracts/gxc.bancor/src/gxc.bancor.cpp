#include <gxc.bancor/gxc.bancor.hpp>
#include <eoslib/dlog.hpp>

namespace gxc {

constexpr name null_account = "gxc.null"_n;

extended_asset bancor_contract::get_fee(const extended_asset& value, const extended_asset& smart, const config& c, bool required) {
   extended_asset fee = { 0, value.get_extended_symbol() };
   bool exempted = true;

   charges chrg(_self, smart.contract.value);
   auto it = chrg.find(smart.quantity.symbol.code().raw());

   if (it != chrg.end()) {
      exempted = it->is_exempted();
      if (!exempted) {
         if (!required) fee = it->get_fee(value);
         else fee = it->get_required_fee(value);
      }
   } else {
      exempted = c.is_exempted();
      if (!exempted) {
         if (!required) fee = c.get_fee(value);
         else fee = c.get_required_fee(value);
      }
   }
   if (!exempted && fee.quantity.amount <= 0) {
      fee.quantity.amount = 1;
   }

   return fee;
}

void bancor_contract::convert(name sender, extended_asset from, extended_asset to) {
   require_auth(sender);

   check((from.quantity.amount > 0) ^ (to.quantity.amount > 0), "Either `from` or `to` should be positive");

   configuration cfg(_self, _self.value);
   check(cfg.exists(), "contract not initialized");

   if (from.get_extended_symbol() == cfg.get().get_connected_symbol()) {
      // initialize connector or buy smart
      connectors conn(_self, to.contract.value);
      auto it = conn.find(to.quantity.symbol.code().raw());
      check(it != conn.end(), "connector not exists");

      conn.modify(it, same_payer, [&](auto& c) {
         if (to.quantity.amount == 0) {
            auto fee = get_fee(from, to, cfg.get());

            auto quant_after_fee = from - fee;
            check(quant_after_fee.quantity.amount > 0, "paid token not enough after charging fee");

            auto smart_issued = c.convert_to_smart(quant_after_fee, to.get_extended_symbol());
            check(smart_issued.value.quantity.amount > 0, "paid token not enough for conversion");

            auto rsv = reserve(to.get_extended_symbol());
            if (rsv) {
               double unit_price = smart_issued.delta.amount * pow(10, smart_issued.value.quantity.symbol.precision())
                                 / double(smart_issued.value.quantity.amount) / pow(10, smart_issued.delta.symbol.precision());
               double rate = rsv.get_rate();
               if (rate > unit_price) {
                  c.balance -= smart_issued.delta;

                  double dS = quant_after_fee.quantity.amount / rate / pow(10, from.quantity.symbol.precision() - to.quantity.symbol.precision());
                  if (dS < 0) dS = 0;

                  auto conversion_rate = ((int64_t)dS) / dS;
                  smart_issued.value.quantity.amount = int64_t(dS);
                  smart_issued.delta.amount = quant_after_fee.quantity.amount - int64_t(quant_after_fee.quantity.amount * (1 - conversion_rate));
                  smart_issued.ratio = conversion_rate;

                  c.balance += smart_issued.delta;
               }
            }

            if ((quant_after_fee.quantity - smart_issued.delta).amount > 0) {
               auto overcharged = int64_t(fee.quantity.amount * (1 - smart_issued.ratio));
               if (overcharged < 0) overcharged = 0;
               fee.quantity.amount -= overcharged;
            }
            auto sum = extended_asset{smart_issued.delta, quant_after_fee.contract} + fee;

            token(from.contract, _self).transfer(sender, _self, sum, "bancor conversion");
            if (fee.quantity.amount > 0) {
               token(from.contract, _self).transfer(_self, cfg.get().owner, fee, "conversion fee");
            }
            token(to.contract).transfer(null_account, smart_issued.value.contract, smart_issued.value);
            token(to.contract).transfer(smart_issued.value.contract, sender, smart_issued.value);
            dlog("effective_price = ", asset(smart_issued.delta.amount * pow(10, smart_issued.value.quantity.symbol.precision()) / smart_issued.value.quantity.amount, from.quantity.symbol));
         } else {
            auto connected_required = c.convert_to_exact_smart(from.get_extended_symbol(), to);

            auto rsv = reserve(to.get_extended_symbol());
            if (rsv) {
               double unit_price = connected_required.delta.amount * pow(10, to.quantity.symbol.precision())
                                 / double(to.quantity.amount) / pow(10, connected_required.delta.symbol.precision());
               double rate = rsv.get_rate();
               if (rate > unit_price) {
                  c.balance -= connected_required.delta;

                  double dC = to.quantity.amount * rate / pow(10, to.quantity.symbol.precision() - from.quantity.symbol.precision());
                  if (dC < 0) dC = 0;

                  auto conversion_rate = ((int64_t)dC) / dC;
                  connected_required.value.quantity.amount = int64_t(dC);
                  connected_required.delta.amount = int64_t(dC);
                  connected_required.ratio = conversion_rate;

                  c.balance += connected_required.delta;
               }
            }

            auto fee = get_fee({connected_required.delta, connected_required.value.contract}, to, cfg.get(), true);

            token(from.contract, _self).transfer(sender, _self, extended_asset{connected_required.delta, from.contract} + fee, "bancor conversion");
            token(to.contract).transfer(null_account, to.contract, to);
            token(to.contract).transfer(to.contract, sender, to);
            if (fee.quantity.amount > 0) {
               token(from.contract, _self).transfer(_self, cfg.get().owner, fee, "conversion fee");
            }
            dlog("effective_price = ", asset(connected_required.delta.amount * pow(10, to.quantity.symbol.precision()) / to.quantity.amount, from.quantity.symbol));
         }
      });
   } else {
      // sell smart
      connectors conn(_self, from.contract.value);
      auto it = conn.find(from.quantity.symbol.code().raw());
      check(it != conn.end(), "connector not exists");

      conn.modify(it, same_payer, [&](auto& c) {
         if (to.quantity.amount == 0) {
            auto connected_out = c.convert_from_smart(from, cfg.get().get_connected_symbol());

            bool need_burn = false;
            auto rsv = reserve(from.get_extended_symbol());
            if (rsv) {
               double unit_price = connected_out.delta.amount * pow(10, from.quantity.symbol.precision())
                                 / double(from.quantity.amount) / pow(10, connected_out.delta.symbol.precision());
               double rate = rsv.get_rate();
               if (rate > unit_price) {
                  c.balance += connected_out.delta;

                  double dC = from.quantity.amount * rate / pow(10, from.quantity.symbol.precision() - to.quantity.symbol.precision());
                  if (dC < 0) dC = 0;

                  auto conversion_rate = ((int64_t)dC) / dC;
                  connected_out.value.quantity.amount = int64_t(dC);
                  connected_out.delta.amount = int64_t(dC);
                  connected_out.ratio = conversion_rate;

                  need_burn = true;
               }
            }

            auto fee = get_fee(connected_out.value, from, cfg.get());

            auto quant_after_fee = connected_out.value - fee;
            check(quant_after_fee.quantity.amount > 0, "paid token not enough after charging fee");

            auto refund = extended_asset{int64_t(from.quantity.amount * (1 - connected_out.ratio)), from.get_extended_symbol()};
            token(from.contract, _self).transfer(sender, _self, from - refund);
            if (!need_burn) {
               token(from.contract, _self).transfer(_self, null_account, from - refund);
            } else {
               token(from.contract, _self).approve(_self, reserve::reserve_account, from - refund);
               rsv.claim(_self, from - refund);
            }
            token(cfg.get().connected_contract, _self).transfer(_self, sender, quant_after_fee);
            if (fee.quantity.amount > 0) {
               token(cfg.get().connected_contract, _self).transfer(_self, cfg.get().owner, fee, "conversion fee");
            }
            dlog("effective_price = ", asset(connected_out.delta.amount * pow(10, from.quantity.symbol.precision()) / (from.quantity.amount - refund.quantity.amount), connected_out.value.quantity.symbol));
         } else {
            auto fee = get_fee(to, from, cfg.get(), true);
            auto smart_required = c.convert_exact_from_smart(from.get_extended_symbol(), to + fee);

            bool need_burn = false;
            auto rsv = reserve(from.get_extended_symbol());
            if (rsv) {
               double unit_price = smart_required.delta.amount * pow(10, smart_required.value.quantity.symbol.precision())
                                 / double(smart_required.value.quantity.amount) / pow(10, smart_required.delta.symbol.precision());
               double rate = rsv.get_rate();
               if (rate > unit_price) {
                  c.balance += smart_required.delta;

                  double dS = to.quantity.amount / rate / pow(10, to.quantity.symbol.precision() - from.quantity.symbol.precision());
                  if (dS < 0) dS = 0;

                  auto conversion_rate = ((int64_t)dS) / dS;
                  smart_required.value.quantity.amount = int64_t(dS);
                  smart_required.delta.amount = to.quantity.amount - int64_t(to.quantity.amount * (1 - conversion_rate));
                  smart_required.ratio = conversion_rate;

                  need_burn = true;
               }
            }

            to.quantity = smart_required.delta - fee.quantity;

            token(from.contract, _self).transfer(sender, _self, smart_required.value);
            if (!need_burn) {
               token(from.contract, _self).transfer(_self, null_account, smart_required.value);
            } else {
               token(from.contract, _self).approve(_self, reserve::reserve_account, smart_required.value);
               rsv.claim(_self, smart_required.value);
            }
            token(to.contract, _self).transfer(_self, sender, to);
            if (fee.quantity.amount > 0) {
               token(to.contract, _self).transfer(_self, cfg.get().owner, fee, "conversion fee");
            }
            dlog("effective_price = ", asset(smart_required.delta.amount * pow(10, from.quantity.symbol.precision()) / (smart_required.value.quantity.amount), to.quantity.symbol));
         }
      });
   }
}

void bancor_contract::init(name owner, extended_symbol connected) {
   require_auth(_self);

   configuration cfg(_self, _self.value);
   check(!cfg.exists(), "already initialized");
   cfg.set({0, asset{0, connected.get_symbol()}, connected.get_contract(), owner}, _self);
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
