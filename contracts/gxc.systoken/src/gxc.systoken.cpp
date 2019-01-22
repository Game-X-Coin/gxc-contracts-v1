/**
 *  @file
 *  @copyright defined in gxc/LICENSE
 */

#include <gxc.systoken/gxc.systoken.hpp>

namespace gxc {

void systoken::create(name issuer, asset maximum_supply) {
   token::validate(token::account);
   eosio_assert(issuer == system::account, "only system account can create systoken");

   _check_args(maximum_supply);

   statstable stats(_self, issuer.value);
   auto existing = stats.find(maximum_supply.symbol.code().raw());
   eosio_assert(existing == stats.end(), "token with symbol already exists");

   stats.emplace(_self, [&](auto& s) {
      s.supply.symbol = maximum_supply.symbol;
      s.max_supply    = maximum_supply;
      s.issuer        = issuer;
   });
}


void systoken::issue(name to, asset quantity, string memo, name issuer) {
   _check_args(quantity, memo);

   statstable stats(_self, issuer.value);
   const auto& st = stats.get(quantity.symbol.code().raw(), "token with symbol does not exist, create token before issue");

   token::validate(st.issuer);

   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

   stats.modify(st, same_payer, [&](auto& s) {
      s.supply += quantity;
   });

   add_balance(st.issuer, quantity);

   if (to != st.issuer) {
      action(permission_level(_self, system::active_permission),
         token::account, "transfer"_n, std::make_tuple(st.issuer, to, quantity, memo, st.issuer)
      ).send();
   }
}

void systoken::retire(asset quantity, string memo, name issuer) {
   _check_args(quantity, memo);

   statstable stats(_self, issuer.value);
   const auto& st = stats.get(quantity.symbol.code().raw());

   token::validate(st.issuer);
   eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

   stats.modify(st, same_payer, [&](auto& s) {
      s.supply -= quantity;
   });

   sub_balance(st.issuer, quantity);
}

void systoken::transfer(name from, name to, asset quantity, string memo, name issuer) {
   _check_args(quantity, memo);

   eosio_assert(from != to, "cannot transfer to self");
   token::validate(from);
   eosio_assert(is_account( to ), "to account does not exist");

   statstable stats(_self, issuer.value);
   const auto& st = stats.get(quantity.symbol.code().raw());

   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

   sub_balance(from, quantity);
   add_balance(to, quantity);

   //require_recipient(from);
   //require_recipient(to);
}

void systoken::sub_balance(name owner, asset value) {
   accountstable from_acnts(_self, owner.value);

   const auto& from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
   eosio_assert(from.balance.amount >= value.amount, "overdrawn balance");

   from_acnts.modify(from, same_payer, [&](auto& a) {
      a.balance -= value;
   });
}

void systoken::add_balance(name owner, asset value) {
   accountstable to_acnts(_self, owner.value);

   auto to = to_acnts.find(value.symbol.code().raw());
   if (to == to_acnts.end()) {
      to_acnts.emplace(_self, [&](auto& a) {
         a.balance = value;
      });
   } else {
      to_acnts.modify(to, same_payer, [&](auto& a) {
         a.balance += value;
      });
   }
}

}

EOSIO_DISPATCH(gxc::systoken, (create)(issue)(transfer)(retire))
