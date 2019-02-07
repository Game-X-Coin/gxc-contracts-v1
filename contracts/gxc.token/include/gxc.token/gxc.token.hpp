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
         enum opt {
            frozen = 0,
            whitelist
         };

         asset     balance;  // 16
         name      issuer;   // 24
         int64_t   _deposit; // 32
         uint64_t  _id;      // 40

         asset get_deposit()const { return asset(_deposit, balance.symbol); }
         void set_deposit(const asset& quantity) {
            check(quantity.symbol == balance.symbol, "symbol mismatch");
            _deposit = quantity.amount;
         }
         bool get_opt(opt option)const { return (_id >> (56 + option)) & 0x1; }
         void set_opt(opt option, bool val) { _id |= (val & 0x1) << (56 + option); }
         void set_primary_key(uint64_t id) {
            check(id <= 0x00FFFFFFFFFFFFFFULL, "uppermost byte of `_id` is reserved for options");
            _id = (_id & 0xFF00000000000000ULL) | id;
         }

         uint64_t  primary_key()const    { return _id & 0x00FFFFFFFFFFFFFFULL; }
         uint128_t by_symbol_code()const { return extended_symbol_code(balance.symbol, issuer).raw(); }

         EOSLIB_SERIALIZE( account_balance, (balance)(issuer)(_deposit)(_id) )
      };

      struct [[eosio::table("stat"), eosio::contract("gxc.token")]] currency_stats {
         enum opt {
            can_recall = 0,
            can_freeze,
            can_whitelist,
            is_frozen,
            enforce_whitelist
         };

         asset    supply;      // 16
         int64_t  _max_supply; // 24
         name     issuer;      // 32
         uint32_t _opts = 0x3; // 36, defaults to (can_recall, can_freeeze)
         uint32_t withdraw_delay_sec;   // 40
         int64_t  _withdraw_min_amount; // 48

         asset get_max_supply()const { return asset(_max_supply, supply.symbol); }
         void set_max_supply(const asset& quantity) {
            check(quantity.symbol == supply.symbol, "symbol mismatch");
            _max_supply = quantity.amount;
         }
         bool get_opt(opt option)const { return (_opts >> (0 + option)) & 0x1; }
         void set_opt(opt option, bool val) { _opts |= (val & 0x1) << option; }
         asset get_withdraw_min_amount()const { return asset(_withdraw_min_amount, supply.symbol); }
         void set_withdraw_min_amount(const asset& quantity) {
            check(quantity.symbol == supply.symbol, "symbol mismatch");
            check(quantity.amount > 0, "withdraw_min_amount should be positive");
            _withdraw_min_amount = quantity.amount;
         }

         uint64_t primary_key()const { return supply.symbol.code().raw(); }

         EOSLIB_SERIALIZE( currency_stats, (supply)(_max_supply)(issuer)(_opts)
                                           (withdraw_delay_sec)(_withdraw_min_amount) )
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
         using opt = currency_stats::opt;

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
         using opt = account_balance::opt;

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
