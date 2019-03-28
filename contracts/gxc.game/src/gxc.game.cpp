/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.game/gxc.game.hpp>

namespace gxc {

void game_contract::setgame(name name, bool activated) {
   require_auth(_self);

   games gms(_self, _self.value);
   auto it = gms.find(name.value);

   if (activated) {
      check(it == gms.end(), "already registered game");
      gms.emplace(_self, [&](auto& g) {
         g.name = name;
      });
   } else {
      check(it != gms.end(), "not registered game");
      gms.erase(it);
   }
}

void game_contract::seturi(name name, std::string uri) {
   require_auth(name);

   games gms(_self, _self.value);
   auto it = gms.find(name.value);

   check(it != gms.end(), "game account is not found");
   gms.modify(it, same_payer, [&](auto& g) {
      g.uri = uri;
   });
}

}
