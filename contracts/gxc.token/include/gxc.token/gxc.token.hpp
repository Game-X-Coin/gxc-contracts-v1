/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>

#include <gxclib/symbol.hpp>
#include <gxclib/key_value.hpp>
#include <gxclib/multi_index.hpp>

using namespace eosio;

namespace gxc {

   class [[eosio::contract("gxc.token")]] token_contract : public contract {
   public:
      using contract::contract;

      void regtoken(name issuer, symbol symbol, name contract);

      [[eosio::action]]
      void create(extended_asset max_supply, std::vector<key_value> opts);

      [[eosio::action]]
      void transfer(name from, name to, extended_asset quantity, std::string memo);

      [[eosio::action]]
      void burn(extended_asset quantity, std::string memo);

      [[eosio::action]]
      void setopts(name issuer, symbol symbol, std::vector<key_value> opts);

      [[eosio::action]]
      void setacntopts(name account, name issuer, symbol symbol, std::vector<key_value> opts);

      [[eosio::action]]
      void open(name owner, name issuer, symbol symbol, std::vector<key_value> opts);

      [[eosio::action]]
      void close(name owner, name issuer, symbol symbol);

      [[eosio::action]]
      void deposit(name owner, extended_asset quantity);

      [[eosio::action]]
      void withdraw(name owner, extended_asset quantity);

      struct [[eosio::table("accounts"), eosio::contract("gxc.token")]] account_balance {
         asset    balance;
         asset    deposit;
         name     issuer;
         bool     frozen    = false;
         bool     whitelist = false;
         uint64_t id;

         uint64_t  primary_key()const    { return id; }
         uint128_t by_symbol_code()const { return extended_symbol_code(balance.symbol, issuer).raw(); }

         EOSLIB_SERIALIZE( account_balance, (balance)(deposit)(issuer)(frozen)(whitelist)(id) )
      };

      struct [[eosio::table("stat"), eosio::contract("gxc.token")]] currency_stats {
         asset    supply;
         asset    max_supply;
         name     issuer;
         asset    withdraw_min_amount;
         uint32_t withdraw_delay_sec  = 24 * 3600; // 1 day
         bool     can_freeze          = true;
         bool     can_recall          = true;
         bool     can_whitelist       = false;
         bool     is_frozen           = false;
         bool     enforce_whitelist   = false;

         uint64_t primary_key()const { return supply.symbol.code().raw(); }

         EOSLIB_SERIALIZE( currency_stats, (supply)(max_supply)(issuer)(withdraw_min_amount)(withdraw_delay_sec)
                                           (can_freeze)(can_recall)(can_whitelist)(is_frozen)(enforce_whitelist) )
      };

      typedef multi_index<"stat"_n, currency_stats> stat;
      typedef multi_index<"accounts"_n, account_balance,
                 indexed_by<"symcode"_n, const_mem_fun<account_balance, uint128_t, &account_balance::by_symbol_code>>
              > accounts;

      struct [[eosio::table("withdraws"), eosio::contract("gxc.token")]] withdrawal_request {
         asset          quantity;
         name           issuer;
         time_point_sec requested_time;
         uint64_t       id;

         uint64_t  primary_key()const       { return id; }
         uint128_t by_symbol_code()const    { return extended_symbol_code(quantity.symbol, issuer).raw(); }
         uint64_t  by_requested_time()const { return static_cast<uint64_t>(requested_time.utc_seconds); }

         EOSLIB_SERIALIZE( withdrawal_request, (quantity)(issuer)(requested_time)(id) )
      };

      typedef multi_index<"withdraws"_n, withdrawal_request,
                 indexed_by<"symcode"_n, const_mem_fun<withdrawal_request, uint128_t, &withdrawal_request::by_symbol_code>>,
                 indexed_by<"reqtime"_n, const_mem_fun<withdrawal_request, uint64_t, &withdrawal_request::by_requested_time>>
              > withdraws;

   private:
      static void check_asset_is_valid(asset quantity) {
         check(quantity.symbol.is_valid(), "invalid symbol name");
         check(quantity.is_valid(), "invalid quantity");
         check(quantity.amount > 0, "must be positive quantity");
      }

      static void check_asset_is_valid(extended_asset value) {
         check_asset_is_valid(value.quantity);
      }

      class token;
      class account;
      class requests;

      class token : public multi_index_item<stat> {
      public:
         token(name receiver, name code, symbol symbol)
         : multi_index_item(receiver, code, symbol.code().raw())
         {}

         token(name receiver, extended_asset value)
         : token(receiver, value.contract, value.quantity.symbol)
         {}

         void create(extended_asset max_supply, const std::vector<key_value>& opts);
         void setopts(const std::vector<key_value>& opts);
         void issue(name to, extended_asset quantity);
         void burn(extended_asset quantity);
         void retire(name owner, extended_asset quantity);
         void transfer(name from, name to, extended_asset quantity);
         void deposit(name owner, extended_asset value);
         void withdraw(name owner, extended_asset value);

         void check_symbol_is_valid(symbol symbol) {
            check(_this->supply.symbol == symbol, "symbol precision mismatch" );
         }

         account get_account(name owner)const {
            check(exists(), "token not found");
            auto _account = account(code(), owner,
                                    extended_symbol_code(_this->supply.symbol,
                                                         _this->issuer).raw(), this);
            return _account;
         }

         inline name issuer()const { return scope(); }
      };

      class account : public multi_index_item<accounts, "symcode"_n, uint128_t> {
      public:
         account(name receiver, name code, uint128_t key, const token* st)
         : multi_index_item(receiver, code, key)
         , _st(st)
         {}

         void check_account_is_valid() {
            if (code() != owner()) {
               check(!_this->frozen, "account is frozen");
               check(!(*_st)->enforce_whitelist || _this->whitelist, "account is not whitelisted");
            }
         }

         void setopts(const std::vector<key_value>& opts);
         void open();
         void close();

         inline name owner()const  { return scope(); }
         inline name issuer()const { return _st->scope(); }

         inline const token& get_token()const { return *_st; }

      private:
         const token* _st;

         void sub_balance(extended_asset value);
         void add_balance(extended_asset value);
         void sub_deposit(extended_asset value);
         void add_deposit(extended_asset value);

         friend class token;
         friend class requests;
      };

      static time_point current_time_point() {
         return time_point(microseconds(static_cast<int64_t>(current_time())));
      }

      class requests : public multi_index_item<withdraws, "symcode"_n, uint128_t> {
      public:
         using multi_index_item::multi_index_item;

         void refresh_schedule(time_point_sec base_time = current_time_point());
         void clear();

         inline name owner()const  { return scope(); }
      };
   };
}
