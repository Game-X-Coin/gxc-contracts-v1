/**
 * @file
 * @copyright defined in gxc/LICENSE
 */

#include <gxc.token/gxc.token.hpp>

namespace gxc {

   void token_contract::token::create(extended_asset max_supply, const std::vector<key_value>& opts) {
      require_auth(code());

      check(!exists(), "token with symbol already exists");
      check_asset_is_valid(max_supply);

      //TODO: check core symbol
      //TODO: check game account

      _tbl.emplace(code(), [&](auto& t) {
         t.supply.symbol = max_supply.quantity.symbol;
         t.max_supply    = max_supply.quantity;
         t.issuer        = max_supply.contract;
         t.withdraw_min_amount.symbol = max_supply.quantity.symbol;

         for (auto o : opts) {
            if (o.key == "can_freeze") {
               t.can_freeze = static_cast<bool>(o.value[0]);
            } else if (o.key == "can_recall") {
               t.can_recall = static_cast<bool>(o.value[0]);
            } else if (o.key == "can_whitelist") {
               t.can_whitelist = static_cast<bool>(o.value[0]);
            } else if (o.key == "is_frozen") {
               t.is_frozen = static_cast<bool>(o.value[0]);
            } else if (o.key == "enforce_whitelist") {
               t.enforce_whitelist = static_cast<bool>(o.value[0]);
            } else if (o.key == "withdraw_min_amount") {
               check(o.value.size() == sizeof(asset), "invalid serialization");
               t.withdraw_min_amount = *reinterpret_cast<asset*>(o.value.data());
            } else if (o.key == "withdraw_delay_sec") {
               check(o.value.size() == sizeof(uint32_t), "invalid serialization");
               t.withdraw_delay_sec = *reinterpret_cast<uint32_t*>(o.value.data());
            } else {
               check(false, "unknown option `" + o.key + "`");
            }
         }

         check(!t.is_frozen || t.can_freeze, "not allowed to set frozen ");
         check(!t.enforce_whitelist || t.can_whitelist, "not allowed to set whitelist");
      });
   }

   void token_contract::token::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_auth(issuer());

      _tbl.modify(_this, same_payer, [&](auto& s) {
         for (auto o : opts) {
            if (o.key == "is_frozen") {
               check(_this->can_freeze, "not allowed to freeze token");
               auto value = static_cast<bool>(o.value[0]);
               check(s.is_frozen != value, "option already has given value");
               s.is_frozen = value;
            } else if (o.key == "enforce_whitelist") {
               check(_this->can_whitelist, "not allowed to apply whitelist");
               auto value = static_cast<bool>(o.value[0]);
               check(s.enforce_whitelist != value, "option already has given value");
               s.enforce_whitelist = value;
            } else {
               check(false, "unknown option `" + o.key + "`");
            }
         }
      });
   }

   void token_contract::token::issue(name to, extended_asset quantity) {
      require_auth(quantity.contract);
      check_asset_is_valid(quantity);
      check(!_this->is_frozen, "token is frozen");

      //TODO: check game account
      check(quantity.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");
      check(quantity.quantity.amount <= _this->max_supply.amount - _this->supply.amount, "quantity exceeds available supply");

      _tbl.modify(_this, same_payer, [&](auto& s) {
         s.supply += quantity.quantity;
      });

      auto _to = get_account(to);

      if (_this->can_recall && (to != quantity.contract))
         _to.add_deposit(quantity);
      else
         _to.add_balance(quantity);

   }

   void token_contract::token::retire(name owner, extended_asset quantity) {
      require_auth(owner);
      check_asset_is_valid(quantity);
      check(!_this->is_frozen, "token is frozen");

      //TODO: check game account
      check(quantity.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");

      _tbl.modify(_this, same_payer, [&](auto& s) {
         s.supply -= quantity.quantity;
      });

      auto _to = get_account(owner);

      if (_this->can_recall && (owner != quantity.contract))
         _to.sub_deposit(quantity);
      else
         _to.sub_balance(quantity);
   }

   void token_contract::token::burn(extended_asset quantity) {
      require_auth(quantity.contract);
      check_asset_is_valid(quantity);
      check(!_this->is_frozen, "token is frozen");

      //TODO: check game account
      check(quantity.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");

      _tbl.modify(_this, same_payer, [&](auto& s) {
         s.supply -= quantity.quantity;
         s.max_supply -= quantity.quantity;
      });

      get_account(quantity.contract).sub_balance(quantity);
   }

   void token_contract::token::transfer(name from, name to, extended_asset quantity) {
      check(from != to, "cannot transfer to self");
      check(is_account(to), "`to` account does not exist");

      check_asset_is_valid(quantity);
      check(!_this->is_frozen, "token is frozen");

      bool is_recall = false;

      if (!has_auth(from)) {
         check(_this->can_recall && has_auth(quantity.contract), "Missing required authority");
         is_recall = true;
      }

      // subtract asset from `from`
      auto _from = get_account(from);

      if (!is_recall)
         _from.sub_balance(quantity);
      else
         _from.sub_deposit(quantity);

      // add asset to `to`
      get_account(to).add_balance(quantity);
   }

   void token_contract::token::deposit(name owner, extended_asset value) {
      check_asset_is_valid(value);
      check(_this->can_recall, "not supported token");

      auto _req = requests(code(), owner,
                           extended_symbol_code(value.quantity.symbol, value.contract).raw());
      if (!_req.exists()) {
         require_auth(owner);
         get_account(owner).sub_balance(value);
      } else {
         auto leftover = _req->quantity - value.quantity;
         check((leftover.amount >= 0 && has_auth(issuer())) || has_auth(owner), "missing required authority");
         if (leftover.amount > 0 ) {
            _req.modify(same_payer, [&](auto& rq) {
               rq.quantity -= value.quantity;
            });
            get_account(code()).sub_balance(value);
         } else {
            get_account(code()).sub_balance(extended_asset(_req->quantity, _req->issuer));
            if (leftover.amount)
               get_account(owner).sub_balance(extended_asset(-leftover, value.contract));
            _req.erase();
            _req.refresh_schedule();
         }
      }
      get_account(owner).add_deposit(value);
   }

   void token_contract::token::withdraw(name owner, extended_asset value) {
      check_asset_is_valid(value);
      require_auth(owner);

      check(_this->can_recall, "not supported token");
      check(value.quantity >= _this->withdraw_min_amount, "withdraw amount is too small");

      auto _req = requests(code(), owner,
                           extended_symbol_code(value.quantity.symbol, value.contract).raw());
      auto ctp = current_time_point();

      if (_req.exists()) {
         _req.modify(same_payer, [&](auto& rq) {
            rq.requested_time = ctp;
            rq.quantity += value.quantity;
         });
      } else {
         _req.emplace(owner, [&](auto& rq) {
            rq.id             = _req.table().available_primary_key();
            rq.requested_time = ctp;
            rq.quantity       = value.quantity;
            rq.issuer         = value.contract;
         });
      }

      get_account(owner).sub_deposit(value);
      get_account(code()).add_balance(value);

      _req.refresh_schedule(ctp);
   }
}
