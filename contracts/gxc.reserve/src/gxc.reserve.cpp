/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.reserve/gxc.reserve.hpp>

namespace gxc {

using system::active_permission;

void reserve::mint(extended_asset derivative, extended_asset underlying, std::vector<key_value> opts) {
   require_vauth(derivative.contract);
   check_is_game(derivative.contract);

   std::vector<std::string> valid_opts = {
      "withdraw_min_amount",
      "withdraw_delay_sec"
   };
   auto option_is_valid = [&](const std::string& key) -> bool {
      return std::find(valid_opts.begin(), valid_opts.end(), key) != valid_opts.end();
   };

   for (auto o : opts) {
      check(option_is_valid(o.key), "not allowed to set option `" + o.key + "`"); 
   }

   check(underlying.contract == name("gxc"), "underlying asset should be system token");

   reserves rsv(_self, derivative.contract.value);
   auto it = rsv.find(derivative.quantity.symbol.code().raw());

   check(it == rsv.end(), "additional issuance not supported yet");
   rsv.emplace(_self, [&](auto& r) {
      r.derivative = derivative.quantity;
      r.issuer     = derivative.contract;
      r.underlying = underlying.quantity;
   });

   // TODO: check allowance
   // deposit underlying asset to reserve
   action(permission_level(_self, system::active_permission),
      token_account, "transfer"_n, std::make_tuple(derivative.contract, _self, underlying, string("deposit in reserve"))
   ).send();

   // create derivative token
   action(permission_level(token_account, system::active_permission),
      token_account, "mint"_n, std::make_tuple(derivative, opts)
   ).send();
}

void reserve::claim(name owner, extended_asset value) {
   check(value.quantity.amount > 0, "invalid quantity");
   require_auth(owner);

   reserves rsv(_self, value.contract.value);
   const auto& it = rsv.get(value.quantity.symbol.code().raw(), "underlying asset not found");

   auto ul_amount = get_float_amount(it.underlying);
   auto de_amount = get_float_amount(it.derivative);

   const auto ratio = ul_amount / de_amount;
   auto claimed_amount = ratio * get_float_amount(value.quantity);
   check(claimed_amount > 0, "minimum amount for claim not satisfied");

   auto claimed_asset = asset(static_cast<int64_t>(claimed_amount * pow(10, it.underlying.symbol.precision())), it.underlying.symbol);

   // adjust input amount
   value.quantity.amount = static_cast<int64_t>(get_float_amount(claimed_asset) / ratio * pow(10, it.derivative.symbol.precision()));

   // transfer token
   action(
      {_self, active_permission}, token_account, "transfer"_n, std::make_tuple(owner, _self, value, string("claim reserve"))
   ).send();

   action(
      {_self, active_permission}, token_account, "transfer"_n, std::make_tuple(_self, value.contract, value, string("claim reserve"))
   ).send();

   action(
      {token_account, active_permission}, token_account, "burn"_n, std::make_tuple(value, string("claim reserve"))
   ).send();

   action(
      {_self, active_permission}, token_account, "transfer"_n,
      std::make_tuple(_self, owner, extended_asset(claimed_asset, system::account),  string("claim reserve"))
   ).send();
}

} /// namespace gxc

EOSIO_DISPATCH(gxc::reserve, (mint)(claim))
