/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.token/gxc.token.hpp>
#include <eosio/transaction.hpp>

namespace gxc {

   constexpr name active_permission {"active"_n};

   void token_contract::requests::refresh_schedule(time_point_sec base_time) {
      auto _idx = get_index<"schedtime"_n>();
      auto _it = _idx.begin();

      if (_it != _idx.end()) {
         transaction out;
         out.actions.emplace_back(action{{owner(), active_permission}, code(), "clrwithdraws"_n, owner()});

         auto withdraw_delay_sec = token(code(), _it->issuer, _it->quantity.symbol)->withdraw_delay_sec;

         if (_it->scheduled_time == base_time) {
            out.delay_sec = withdraw_delay_sec;
         } else {
            auto timeleft = _it->scheduled_time - base_time;
            out.delay_sec = static_cast<uint32_t>(timeleft.to_seconds());
         }

         cancel_deferred(owner().value);
         out.send(owner().value, owner(), true);
      } else {
         cancel_deferred(owner().value);
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
         _token.get_account(owner()).add_balance(_it->value());

         event_withdraw(code(), {code(), "active"_n}).send(owner(), _it->value());

         _idx.erase(_it);
      }

      refresh_schedule();
   }
}
