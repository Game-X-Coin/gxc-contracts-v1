/**
 * @file
 * @copyright defined in gxc/LICENSE
 */

#include <gxc.token/gxc.token.hpp>

namespace gxc {

   void token_contract::account::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_auth(_st->get_scope());

      _tbl.modify(_this, same_payer, [&](auto& a) {
         for (auto o : opts) {
            if (o.key == "frozen") {
               check((*_st)->can_freeze, "not configured to freeze account");
               a.frozen = static_cast<bool>(o.value[0]);
            } else if (o.key == "whitelist") {
               check((*_st)->can_whitelist, "not configured to whitelist account");
               a.whitelist = static_cast<bool>(o.value[0]);
            } else {
               check(false, "unknown option `" + o.key + "`");
            }
         }
      });
   }

   void token_contract::account::sub_balance(extended_asset value) {
      check_account_is_valid();
      check(_this->balance.amount >= value.quantity.amount, "overdrawn balance");

      if (!_this->whitelist &&
          _this->balance.amount == value.quantity.amount &&
          _this->deposit.amount == 0)
      {
         _tbl.erase(_this);
      } else {
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.balance -= value.quantity;
         });
      }
   }

   void token_contract::account::add_balance(extended_asset value) {
      if (!exists()) {
         check(!(*_st)->enforce_whitelist || has_auth(value.contract), "required to open balance manually");
         _tbl.emplace(get_self(), [&](auto& a) {
            a.id = _tbl.available_primary_key();
            a.balance = value.quantity;
            a.deposit.symbol = value.quantity.symbol;
            a.issuer = value.contract;
         });
      } else {
         check_account_is_valid();
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.balance += value.quantity;
         });
      }
   }

   void token_contract::account::sub_deposit(extended_asset value) {
      check_account_is_valid();
      check(_this->deposit.amount >= value.quantity.amount, "overdrawn deposit");

      if (!_this->whitelist &&
          _this->deposit.amount == value.quantity.amount &&
          _this->balance.amount == 0)
      {
         _tbl.erase(_this);
      } else {
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.deposit -= value.quantity;
         });
      }
   }

   void token_contract::account::add_deposit(extended_asset value) {
      if (!exists()) {
         check(!(*_st)->enforce_whitelist || has_auth(value.contract), "required to open deposit manually");
         _tbl.emplace(get_self(), [&](auto& a) {
            a.id = _tbl.available_primary_key();
            a.balance.symbol = value.quantity.symbol;
            a.deposit = value.quantity;
            a.issuer = value.contract;
         });
      } else {
         check_account_is_valid();
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.deposit += value.quantity;
         });
      }
   }
}
