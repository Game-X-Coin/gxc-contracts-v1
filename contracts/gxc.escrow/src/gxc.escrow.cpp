/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.escrow/gxc.escrow.hpp>

namespace gxc {

void escrow::deposit(name issuer, asset derv, asset base) {
   require_auth(issuer);
   eosio_assert(game::is_game(issuer), "issuer should be a game account");

   escrowtable escr(_self, issuer.value);
   auto itr = escr.find(derv.symbol.code().raw());

   eosio_assert(itr == escr.end(), "additional issuance not supported");
   escr.emplace(_self, [&](auto& es) {
      es.derv = derv;
      es.base = base;
      es.issuer = issuer;
   });

   // deposit base asset to escrow
   action(permission_level(_self, system::active_permission),
      token::account, "transfer"_n, std::make_tuple(issuer, _self, base, string("deposit escrow"), system::account)
   ).send();

   // create derivative token
   action(permission_level(_self, system::active_permission),
      token::account, "create"_n, std::make_tuple(issuer, derv, token::standard_token)
   ).send();
}

void escrow::claim(name account_name, asset quantity, name issuer) {
   eosio_assert(quantity.amount > 0, "invalid quantity");
   require_auth(account_name);

   escrowtable escr(_self, issuer.value);

   const auto& st = escr.get(quantity.symbol.code().raw(), "base asset for given token not found");

   auto base_amount = get_float_amount(st.base);
   auto derv_amount = get_float_amount(st.derv);

   auto ratio = base_amount / derv_amount;
   auto claimed_amount = ratio * get_float_amount(quantity);
   auto claimed_asset = asset((int64_t)(claimed_amount * pow(10, st.base.symbol.precision())), st.base.symbol);
   eosio_assert(claimed_asset.amount > 0, "least amount for claim request not satisfied");

   // adjust input amount
   quantity.amount = (int64_t)(get_float_amount(claimed_asset) / ratio * pow(10, st.derv.symbol.precision()));

   // transfer token
   action(permission_level(_self, system::active_permission),
      token::account, "transfer"_n, std::make_tuple(account_name, _self, quantity, string("claim escrow"), issuer)
   ).send();

   action(permission_level(_self, system::active_permission),
      token::account, "transfer"_n, std::make_tuple(_self, issuer, quantity, string("claim escrow"), issuer)
   ).send();

   // FIXME: exterminate token (not retire)
   action(permission_level(_self, system::active_permission),
      token::account, "retire"_n, std::make_tuple(quantity, string("claim escrow"), issuer)
   ).send();

   // transfer system token
   action(permission_level(_self, system::active_permission),
      token::account, "transfer"_n, std::make_tuple(_self, account_name, claimed_asset, string("claim escrow"), system::account)
   ).send();
}

} /// namespace gxc

EOSIO_DISPATCH(gxc::escrow, (deposit)(claim))
