/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.token/gxc.token.hpp>

namespace gxc {

   void token_contract::account::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_auth(issuer());

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
         _tbl.emplace(code(), [&](auto& a) {
            a.id      = _tbl.available_primary_key();
            a.balance = value.quantity;
            a.deposit = asset(0, value.quantity.symbol);
            a.issuer  = value.contract;
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
         _tbl.emplace(code(), [&](auto& a) {
            a.id      = _tbl.available_primary_key();
            a.balance = asset(0, value.quantity.symbol);
            a.deposit = value.quantity;
            a.issuer  = value.contract;
         });
      } else {
         check_account_is_valid();
         _tbl.modify(_this, same_payer, [&](auto& a) {
            a.deposit += value.quantity;
         });
      }
   }

   void token_contract::account::open() {
      require_auth(issuer());
      check(!exists(), "account balance already exists");
      _tbl.emplace(code(), [&](auto& a) {
         a.id             = _tbl.available_primary_key();
         a.balance.symbol = (*_st)->supply.symbol;
         a.deposit.symbol = (*_st)->supply.symbol;
         a.issuer         = (*_st)->issuer;
      });
   }

   void token_contract::account::close() {
      require_auth(owner());
      check(exists(), "account balance doesn't exist");
      check(!_this->balance.amount && !_this->deposit.amount, "cannot close non-zero balance");
      _tbl.erase(_this);
   }

   void token_contract::account::deposit(extended_asset value) {
      check_asset_is_valid(value);

      auto _token = token(code(), value.contract, value.quantity.symbol.code().raw());
      check(_token->can_recall, "not supported token");

      auto _req = requests(code(), owner(),
                           extended_symbol_code(value.quantity.symbol, value.contract).raw());

      if (!_req.exists()) {
         require_auth(owner());
         sub_balance(value);
      } else {
         auto leftover = _req->quantity - value.quantity;
         check((leftover.amount >= 0 && has_auth(issuer())) || has_auth(owner()), "missing required authority");
         if (leftover.amount > 0 ) {
            _req.modify(same_payer, [&](auto& rq) {
               rq.quantity -= value.quantity;
            });
            _token.get_account(code()).sub_balance(value);
         } else {
            _token.get_account(code()).sub_balance(extended_asset(_req->quantity, _req->issuer));
            if (leftover.amount)
               _token.get_account(owner()).sub_balance(extended_asset(-leftover, value.contract));
            _req.erase();
            _req.refresh_schedule();
         }
      }

      _token.get_account(owner()).add_deposit(value);
   }

   void token_contract::account::withdraw(extended_asset value) {
      check_asset_is_valid(value);
      require_auth(owner());

      auto _token = token(code(), value.contract, value.quantity.symbol.code().raw());
      check(_token->can_recall, "not supported token");
      check(value.quantity >= _token->withdraw_min_amount, "withdraw amount is too small");

      auto _req = requests(code(), owner(),
                           extended_symbol_code(value.quantity.symbol, value.contract).raw());
      auto ctp = current_time_point();

      if (_req.exists()) {
         _req.modify(same_payer, [&](auto& rq) {
            rq.requested_time = ctp;
            rq.quantity += value.quantity;
         });
      } else {
         _req.emplace(owner(), [&](auto& rq) {
            rq.id             = _req.table().available_primary_key();
            rq.requested_time = ctp;
            rq.quantity       = value.quantity;
            rq.issuer         = value.contract;
         });
      }

      _token.get_account(owner()).sub_deposit(value);
      _token.get_account(code()).add_balance(value);

      _req.refresh_schedule(ctp);
   }
}
