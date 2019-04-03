#include <eosio.bios/eosio.bios.hpp>
#include <gxclib/system.hpp>

namespace eosio {

using namespace gxc::system;

void bios::init () {
   require_auth(_self);

   authority owner,active;
   owner.add_account(_self);
   active.add_account(_self);

   if (!is_account(user_account)) {
      action_newaccount(_self, {_self, active_permission}).send(_self, user_account, owner, active);
   }

   if (!is_account(game_account)) {
      action_newaccount(_self, {_self, active_permission}).send(_self, game_account, owner, active);
   }

   if (!is_account(reserve_account)) {
      active.add_account(reserve_account,code_permission);
      action_newaccount(_self, {_self, active_permission}).send(_self, reserve_account, owner, active);

      if (!is_account(token_account)) {
         active.add_account(token_account,code_permission);
         action_newaccount(_self, {_self, active_permission}).send(_self, token_account, owner, active);
      }
   }
}

void bios::setprivs() {
   setpriv(token_account,true);
}

}
