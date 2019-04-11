/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.token/gxc.token.hpp>
#include <eosio/transaction.hpp>

namespace gxc {

   void token_contract::requests::refresh_schedule() {
      auto _idx = get_index<"schedtime"_n>();
      auto _it = _idx.begin();

      cancel_deferred(owner().value);

      if (_it != _idx.end()) {
         auto timeleft = (_it->scheduled_time - current_time_point()).to_seconds();
         if (timeleft <= 0) {
            clear_withdraws(code(), {owner(), active_permission}).send(owner());
         } else {
            transaction out;
            out.actions.emplace_back(action{{owner(), active_permission}, code(), "clrwithdraws"_n, owner()});
            out.delay_sec = static_cast<uint32_t>(timeleft);
            out.send(owner().value, owner(), true);
         }
      }
   }

   void token_contract::requests::clear() {
      require_auth(owner());

      auto _idx = get_index<"schedtime"_n>();
      auto _it = _idx.begin();

      check(_it != _idx.end(), "withdrawal requests not found");

      for ( ; _it != _idx.end(); _it = _idx.begin()) {
         auto _token = token(code(), _it->issuer, _it->quantity.symbol);
         if (_it->scheduled_time > current_time_point()) break;

         _token.get_account(code()).sub_balance(_it->value());
         _token.get_account(owner()).paid_by(owner()).add_balance(_it->value());

         withdraw_processed(code(), {code(), active_permission}).send(owner(), _it->value());

         _idx.erase(_it);
      }

      refresh_schedule();
   }
}
