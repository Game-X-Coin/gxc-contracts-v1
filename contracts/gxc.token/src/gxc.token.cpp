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

   void token_contract::regtoken(name issuer, symbol_code symbol, name contract) {
      require_auth(_self);
      check(false, "external tokens are not supported yet");
   }

   void token_contract::mint(extended_asset value, std::vector<key_value> opts) {
      token(_self, value).mint(value, opts);
   }

   void token_contract::setopts(name issuer, symbol_code symbol, std::vector<key_value> opts) {
      token(_self, issuer, symbol).setopts(opts);
   }

   void token_contract::setacntopts(name account, name issuer, symbol_code symbol, std::vector<key_value> opts) {
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

   void token_contract::open(name owner, name issuer, symbol_code symbol, std::vector<key_value> opts) {
      auto _token = token(_self, issuer, symbol);

      auto _account = _token.get_account(owner);
      _account.open();
      _account.setopts(opts);
   }

   void token_contract::close(name owner, name issuer, symbol_code symbol) {
      token(_self, issuer, symbol).get_account(owner).close();
   }

   void token_contract::deposit(name owner, extended_asset value) {
      token(_self, value).deposit(owner, value);
   }

   void token_contract::pushwithdraw(name owner, extended_asset value) {
      token(_self, value).withdraw(owner, value);
   }

   void token_contract::popwithdraw(name owner, name issuer, symbol_code symbol) {
      token(_self, issuer, symbol).cancel_withdraw(owner, issuer, symbol);
   }

   void token_contract::clrwithdraws(name owner) {
      requests(_self, owner).clear();
   }

   void token_contract::approve(name owner, name spender, extended_asset value) {
      token(_self, value).get_account(owner).approve(spender, value);
   }
}

EOSIO_DISPATCH(gxc::token_contract,
   (mint)(transfer)(burn)(setopts)(setacntopts)(open)(close)
   (deposit)(pushwithdraw)(popwithdraw)(clrwithdraws)(withdraw)(revtwithdraw)(approve)
)
