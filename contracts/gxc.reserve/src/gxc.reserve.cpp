/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.reserve/gxc.reserve.hpp>
#include <gxclib/token.hpp>

using token = gxc::token_contract_mock;

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
      r.derivative = derivative;
      r.underlying = underlying.quantity;
      r.rate = underlying.quantity.amount / double(derivative.quantity.amount);
   });

   // TODO: check allowance
   // deposit underlying asset to reserve
   token(_self).transfer(basename(derivative.contract), _self, underlying, "deposit in reserve");

   // create derivative token
   token(token_account).mint(derivative, opts);
}

#ifdef TARGET_TESTNET
void reserve::migrate(extended_symbol derivative) {
   using namespace eosio::internal_use_do_not_use;

   uint64_t code = _self.value;
   uint64_t scope = derivative.get_contract().value;
   uint64_t table = "reserve"_n.value;
   uint64_t id = derivative.get_symbol().code().raw();

   uint32_t it = db_find_i64(code, scope, table, id);
   check(it != db_end_i64(code, scope, table), "not found");

   currency_reserves c;
   size_t size = db_get_i64(it, (char*)&c, sizeof(c));

   if (size == sizeof(c)) return;

   c.rate = c.underlying.amount / double(c.derivative.quantity.amount);
   db_update_i64(it, 0, (char*)&c, sizeof(c));
}
#endif

} /// namespace gxc
