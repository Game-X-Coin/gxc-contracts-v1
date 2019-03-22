/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <utf8/utf8.h>
#include <gxclib/system.hpp>

using namespace eosio;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.user")]] user_contract : public contract {
public:
   using contract::contract;

   [[eosio::action]]
   void connect(name account_name, name game_name, string login_token);

   [[eosio::action]]
   void login(name account_name, name game_name, string login_token);

   [[eosio::action]]
   void setnick(name account_name, string nickname);

   [[eosio::action]]
   void rmvnick(name account_name);

   [[eosio::action]]
   void payram4nick(name account_name);

   void authenticate(name account_name, name game_name, const string& login_token);

   struct [[eosio::table, eosio::contract("gxc.user")]] nickrow {
      name    account_name;
      string  nickname;
      string  title;

      uint64_t    primary_key()const   { return account_name.value; }
      checksum256 secondary_key()const { return sha256(nickname.data(), nickname.size()); }

      EOSLIB_SERIALIZE(nickrow, (account_name)(nickname)(title))
   };

   typedef eosio::multi_index<"nick"_n, nickrow,
      indexed_by<"nickname"_n, const_mem_fun<nickrow, eosio::checksum256, &nickrow::secondary_key>>
   > nicktable;

   static bool is_valid_nickname(string nickname) {
      if (nickname.empty()) return false;

      auto it = nickname.begin();
      for (auto cp = utf8::unchecked::next(it) ; cp; cp = utf8::unchecked::next(it))
         if (!is_valid_char(cp)) return false;

      return true;
   }

private:
   constexpr static bool is_valid_char(uint32_t cp) {
      if( (cp >= 'A') && (cp <= 'Z') ) return true;
      if( (cp >= 'a') && (cp <= 'z') ) return true;
      if( (cp >= '0') && (cp <= '9') ) return true;
      if( (cp >= 0xAC00) && (cp <= 0xD7AF) ) return true;
      return false;
   }
};

}
