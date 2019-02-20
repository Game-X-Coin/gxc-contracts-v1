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
         t.set_max_supply(max_supply.quantity);
         t.issuer        = max_supply.contract;

         for (auto o : opts) {
            if (o.key == "can_recall") {
               t.set_opt(opt::can_recall, static_cast<bool>(o.value[0]));
            } else if (o.key == "can_freeze") {
               t.set_opt(opt::can_freeze, static_cast<bool>(o.value[0]));
            } else if (o.key == "can_whitelist") {
               t.set_opt(opt::can_whitelist, static_cast<bool>(o.value[0]));
            } else if (o.key == "is_frozen") {
               t.set_opt(opt::is_frozen, static_cast<bool>(o.value[0]));
            } else if (o.key == "enforce_whitelist") {
               t.set_opt(opt::enforce_whitelist, static_cast<bool>(o.value[0]));
            } else if (o.key == "withdraw_min_amount") {
               check(o.value.size() == sizeof(asset), "invalid serialization");
               t.set_withdraw_min_amount(*reinterpret_cast<asset*>(o.value.data()));
            } else if (o.key == "withdraw_delay_sec") {
               check(o.value.size() == sizeof(uint32_t), "invalid serialization");
               t.withdraw_delay_sec = *reinterpret_cast<uint32_t*>(o.value.data());
            } else {
               check(false, "unknown option `" + o.key + "`");
            }
         }

         check(!t.get_opt(opt::is_frozen) || t.get_opt(opt::can_freeze), "not allowed to set frozen ");
         check(!t.get_opt(opt::enforce_whitelist) || t.get_opt(opt::can_whitelist), "not allowed to set whitelist");
      });
   }

   void token_contract::token::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_auth(issuer());

      _tbl.modify(_this, same_payer, [&](auto& s) {
         for (auto o : opts) {
            if (o.key == "is_frozen") {
               check(s.get_opt(opt::can_freeze), "not allowed to freeze token");
               auto value = static_cast<bool>(o.value[0]);
               check(s.get_opt(opt::is_frozen) != value, "option already has given value");
               s.set_opt(opt::is_frozen, value);
            } else if (o.key == "enforce_whitelist") {
               check(s.get_opt(opt::can_whitelist), "not allowed to apply whitelist");
               auto value = static_cast<bool>(o.value[0]);
               check(s.get_opt(opt::enforce_whitelist) != value, "option already has given value");
               s.set_opt(opt::enforce_whitelist, value);
            } else {
               check(false, "unknown option `" + o.key + "`");
            }
         }
      });
   }

   void token_contract::token::issue(name to, extended_asset value) {
      require_auth(value.contract);
      check_asset_is_valid(value);
      check(!_this->get_opt(opt::is_frozen), "token is frozen");

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");
      check(value.quantity.amount <= _this->get_max_supply().amount - _this->supply.amount, "quantity exceeds available supply");

      _tbl.modify(_this, same_payer, [&](auto& s) {
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
         check(_this->get_opt(opt::can_recall) && has_auth(value.contract), "Missing required authority");
         is_recall = true;
      }

      _tbl.modify(_this, same_payer, [&](auto& s) {
         s.supply -= value.quantity;
      });

      auto _to = get_account(owner);

      if (!is_recall)
         _to.sub_balance(value);
      else
         _to.sub_deposit(value);
   }

   void token_contract::token::burn(extended_asset value) {
      require_auth(value.contract);
      check_asset_is_valid(value);
      check(!_this->get_opt(opt::is_frozen), "token is frozen");

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");

      _tbl.modify(_this, same_payer, [&](auto& s) {
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

      if (!has_auth(from)) {
         check(_this->get_opt(opt::can_recall) && has_auth(value.contract), "Missing required authority");
         is_recall = true;
      }

      // subtract asset from `from`
      auto _from = get_account(from);

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
         if (has_auth(issuer())) {
            auto recallable = _req->value.quantity - value.quantity;
            check(recallable.amount >= 0, "cannot deposit more than withdrawal requested by issuer");

            if (recallable.amount > 0)
               _req.modify(same_payer, [&](auto& rq) {
                  rq.value -= value;
               });
            else {
               _req.erase();
               _req.refresh_schedule();
            }
            get_account(code()).sub_balance(value);
         } else {
            require_auth(owner);
            auto recallable = _req->value.quantity - _this->get_withdraw_min_amount();

            if (recallable > value.quantity) {
               _req.modify(same_payer, [&](auto& rq) {
                  rq.value -= value;
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
            rq.value += value;
         });
      } else {
         _req.emplace(owner, [&](auto& rq) {
            rq.scheduled_time = ctp + seconds(_this->withdraw_delay_sec);
            rq.value          = value;
         });
      }

      get_account(owner).sub_deposit(value, true);
      get_account(code()).add_balance(value);

      _req.refresh_schedule(ctp + seconds(_this->withdraw_delay_sec));
   }
}
