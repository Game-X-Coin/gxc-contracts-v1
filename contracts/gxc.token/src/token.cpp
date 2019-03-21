/**
 * @file
 * @copyright defined in gxc/LICENSE
 */

#include <gxc.token/gxc.token.hpp>

namespace gxc {

   void token_contract::token::mint(extended_asset value, const std::vector<key_value>& opts) {
      require_auth(code());

      //check(!exists(), "token with symbol already exists");
      check_asset_is_valid(value);

      //TODO: check core symbol
      //TODO: check game account

      bool init = !exists();

      if (init) {
         emplace(code(), [&](auto& t) {
            t.supply.symbol = value.quantity.symbol;
            t.set_max_supply(value.quantity);
            t.issuer        = value.contract;
         });
      } else {
         modify(same_payer, [&](auto& t) {
            t.set_max_supply(t.get_max_supply() + value.quantity);
         });
      }

      _setopts(opts, init);
   }

   void token_contract::token::_setopts(const std::vector<key_value>& opts, bool init) {
      modify(same_payer, [&](auto& t) {
         for (auto o : opts) {
            if (o.key == "can_recall") {
               check(init, "not allowed to change the option `" + o.key + "`");
               t.set_opt(opt::can_recall, unpack<bool>(o.value));
            } else if (o.key == "can_freeze") {
               check(init, "not allowed to change the option `" + o.key + "`");
               t.set_opt(opt::can_freeze, unpack<bool>(o.value));
            } else if (o.key == "can_whitelist") {
               check(init, "not allowed to change the option `" + o.key + "`");
               t.set_opt(opt::can_whitelist, unpack<bool>(o.value));
            } else if (o.key == "is_frozen") {
               t.set_opt(opt::is_frozen, unpack<bool>(o.value));
            } else if (o.key == "enforce_whitelist") {
               t.set_opt(opt::enforce_whitelist, unpack<bool>(o.value));
            } else if (o.key == "withdraw_min_amount") {
               check(init, "not allowed to change the option `" + o.key + "`");
               auto value = unpack<int64_t>(o.value);
               check(value >= 0, "withdraw_min_amount should be positive");
               t.set_withdraw_min_amount(asset(value, t.supply.symbol));
            } else if (o.key == "withdraw_delay_sec") {
               check(init, "not allowed to change the option `" + o.key + "`");
               auto value = unpack<uint64_t>(o.value);
               t.withdraw_delay_sec = static_cast<uint32_t>(value);
            } else {
               check(false, "unknown option `" + o.key + "`");
            }
         }
      });

      check(!_this->get_opt(opt::is_frozen) || _this->get_opt(opt::can_freeze), "not allowed to set frozen ");
      check(!_this->get_opt(opt::enforce_whitelist) || _this->get_opt(opt::can_whitelist), "not allowed to set whitelist");
   }

   void token_contract::token::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_vauth(issuer());

      _setopts(opts);
   }

   void token_contract::token::issue(name to, extended_asset value) {
      require_vauth(value.contract);
      check_asset_is_valid(value);
      check(!_this->get_opt(opt::is_frozen), "token is frozen");

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");
      check(value.quantity.amount <= _this->get_max_supply().amount - _this->supply.amount, "quantity exceeds available supply");

      modify(same_payer, [&](auto& s) {
         s.supply += value.quantity;
      });

      auto _to = get_account(to);

      if (_this->get_opt(opt::can_recall) && (to != value.contract))
         _to.add_deposit(value);
      else
         _to.add_balance(value);

   }

   void token_contract::token::retire(name owner, extended_asset value) {
      check_asset_is_valid(value);
      check(!_this->get_opt(opt::is_frozen), "token is frozen");

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");

      bool is_recall = false;

      if (!has_auth(owner)) {
         check(_this->get_opt(opt::can_recall) && has_vauth(value.contract), "Missing required authority");
         is_recall = true;
      }

      modify(same_payer, [&](auto& s) {
         s.supply -= value.quantity;
      });

      auto _to = get_account(owner);

      if (!is_recall)
         _to.sub_balance(value);
      else
         _to.sub_deposit(value);
   }

   void token_contract::token::burn(extended_asset value) {
      if (!has_vauth(value.contract)) {
         require_auth(code());
      }
      check_asset_is_valid(value);
      check(!_this->get_opt(opt::is_frozen), "token is frozen");

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");

      modify(same_payer, [&](auto& s) {
         s.supply -= value.quantity;
         s.set_max_supply(s.get_max_supply() - value.quantity);
      });

      get_account(value.contract).sub_balance(value);
   }

   void token_contract::token::transfer(name from, name to, extended_asset value) {
      check(from != to, "cannot transfer to self");
      check(is_account(to), "`to` account does not exist");

      check_asset_is_valid(value);
      check(!_this->get_opt(opt::is_frozen), "token is frozen");

      bool is_recall = false;
      bool is_allowed = false;

      if (!has_auth(from)) {
         if (_this->get_opt(opt::can_recall) && has_vauth(value.contract)) {
            is_recall = true;
         } else if (has_auth(to)) {
            allowed _allowed(code(), from.value);
            auto it = _allowed.find(allowance::get_id(to, value));
            is_allowed = (it != _allowed.end());
         }
         check(is_recall || is_allowed, "Missing required authority");
      }

      // subtract asset from `from`
      auto _from = get_account(from);

      if (is_allowed)
         _from.sub_allowance(to, value);

      if (!is_recall)
         _from.sub_balance(value);
      else
         _from.sub_deposit(value);

      // add asset to `to`
      get_account(to).add_balance(value);
   }

   void token_contract::token::deposit(name owner, extended_asset value) {
      check_asset_is_valid(value);
      check(_this->get_opt(opt::can_recall), "not supported token");

      auto _owner = get_account(owner);
      auto _req = requests(code(), owner, value);

      if (!_req.exists()) {
         require_auth(owner);
         _owner.sub_balance(value);
      } else {
         if (has_vauth(issuer())) {
            auto recallable = _req->quantity - value.quantity;
            check(recallable.amount >= 0, "cannot deposit more than withdrawal requested by issuer");

            if (recallable.amount > 0)
               _req.modify(same_payer, [&](auto& rq) {
                  rq.quantity -= value.quantity;
               });
            else {
               _req.erase();
               _req.refresh_schedule();
            }
            get_account(code()).sub_balance(value);
         } else {
            require_auth(owner);
            auto recallable = _req->quantity - _this->get_withdraw_min_amount();

            if (recallable > value.quantity) {
               _req.modify(same_payer, [&](auto& rq) {
                  rq.quantity -= value.quantity;
               });
               get_account(code()).sub_balance(value);
            } else {
               _req.erase();
               _req.refresh_schedule();
               get_account(code()).sub_balance(extended_asset(recallable, value.contract));

               if (recallable < value.quantity)
                  _owner.sub_balance(extended_asset(value.quantity - recallable, value.contract));
            }
         }
      }
      _owner.add_deposit(value);
   }

   void token_contract::token::withdraw(name owner, extended_asset value) {
      check_asset_is_valid(value);
      require_auth(owner);

      check(_this->get_opt(opt::can_recall), "not supported token");
      check(value.quantity >= _this->get_withdraw_min_amount(), "withdraw amount is too small");

      auto _req = requests(code(), owner, value);
      auto ctp = current_time_point();

      if (_req.exists()) {
         _req.modify(same_payer, [&](auto& rq) {
            rq.scheduled_time = ctp + seconds(_this->withdraw_delay_sec);
            rq.quantity += value.quantity;
         });
      } else {
         _req.emplace(owner, [&](auto& rq) {
            rq.scheduled_time = ctp + seconds(_this->withdraw_delay_sec);
            rq.quantity       = value.quantity;
            rq.issuer         = value.contract;
         });
      }

      get_account(owner).keep().sub_deposit(value);
      get_account(code()).add_balance(value);

      _req.refresh_schedule(ctp + seconds(_this->withdraw_delay_sec));
   }
}
