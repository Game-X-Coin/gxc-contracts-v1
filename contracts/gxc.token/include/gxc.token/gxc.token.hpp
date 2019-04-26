/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <gxc.token/config.hpp>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

#include <gxclib/symbol.hpp>
#include <gxclib/action.hpp>

using namespace eosio;

namespace gxc {

   constexpr name active_permission {"active"_n};
   constexpr uint64_t MASK_4BITS = 0xFull;

   class [[eosio::contract("gxc.token")]] token_contract : public contract {
   public:
      using contract::contract;
      using key_value = std::pair<std::string, std::vector<int8_t>>;

      void regtoken(name issuer, symbol_code symbol, name contract);

      // ACTION LIST BEGIN

      [[eosio::action]]
      void mint(extended_asset value, std::vector<key_value> opts);

      [[eosio::action]]
      void transfer(name from, name to, extended_asset value, std::string memo);

      [[eosio::action]]
      void burn(extended_asset value, std::string memo);

      [[eosio::action]]
      void setopts(name issuer, symbol_code symbol, std::vector<key_value> opts);

      [[eosio::action]]
      void setacntopts(name account, name issuer, symbol_code symbol, std::vector<key_value> opts);

      [[eosio::action]]
      void setacntsopts(std::vector<name> accounts, name issuer, symbol_code symbol, std::vector<key_value> opts);

      [[eosio::action]]
      void open(name owner, name issuer, symbol_code symbol, name payer);

      [[eosio::action]]
      void close(name owner, name issuer, symbol_code symbol);

      [[eosio::action]]
      void deposit(name owner, extended_asset value);

      [[eosio::action]]
      void pushwithdraw(name owner, extended_asset value);

      [[eosio::action]]
      void popwithdraw(name owner, name issuer, symbol_code symbol);

      [[eosio::action]]
      void clrwithdraws(name owner);
      typedef action_wrapper<"clrwithdraws"_n, &token_contract::clrwithdraws> clear_withdraws;

      [[eosio::action]]
      void approve(name owner, name spender, extended_asset value);

      // dummy actions
      [[eosio::action]]
      void withdraw(name owner, extended_asset value) { require_auth(_self); }
      typedef action_wrapper<"withdraw"_n, &token_contract::withdraw> withdraw_processed;

      [[eosio::action]]
      void revtwithdraw(name owner, extended_asset value) { require_auth(_self); }
      typedef action_wrapper<"revtwithdraw"_n, &token_contract::revtwithdraw> withdraw_reverted;

      // ACTION LIST END

      static uint64_t get_token_id(const extended_asset& value) {
         auto sym_code = extended_symbol_code(value.quantity.symbol, value.contract);
         return token_hash(reinterpret_cast<const char*>(&sym_code), sizeof(uint128_t));
      }

      // To reduce ram usage, some fields in a row of multi-index table store more than one type of info.
      // Do not access field with underscore suffix directly, but use accessor methods.
      struct [[eosio::table("accounts"), eosio::contract("gxc.token")]] account_balance {
      public:
         asset     balance;  // 16
      private:
         name      issuer_;  // 24, the lowest 4 bits assigned to opts
         int64_t   deposit_; // 32

      public:
         enum opt {
            frozen = 0,
            whitelist
         };

         name issuer()const { return name(issuer_.value & ~MASK_4BITS); }
         void issuer(name account) { issuer_ = name((issuer_.value & MASK_4BITS) | account.value); }

         asset deposit()const { return asset(deposit_, balance.symbol); }
         void deposit(const asset& quantity) {
            check(quantity.symbol == balance.symbol, "symbol mismatch");
            deposit_ = quantity.amount;
         }

         bool option(opt n)const { return (issuer_.value >> (0 + n)) & 0x1; }
         void option(opt n, bool val) {
            if (val) issuer_.value |= 0x1 << n;
            else     issuer_.value &= ~(0x1 << n);
         }

         uint64_t primary_key()const { return get_token_id(extended_asset(balance, issuer())); }
         uint64_t by_issuer()const   { return issuer().value; }

         EOSLIB_SERIALIZE(account_balance, (balance)(issuer_)(deposit_))
      };

      typedef multi_index<"accounts"_n, account_balance,
                 indexed_by<"issuer"_n, const_mem_fun<account_balance, uint64_t, &account_balance::by_issuer>>
              > accounts;

      struct [[eosio::table("stat"), eosio::contract("gxc.token")]] currency_stats {
         asset    supply;      // 16
      private:
         int64_t  max_supply_; // 24
      public:
         name     issuer;      // 32
      private:
         uint32_t opts_ = 0x7; // 36, defaults to (mintable, recallable, freezable)
      public:
         uint32_t withdraw_delay_sec = 24 * 3600; // 40, defaults to 1 day
      private:
         int64_t  withdraw_min_amount_; // 48

      public:
         enum opt {
            mintable = 0,
            recallable,
            freezable,
            pausable,
            paused,
            whitelistable,
            whitelist_on
         };

         asset max_supply()const { return asset(max_supply_, supply.symbol); }
         void max_supply(const asset& quantity) {
            check(quantity.symbol == supply.symbol, "symbol mismatch");
            max_supply_ = quantity.amount;
         }

         bool option(opt n)const { return (opts_ >> (0 + n)) & 0x1; }
         void option(opt n, bool val) {
            if (val) opts_ |= 0x1 << n;
            else     opts_ &= ~(0x1 << n);
         }

         asset withdraw_min_amount()const { return asset(withdraw_min_amount_, supply.symbol); }
         void withdraw_min_amount(const asset& quantity) {
            check(quantity.symbol == supply.symbol, "symbol mismatch");
            check(quantity.amount > 0, "withdraw_min_amount should be positive");
            withdraw_min_amount_ = quantity.amount;
         }

         uint64_t primary_key()const { return supply.symbol.code().raw(); }

         EOSLIB_SERIALIZE(currency_stats, (supply)(max_supply_)(issuer)(opts_)
                                          (withdraw_delay_sec)(withdraw_min_amount_))
      };

      typedef multi_index<"stat"_n, currency_stats> stat;

      struct [[eosio::table("withdraws"), eosio::contract("gxc.token")]] withdrawal_request {
         asset          quantity;
         name           issuer;
         time_point_sec scheduled_time;

         inline extended_asset value()const { return extended_asset(quantity, issuer); }
        
         uint64_t primary_key()const       { return get_token_id(extended_asset(quantity, issuer)); }
         uint64_t by_scheduled_time()const { return static_cast<uint64_t>(scheduled_time.utc_seconds); }

         EOSLIB_SERIALIZE(withdrawal_request, (quantity)(issuer)(scheduled_time))
      };

      typedef multi_index<"withdraws"_n, withdrawal_request,
                 indexed_by<"schedtime"_n, const_mem_fun<withdrawal_request, uint64_t, &withdrawal_request::by_scheduled_time>>
              > withdraws;

      struct [[eosio::table("allowance"), eosio::contract("gxc.token")]] allowance {
         name  spender;  //  8
         asset quantity; // 24
         name  issuer;   // 32

         static uint64_t get_approval_id(name spender, extended_asset value) {
            std::array<char,24> raw;
            auto sym_code = extended_symbol_code(value.quantity.symbol, value.contract).raw();
            datastream<char*> ds(raw.data(), raw.size());
            ds << spender;
            ds << sym_code;
            return token_hash(raw.data(), raw.size());
         }

         inline extended_asset value()const { return extended_asset(quantity, issuer); }

         uint64_t primary_key()const { return get_approval_id(spender, extended_asset(quantity, issuer)); }

         EOSLIB_SERIALIZE(allowance, (spender)(quantity)(issuer))
      };

      typedef multi_index<"allowance"_n, allowance> allowed;

   private:
      static void check_asset_is_valid(asset quantity, bool zeroable = false) {
         check(quantity.symbol.is_valid(), "invalid symbol name `" + quantity.symbol.code().to_string() + "`");
         check(quantity.is_valid(), "invalid quantity");
         if (zeroable)
            check(quantity.amount >= 0, "must not be negative quantity");
         else
            check(quantity.amount > 0, "must be positive quantity");
      }

      static void check_asset_is_valid(extended_asset value, bool zeroable = false) {
         check_asset_is_valid(value.quantity, zeroable);
      }

      class token;
      class account;
      class requests;

      class token : public multi_index_wrapper<stat> {
      public:
         using opt = currency_stats::opt;

         token(name code, name scope, symbol_code symbol)
         : multi_index_wrapper(code, scope, symbol.raw())
         {}

         token(name code, name scope, symbol symbol)
         : token(code, scope, symbol.code())
         {}

         token(name code, extended_asset value)
         : token(code, value.contract, value.quantity.symbol)
         {}

         void mint(extended_asset value, const std::vector<key_value>& opts);
         void setopts(const std::vector<key_value>& opts);
         void issue(name to, extended_asset quantity);
         void burn(extended_asset quantity);
         void retire(name owner, extended_asset quantity);
         void transfer(name from, name to, extended_asset quantity);
         void deposit(name owner, extended_asset value);
         void withdraw(name owner, extended_asset value);
         void cancel_withdraw(name owner, name issuer, symbol_code symbol);

         account get_account(name owner)const {
            check(exists(), "token not found");
            return account(code(), owner, get_token_id(extended_asset(asset(0, _this->supply.symbol), _this->issuer)), *this);           
         }

         inline name issuer()const { return scope(); }

      private:
         void _setopts(const std::vector<key_value>& opts, bool init = false);
      };

      class account : public multi_index_wrapper<accounts> {
      public:
         using opt = account_balance::opt;

         account(name code, name scope, uint64_t key, const token& st)
         : multi_index_wrapper(code, scope, key)
         , _st(st)
         {}

         void check_account_is_valid() {
            if (code() != owner()) {
               check(!_this->option(opt::frozen), "account is frozen");
               check(!_st->option(token::opt::whitelist_on) || _this->option(opt::whitelist), "not whitelisted account");
            }
         }

         void setopts(const std::vector<key_value>& opts);
         void open();
         void close();
         void approve(name spender, extended_asset value);

         inline name owner()const  { return scope(); }
         inline name issuer()const { return _st.scope(); }

         inline const token& get_token()const { return _st; }

         account& keep() {
            keep_balance = true;
            return *this;
         }

         account& unkeep() {
            keep_balance = false;
            return *this;
         }

         account& paid_by(name payer) {
            ram_payer = payer;
            return *this;
         }

      private:
         const token& _st;
         bool  keep_balance;
         name  ram_payer = eosio::same_payer;

         void sub_balance(extended_asset value);
         void add_balance(extended_asset value);
         void sub_deposit(extended_asset value);
         void add_deposit(extended_asset value);
         void sub_allowance(name spender, extended_asset value);

         friend class token;
         friend class requests;
      };

      class requests : public multi_index_wrapper<withdraws> {
      public:
         using multi_index_wrapper::multi_index_wrapper;

         requests(name code, name scope, extended_asset value)
         : multi_index_wrapper(code, scope, get_token_id(value))
         {}

         void refresh_schedule();
         void clear();

         inline name owner()const  { return scope(); }
      };
   };
}
