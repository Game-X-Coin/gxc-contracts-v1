/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.token/gxc.token.hpp>
#include <eosiolib/transaction.hpp>

namespace gxc {

   constexpr name active_permission {"active"_n};

   void token_contract::requests::refresh_schedule(time_point_sec base_time) {
      auto _idx = _tbl.get_index<"reqtime"_n>();
      auto _it = _idx.begin();

      if (_it != _idx.end()) {
         transaction out;
         out.actions.emplace_back(action{{self(), active_permission}, self(), "clearreqs"_n, owner()});

         auto withdraw_delay_sec = token(self(), _it->issuer,
                                         _it->quantity.symbol.code().raw())->withdraw_delay_sec;

         if (_it->requested_time == base_time) {
            out.delay_sec = withdraw_delay_sec;
         } else {
            auto timeleft = seconds(withdraw_delay_sec) - (base_time - _it->requested_time);
            out.delay_sec = static_cast<uint32_t>(timeleft.to_seconds());
         }

         cancel_deferred(owner().value);
         out.send(owner().value, owner(), true);
      } else {
         cancel_deferred(owner().value);
      }
   }

   void token_contract::requests::clear() {
      require_auth(self());

      auto _idx = _tbl.get_index<"reqtime"_n>();
      auto _it = _idx.begin();

      check(_it != _idx.end(), "withdrawal requests not found");

      for ( ; _it != _idx.end(); _it = _idx.begin()) {
         auto _token = token(self(), _it->issuer, _it->quantity.symbol.code().raw());
         if (_it->requested_time + seconds(_token->withdraw_delay_sec) > current_time_point()) break;

         _token.get_account(self()).sub_balance(extended_asset(_it->quantity, _it->issuer));
         _token.get_account(owner()).sub_balance(extended_asset(_it->quantity, _it->issuer));

         _idx.erase(_it);
      }

      refresh_schedule();
   }
}
