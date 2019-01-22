/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.game/gxc.game.hpp>

namespace gxc { namespace game {

void contract::setgame(name account_name, bool is_game) {
   require_auth(game::account);

   gametable gt(_self, _self.value);
   auto itr = gt.find(account_name.value);

   if (is_game) {
      eosio_assert(itr == gt.end(), "already registered game");
      gt.emplace(_self, [&](auto& gm) {
         gm.account_name = account_name;
      });
   } else {
      eosio_assert(itr != gt.end(), "not registered game");
      gt.erase(itr);
   }
}

} }

EOSIO_DISPATCH(gxc::game::contract, (setgame))
