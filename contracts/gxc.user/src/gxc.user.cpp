/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.user/gxc.user.hpp>

namespace gxc { namespace user {

void contract::login(name account_name, name game_name, string login_token) {
   eosio_assert(login_token.size() == 16, "login_token has invalid length");
   require_auth(account_name);

   char* end;
   auto expire_time = strtoul(login_token.substr(8).data(), &end, 16);
   eosio_assert(now() <= expire_time, "login_token is expired");
}

void contract::setnick(name account_name, string nickname) {
   eosio_assert(nickname.size() <= 24, "nickname too long");
   eosio_assert(is_valid_nickname(nickname), "invalid nickname");

   require_auth(account_name);

   nicktable nt(_self, _self.value);

   const auto& idx = nt.get_index<"nickname"_n>();
   auto dup = idx.find(sha256(nickname.data(), nickname.size()));
   eosio_assert(dup == idx.end(), "nickname already occupied");

   auto cb = [&](auto& n) {
      n.account_name = account_name;
      n.nickname = nickname;
   };

   auto itr = nt.find(account_name.value);
   if (itr != nt.end()) {
      eosio_assert(false, "nickname change not supported");
      //nt.modify(itr, same_payer, cb);
   } else {
      nt.emplace(_self, cb);
   }
}

void contract::rmvnick(name account_name) {
   eosio_assert(false, "nickname removal not supported");

   // control flow should not reach here
   require_auth(_self);

   nicktable nt(_self, _self.value);

   auto itr = nt.find(account_name.value);
   eosio_assert(itr != nt.end(), "nickname not registered");

   nt.erase(itr);
}

} }

EOSIO_DISPATCH(gxc::user::contract, (login)(setnick))
