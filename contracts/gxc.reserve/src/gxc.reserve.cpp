/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.reserve/gxc.reserve.hpp>
#include <gxclib/token.hpp>

using token_contract = gxc::token_contract_mock;

namespace gxc {

void reserve::mint(extended_asset derivative, extended_asset underlying, std::vector<key_value> opts) {
   require_vauth(derivative.contract);
   require_gauth(derivative.contract);

   std::vector<std::string> valid_opts = {
      "withdraw_min_amount",
      "withdraw_delay_sec"
   };
   auto option_is_valid = [&](const std::string& key) -> bool {
      return std::find(valid_opts.begin(), valid_opts.end(), key) != valid_opts.end();
   };

   for (auto o : opts) {
      check(option_is_valid(o.first), "not allowed to set option `" + o.first + "`"); 
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
   token_contract({{_self, active_permission}}).transfer(basename(derivative.contract), _self, underlying, "deposit in reserve");

   // create derivative token
   token_contract({{token_account, active_permission}}).mint(derivative, opts);
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
   token_contract({{_self, active_permission}}).transfer(owner, _self, value, "claim reserve");
   token_contract({{_self, active_permission}}).transfer(_self, basename(value.contract), value, "claim reserve");
   token_contract({{token_account, active_permission}}).burn(value, "claim reserve");
   token_contract({{_self, active_permission}}).transfer(_self, owner, extended_asset(claimed_asset, system_account), "claim reserve");
}

} /// namespace gxc

EOSIO_DISPATCH(gxc::reserve, (mint)(claim))
