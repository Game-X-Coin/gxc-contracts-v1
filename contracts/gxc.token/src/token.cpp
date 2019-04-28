/**
 * @file
 * @copyright defined in gxc/LICENSE
 */

#include <gxc.token/gxc.token.hpp>
#include <gxclib/game.hpp>

namespace gxc {

   void token_contract::token::mint(extended_asset value, const std::vector<key_value>& opts) {
      require_auth(code());
      check_asset_is_valid(value);

      //TODO: check core symbol
      //TODO: check game account

      bool init = !exists();

      if (init) {
         emplace(code(), [&](auto& t) {
            t.supply.symbol = value.quantity.symbol;
            t.max_supply(value.quantity);
            t.issuer        = value.contract;
         });
      } else {
         check(_this->option(opt::mintable), "not allowed additional mint");
         modify(same_payer, [&](auto& t) {
            t.max_supply(t.max_supply() + value.quantity);
         });
      }

      _setopts(opts, init);
   }

   void token_contract::token::_setopts(const std::vector<key_value>& opts, bool init) {
      modify(same_payer, [&](auto& t) {
         for (auto o : opts) {
            if (o.first == "paused")
               t.option(opt::paused, unpack<bool>(o.second));
            else if (o.first == "whitelist_on")
               t.option(opt::whitelist_on, unpack<bool>(o.second));
            else {
               // Below options can be configured only when creating token.
               check(init, "not allowed to change the option `" + o.first + "`");

               if (o.first == "mintable") {
                  t.option(opt::mintable, unpack<bool>(o.second));
               } else if (o.first == "recallable") {
                  t.option(opt::recallable, unpack<bool>(o.second));
               } else if (o.first == "freezable") {
                  t.option(opt::freezable, unpack<bool>(o.second));
               } else if (o.first == "pausable") {
                  t.option(opt::pausable, unpack<bool>(o.second));
               } else if (o.first == "whitelistable") {
                  t.option(opt::whitelistable, unpack<bool>(o.second));
               } else if (o.first == "withdraw_min_amount") {
                  auto value = unpack<int64_t>(o.second);
                  check(value >= 0, "withdraw_min_amount should be positive");
                  t.withdraw_min_amount(asset(value, t.supply.symbol));
               } else if (o.first == "withdraw_delay_sec") {
                  auto value = unpack<uint64_t>(o.second);
                  t.withdraw_delay_sec = static_cast<uint32_t>(value);
               } else {
                  check(false, "unknown option `" + o.first + "`");
               }
            }
         }
      });

      check(!_this->option(opt::paused) || (init || _this->option(opt::pausable)), "not allowed to set paused");
      check(!_this->option(opt::whitelist_on) || _this->option(opt::whitelistable), "not allowed to set whitelist");
   }

   void token_contract::token::setopts(const std::vector<key_value>& opts) {
      check(opts.size(), "no changes on options");
      require_vauth(issuer());

      _setopts(opts);
   }

   void token_contract::token::issue(name to, extended_asset value) {
      require_vauth(value.contract);
      check_asset_is_valid(value);

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");
      check(value.quantity.amount <= _this->max_supply().amount - _this->supply.amount, "quantity exceeds available supply");

      modify(same_payer, [&](auto& s) {
         s.supply += value.quantity;
      });

      name payer = (value.contract == "gxc"_n || has_gauth(value.contract)) ? code() : value.contract;

      auto _to = get_account(to);

      if (_this->option(opt::recallable) && (to != value.contract))
         _to.paid_by(code()).add_deposit(value);
      else
         _to.paid_by(payer).add_balance(value);

   }

   void token_contract::token::retire(name owner, extended_asset value) {
      check_asset_is_valid(value);

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");

      bool is_recall = false;

      if (!has_auth(owner)) {
         check(_this->option(opt::recallable) && has_vauth(value.contract), "Missing required authority");
         is_recall = true;
      }

      modify(same_payer, [&](auto& s) {
         s.supply -= value.quantity;
      });

      auto _to = get_account(owner);

      if (!is_recall)
         _to.sub_balance(value);
      else
         _to.paid_by(code()).sub_deposit(value);
   }

   void token_contract::token::burn(extended_asset value) {
      if (!has_vauth(value.contract)) {
         require_auth(code());
      }
      check_asset_is_valid(value);

      //TODO: check game account
      check(value.quantity.symbol == _this->supply.symbol, "symbol precision mismatch");

      modify(same_payer, [&](auto& s) {
         s.supply -= value.quantity;
         s.max_supply(s.max_supply() - value.quantity);
      });

      get_account(value.contract).sub_balance(value);
   }

   void token_contract::token::transfer(name from, name to, extended_asset value) {
      check(from != to, "cannot transfer to self");
      check(is_account(to), "`to` account does not exist");

      check_asset_is_valid(value);
      check(!_this->option(opt::paused), "token is paused");

      bool is_recall = false;
      bool is_allowed = false;

      if (!has_auth(from)) {
         if (_this->option(opt::recallable) && has_vauth(value.contract)) {
            is_recall = true;
         } else if (has_auth(to)) {
            allowed _allowed(code(), from.value);
            auto it = _allowed.find(allowance::get_approval_id(to, value));
            is_allowed = (it != _allowed.end());
         }
         check(is_recall || is_allowed, "Missing required authority");
      }

      // subtract asset from `from`
      auto _from = get_account(from);

      if (is_allowed)
         _from.sub_allowance(to, value);

      if (!is_recall) {
         _from.sub_balance(value);
      } else {
         // normal case, transfer from's deposit
         if (_from->deposit() >= value.quantity) {
            _from.paid_by(code()).sub_deposit(value);
         } else {
            // exceptional case, cached amount is not enough
            // so withdrawal request is partially cancelled
            auto leftover = value.quantity - _from->deposit();
            auto _req = requests(code(), from, value);
            check(_req, "overdrawn deposit, but no withdrawal request");
            check(_req->quantity >= leftover, "overdrawn deposit, but not enough withdrawal requested amount");

            if (_req->quantity > leftover) {
               _req.modify(same_payer, [&](auto& rq) {
                  rq.quantity -= leftover;
               });
            } else {
               _req.erase();
               _req.refresh_schedule();
            }
            get_account(code()).sub_balance(extended_asset(leftover, value.contract));
            _from.paid_by(code()).sub_deposit(extended_asset(_from->deposit(), value.contract));

            withdraw_reverted(code(), {code(), active_permission}).send(from, extended_asset(leftover, value.contract));
         }
      }

      name payer;

      // case is_recall   : in-game transfer
      // case has_auth(to): approved transfer
      // case else        : all other cases
      if (is_recall) payer = code();
      else if (has_auth(to)) payer = to;
      else payer = from;

      // add asset to `to`
      get_account(to).paid_by(payer).add_balance(value);
   }

   void token_contract::token::deposit(name owner, extended_asset value) {
      check_asset_is_valid(value);
      check(_this->option(opt::recallable), "not supported token");

      auto _owner = get_account(owner);

      _owner.sub_balance(value);
      _owner.paid_by(owner).add_deposit(value);
   }

   void token_contract::token::withdraw(name owner, extended_asset value) {
      check_asset_is_valid(value);
      require_auth(owner);

      check(_this->option(opt::recallable), "not supported token");
      check(value.quantity >= _this->withdraw_min_amount(), "withdraw amount is too small");

      auto _req = requests(code(), owner, value);

      if (_req) {
         _req.modify(same_payer, [&](auto& rq) {
            rq.scheduled_time = current_time_point() + seconds(_this->withdraw_delay_sec);
            rq.quantity += value.quantity;
         });
      } else {
         _req.emplace(owner, [&](auto& rq) {
            rq.scheduled_time = current_time_point() + seconds(_this->withdraw_delay_sec);
            rq.quantity       = value.quantity;
            rq.issuer         = value.contract;
         });
      }

      get_account(owner).keep().sub_deposit(value);
      get_account(code()).paid_by(code()).add_balance(value);

      _req.refresh_schedule();
   }

   void token_contract::token::cancel_withdraw(name owner, name issuer, symbol_code sym) {
      require_auth(owner);

      auto _req = requests(code(), owner, extended_asset(asset(0, symbol(sym, 0)), issuer));
      check(_req, "withdrawal request not found");

      auto value = extended_asset(_req->quantity, _req->issuer);
      get_account(code()).sub_balance(value);
      get_account(owner).paid_by(owner).add_deposit(value);

      withdraw_reverted(code(), {code(), active_permission}).send(owner, value);

      _req.erase();
   }
}
