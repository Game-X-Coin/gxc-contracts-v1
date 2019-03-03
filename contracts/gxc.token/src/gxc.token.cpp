/**
 * @file
 * @copyright defined in gxc/LICENSE
 */

#include <gxc.token/gxc.token.hpp>

#include "token.cpp"
#include "account.cpp"
#include "requests.cpp"

namespace gxc {

   constexpr name system_account {"gxc"_n};
   constexpr name null_account   {"gxc.null"_n};

   void token_contract::regtoken(name issuer, symbol symbol, name contract) {
      require_auth(_self);
      check(false, "external tokens are not supported yet");
   }

   void token_contract::create(extended_asset max_supply, std::vector<key_value> opts) {
      token(_self, max_supply).create(max_supply, opts);
   }

   void token_contract::setopts(name issuer, symbol symbol, std::vector<key_value> opts) {
      token(_self, issuer, symbol).setopts(opts);
   }

   void token_contract::setacntopts(name account, name issuer, symbol symbol, std::vector<key_value> opts) {
      token(_self, issuer, symbol).get_account(account).setopts(opts);
   }

   void token_contract::transfer(name from, name to, extended_asset value, std::string memo) {
      check(memo.size() <= 256, "memo has more than 256 bytes");
      check(from != to, "cannot transfer to self");
      check(is_account(to), "`to` account does not exist");

      auto _token = token(_self, value);

      if (from == null_account)
         _token.issue(to, value);
      else if (to == null_account)
         _token.retire(from, value);
      else
         _token.transfer(from, to, value);
   }

   void token_contract::burn(extended_asset value, std::string memo) {
      check(memo.size() <= 256, "memo has more than 256 bytes");

      token(_self, value).burn(value);
   }

   void token_contract::open(name owner, name issuer, symbol symbol, std::vector<key_value> opts) {
      auto _token = token(_self, issuer, symbol);
      _token.check_symbol_is_valid(symbol);

      auto _account = _token.get_account(owner);
      _account.open();
      _account.setopts(opts);
   }

   void token_contract::close(name owner, name issuer, symbol symbol) {
      token(_self, issuer, symbol).get_account(owner).close();
   }

   void token_contract::deposit(name owner, extended_asset value) {
      token(_self, value).deposit(owner, value);
   }

   void token_contract::reqwithdraw(name owner, extended_asset value) {
      token(_self, value).withdraw(owner, value);
   }

   void token_contract::withdraw(name owner, extended_asset value) {
      /* Do noting, but just for tracking withdraw */
   }

   void token_contract::clearreqs(name owner) {
      requests(_self, owner).clear();
   }
}

EOSIO_DISPATCH(gxc::token_contract, (create)(transfer)(burn)(setopts)(setacntopts)(open)(close)(deposit)(reqwithdraw)(withdraw)(clearreqs))
