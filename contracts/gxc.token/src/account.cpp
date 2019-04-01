/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.token/gxc.token.hpp>

namespace gxc {

   void token_contract::account::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_vauth(issuer());

      modify(ram_payer, [&](auto& a) {
         for (auto o : opts) {
            if (o.first == "frozen") {
               check(_st->get_opt(token::opt::freezable), "not configured to freeze account");
               auto value = unpack<bool>(o.second);
               check(a.get_opt(opt::frozen) != value, "option already has give value");
               a.set_opt(opt::frozen, value);
            } else if (o.first == "whitelist") {
               check(_st->get_opt(token::opt::whitelistable), "not configured to whitelist account");
               auto value = unpack<bool>(o.second);
               check(a.get_opt(opt::whitelist) != value, "option already has give value");
               a.set_opt(opt::whitelist, value);
            } else {
               check(false, "unknown option `" + o.first + "`");
            }
         }
      });
   }

   void token_contract::account::sub_balance(extended_asset value) {
      check_account_is_valid();
      check(_this->balance.amount >= value.quantity.amount, "overdrawn balance");

      if (!_this->get_opt(opt::whitelist) && !keep_balance &&
          _this->balance.amount == value.quantity.amount &&
          _this->get_deposit().amount == 0)
      {
         erase();
      } else {
         modify(ram_payer, [&](auto& a) {
            a.balance -= value.quantity;
         });
      }
   }

   void token_contract::account::add_balance(extended_asset value) {
      if (!exists()) {
         check(!_st->get_opt(token::opt::whitelist_on) || has_vauth(value.contract), "required to open balance manually");
         emplace(ram_payer, [&](auto& a) {
            a.balance = value.quantity;
            a.set_deposit(asset(0, value.quantity.symbol));
            a.set_issuer(value.contract);
         });
      } else {
         check_account_is_valid();
         modify(ram_payer, [&](auto& a) {
            a.balance += value.quantity;
         });
      }
   }

   void token_contract::account::sub_deposit(extended_asset value) {
      check_account_is_valid();
      check(_this->get_deposit().amount >= value.quantity.amount, "overdrawn deposit");

      if (!_this->get_opt(opt::whitelist) && !keep_balance &&
          _this->get_deposit().amount == value.quantity.amount &&
          _this->balance.amount == 0)
      {
         erase();
      } else {
         modify(ram_payer, [&](auto& a) {
            a.set_deposit(a.get_deposit() - value.quantity);
         });
      }
   }

   void token_contract::account::add_deposit(extended_asset value) {
      if (!exists()) {
         check(!_st->get_opt(token::opt::whitelist_on) || has_vauth(value.contract), "required to open deposit manually");
         emplace(ram_payer, [&](auto& a) {
            a.balance = asset(0, value.quantity.symbol);
            a.set_deposit(value.quantity);
            a.set_issuer(value.contract);
         });
      } else {
         check_account_is_valid();
         modify(ram_payer, [&](auto& a) {
            a.set_deposit(a.get_deposit() + value.quantity);
         });
      }
   }

   void token_contract::account::open() {
      if (!exists()) {
         emplace(ram_payer, [&](auto& a) {
            a.balance.symbol = _st->supply.symbol;
            a.set_issuer(_st->issuer);
         });
      }
   }

   void token_contract::account::close() {
      require_auth(owner());
      check(exists(), "account balance doesn't exist");
      check(!_this->balance.amount && !_this->get_deposit().amount, "cannot close non-zero balance");
      erase();
   }

   void token_contract::account::approve(name spender, extended_asset value) {
      check_asset_is_valid(value, true);
      require_auth(owner());

      allowed _allowed(code(), owner().value);

      auto it = _allowed.find(allowance::get_id(spender, value));
      if (it == _allowed.end()) {
         // no existing allowance, but try approving `0` amount (erase allowance)
         check(value.quantity.amount > 0, "allowance not found");

         _allowed.emplace(owner(), [&](auto& a) {
            a.spender  = spender;
            a.quantity = value.quantity;
            a.issuer   = value.contract;
         });
      } else if (value.quantity.amount > 0) {
         _allowed.modify(it, owner(), [&](auto& a) {
            a.quantity = value.quantity;
         });
      } else {
         _allowed.erase(it);
      }
   }

   void token_contract::account::sub_allowance(name spender, extended_asset value) {
      allowed _allowed(code(), owner().value);

      const auto& it = _allowed.get(allowance::get_id(spender, value));

      if (it.quantity > value.quantity)
         _allowed.modify(it, owner(), [&](auto& a) {
            a.quantity -= value.quantity;
         });
      else if (it.quantity == value.quantity)
         _allowed.erase(it);
      else
         check(false, "try transfering more than allowed");
   }
}
