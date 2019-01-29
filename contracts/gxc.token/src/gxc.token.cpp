/**
 * @file
 * @copyright defined in gxc/LICENSE
 */

#include <gxc.token/gxc.token.hpp>

#include "token.cpp"
#include "account.cpp"

namespace gxc {

   constexpr name system_account {"gxc"_n};
   constexpr name null_account   {"gxc.null"_n};

   void token_contract::regtoken(name issuer, symbol symbol, name contract) {
      require_auth(_self);
      check(false, "external tokens are not supported yet");
   }

   void token_contract::create(extended_asset max_supply, std::vector<key_value> opts) {
      token(_self,
            max_supply.contract,
            max_supply.quantity.symbol.code().raw()).create(max_supply, opts);
   }

   void token_contract::setopts(name issuer, symbol symbol, std::vector<key_value> opts) {
      token(_self, issuer, symbol.code().raw()).setopts(opts);
   }

   void token_contract::setacntopts(name account, name issuer, symbol symbol, std::vector<key_value> opts) {
      token(_self, issuer, symbol.code().raw()).get_account(account).setopts(opts);
   }

   void token_contract::transfer(name from, name to, extended_asset quantity, std::string memo) {
      check(memo.size() <= 256, "memo has more than 256 bytes");
      check(from != to, "cannot transfer to self");
      check(is_account(to), "`to` account does not exist");

      auto _token = token(_self, quantity.contract, quantity.quantity.symbol.code().raw());

      if (from == null_account)
         _token.issue(to, quantity);
      else if (to == null_account)
         _token.retire(from, quantity);
      else
         _token.transfer(from, to, quantity);
   }

   void token_contract::burn(extended_asset quantity, std::string memo) {
      check(memo.size() <= 256, "memo has more than 256 bytes");

      auto _token = token(_self, quantity.contract, quantity.quantity.symbol.code().raw());
      _token.burn(quantity);
   }
}

EOSIO_DISPATCH(gxc::token_contract, (create)(transfer)(burn)(setopts)(setacntopts))
