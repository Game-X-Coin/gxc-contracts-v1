/**
 *  @file
 *  @copyright defined in gxc/LICENSE
 */

#include <gxc.token/gxc.token.hpp>
#include <gxclib/grc2.hpp>

namespace gxc { namespace token {

void contract::create(name issuer, asset maximum_supply, binary_extension<name> type) {
   eosio_assert(has_admin_auth("create"_n) || has_auth(_self), "missing required authority");

   tokentable tt(_self, issuer.value);
   auto dup = tt.find(maximum_supply.symbol.code().raw());
   eosio_assert(dup == tt.end(), "registered token");

   auto ttype = type.value_or(static_cast<name>(system_token));

   tt.emplace(_self, [&](auto& t) {
      t.type = ttype;
      t.sym = maximum_supply.symbol;
   });

   action( {{_self, system::active_permission}},
      ttype, "create"_n, std::make_tuple(issuer, maximum_supply)
   ).send();
}

void contract::issue(name to, asset quantity, string memo, binary_extension<name> issuer) {
   auto _issuer = issuer.value_or(static_cast<name>(system::account));
   eosio_assert(has_admin_auth("issue"_n) || has_auth(_issuer), "missing required authority");

   auto ttype = get_token_type(quantity.symbol.code(), _issuer);
   eosio_assert(ttype != name(), "not registered token");

   action( {{_self, system::active_permission}, {_issuer, system::active_permission}},
      ttype, "issue"_n, std::make_tuple(to, quantity, memo, _issuer)
   ).send();
}

void contract::retire(asset quantity, string memo, binary_extension<name> issuer) {
   auto _issuer = issuer.value_or(static_cast<name>(system::account));
   eosio_assert(has_admin_auth("retire"_n) || has_auth(_issuer), "missing required authority");

   auto ttype = get_token_type(quantity.symbol.code(), _issuer);
   eosio_assert(ttype != name(), "not registered token");

   action( {{_self, system::active_permission}, {_issuer, system::active_permission}},
      ttype, "retire"_n, std::make_tuple(quantity, memo, _issuer)
   ).send();
}

void contract::transfer(name from, name to, asset quantity, string memo, binary_extension<name> issuer) {
   auto _issuer = issuer.value_or(static_cast<name>(system::account));

   auto auth = from;
   if (!has_admin_auth("transfer"_n) && !has_auth(auth)) {
      auth = to;
      eosio_assert(has_auth(auth), "missing required authority");
   }

   auto ttype = get_token_type(quantity.symbol.code(), _issuer);
   eosio_assert(ttype != name(), "not registered token");

   INLINE_ACTION_SENDER(gxc::grc2, transfer)(
      ttype, {{_self, system::active_permission}, {auth, system::active_permission}},
      {from, to, quantity, memo, _issuer}
   );
}

void contract::setadmin(name account_name, name action_name, bool is_admin) {
   require_auth(_self);
   eosio_assert(is_account(account_name), "account not found");

   admintable at(_self, action_name.value);
   auto itr = at.find(account_name.value);

   if (is_admin) {
      eosio_assert(itr == at.end(), "already registered administrator");
      at.emplace(_self, [&](auto& a) {
         a.account_name = account_name;
      });
   } else {
      eosio_assert(itr != at.end(), "not registered administrator");
      at.erase(itr);
   }
}

} }

EOSIO_DISPATCH(gxc::token::contract, (create)(issue)(transfer)(retire)(setadmin))
