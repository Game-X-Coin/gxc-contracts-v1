/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>

#include <eosio-xt/eosio-xt.hpp>
#include <gxclib/symbol.hpp>
#include <gxclib/action.hpp>

using namespace eosio;

namespace gxc {

   class [[eosio::contract("gxc.token")]] token_contract : public contract {
   public:
      using contract::contract;
      using key_value = std::pair<std::string, std::vector<int8_t>>;

      void regtoken(name issuer, symbol_code symbol, name contract);

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
      void open(name owner, name issuer, symbol_code symbol, std::vector<key_value> opts);

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

      [[eosio::action]]
      void approve(name owner, name spender, extended_asset value);

      // dummy actions
      void withdraw(name owner, extended_asset value) {}
      void revtwithdraw(name owner, extended_asset value) {}

      static uint64_t get_id(const extended_asset& value) {
         auto sym_code = extended_symbol_code(value.quantity.symbol, value.contract);
         return xxh3_64(reinterpret_cast<const char*>(&sym_code), sizeof(uint128_t));
      }

      struct [[eosio::table("accounts"), eosio::contract("gxc.token")]] account_balance {
         enum opt {
            frozen = 0,
            whitelist
         };

         asset     balance;  // 16
         name      _issuer;  // 24, the lowest 4 bits assigned to opts
         int64_t   _deposit; // 32

         name get_issuer()const { return name(_issuer.value & 0xFFFFFFFFFFFFFFF0ULL); }
         void set_issuer(name issuer) { _issuer = name((_issuer.value & 0xF) | issuer.value); }
         asset get_deposit()const { return asset(_deposit, balance.symbol); }
         void set_deposit(const asset& quantity) {
            check(quantity.symbol == balance.symbol, "symbol mismatch");
            _deposit = quantity.amount;
         }
         bool get_opt(opt option)const { return (_issuer.value >> (0 + option)) & 0x1; }
         void set_opt(opt option, bool val) {
            if (val) _issuer.value |= 0x1 << option;
            else     _issuer.value &= ~(0x1 << option);
         }

         static uint64_t get_id(extended_asset value) { return token_contract::get_id(value); }

         uint64_t primary_key()const { return get_id(extended_asset(balance, get_issuer())); }
         uint64_t by_issuer()const   { return _issuer.value & 0xFFFFFFFFFFFFFFF0ULL; }

         EOSLIB_SERIALIZE( account_balance, (balance)(_issuer)(_deposit) )
      };

      typedef multi_index<"accounts"_n, account_balance,
                 indexed_by<"issuer"_n, const_mem_fun<account_balance, uint64_t, &account_balance::by_issuer>>
              > accounts;

      struct [[eosio::table("stat"), eosio::contract("gxc.token")]] currency_stats {
         enum opt {
            mintable = 0,
            recallable,
            freezable,
            pausable,
            paused,
            whitelistable,
            whitelist_on
         };

         asset    supply;      // 16
         int64_t  _max_supply; // 24
         name     issuer;      // 32
         uint32_t _opts = 0x7; // 36, defaults to (mintable, recallable, freezable)
         uint32_t withdraw_delay_sec = 24 * 3600; // 40, defaults to 1 day
         int64_t  _withdraw_min_amount; // 48

         asset get_max_supply()const { return asset(_max_supply, supply.symbol); }
         void set_max_supply(const asset& quantity) {
            check(quantity.symbol == supply.symbol, "symbol mismatch");
            _max_supply = quantity.amount;
         }
         bool get_opt(opt option)const { return (_opts >> (0 + option)) & 0x1; }
         void set_opt(opt option, bool val) {
            if (val) _opts |= 0x1 << option;
            else     _opts &= ~(0x1 << option);
         }
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

      struct [[eosio::table("withdraws"), eosio::contract("gxc.token")]] withdrawal_request {
         asset          quantity;
         name           issuer;
         time_point_sec scheduled_time;

         static uint64_t get_id(extended_asset value) { return token_contract::get_id(value); }

         uint64_t primary_key()const       { return get_id(extended_asset(quantity, issuer)); }
         uint64_t by_scheduled_time()const { return static_cast<uint64_t>(scheduled_time.utc_seconds); }
 
         EOSLIB_SERIALIZE( withdrawal_request, (quantity)(issuer)(scheduled_time) )
      };

      typedef multi_index<"withdraws"_n, withdrawal_request,
                 indexed_by<"schedtime"_n, const_mem_fun<withdrawal_request, uint64_t, &withdrawal_request::by_scheduled_time>>
              > withdraws;

      struct [[eosio::table("allowance"), eosio::contract("gxc.token")]] allowance {
         name  spender;  //  8
         asset quantity; // 24
         name  issuer;   // 32

         static uint64_t get_id(name spender, extended_asset value) {
            std::array<char,24> raw;
            auto sym_code = extended_symbol_code(value.quantity.symbol, value.contract).raw();
            datastream<char*> ds(raw.data(), raw.size());
            ds << spender;
            ds << sym_code;
            return xxh3_64(raw.data(), raw.size());
         }

         uint64_t primary_key()const { return get_id(spender, extended_asset(quantity, issuer)); }

         EOSLIB_SERIALIZE( allowance, (spender)(quantity)(issuer) )
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

         token(name receiver, name code, symbol_code symbol)
         : multi_index_wrapper(receiver, code, symbol.raw())
         {}

         token(name receiver, name code, symbol symbol)
         : token(receiver, code, symbol.code())
         {}

         token(name receiver, extended_asset value)
         : token(receiver, value.contract, value.quantity.symbol)
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
            auto _account = account(code(), owner, account_balance::get_id(extended_asset(asset(0, _this->supply.symbol), _this->issuer)), this);
            return _account;
         }

         inline name issuer()const { return scope(); }

      private:
         void _setopts(const std::vector<key_value>& opts, bool init = false);
      };

      class account : public multi_index_wrapper<accounts> {
      public:
         using opt = account_balance::opt;

         account(name receiver, name code, uint64_t key, const token* st)
         : multi_index_wrapper(receiver, code, key)
         , _st(st)
         {}

         void check_account_is_valid() {
            if (code() != owner()) {
               check(!_this->get_opt(opt::frozen), "account is frozen");
               check(!(*_st)->get_opt(token::opt::whitelist_on) || _this->get_opt(opt::whitelist), "account is not whitelisted");
            }
         }

         void setopts(const std::vector<key_value>& opts);
         void open();
         void close();
         void approve(name spender, extended_asset value);

         inline name owner()const  { return scope(); }
         inline name issuer()const { return _st->scope(); }

         inline const token& get_token()const { return *_st; }

         account& keep() {
            keep_balance = true;
            return *this;
         }

         account& unkeep() {
            keep_balance = false;
            return *this;
         }

      private:
         const token* _st;
         bool  keep_balance;

         void sub_balance(extended_asset value);
         void add_balance(extended_asset value);
         void sub_deposit(extended_asset value);
         void add_deposit(extended_asset value);
         void sub_allowance(name spender, extended_asset value);

         friend class token;
         friend class requests;
      };

      static time_point current_time_point() {
         return time_point(microseconds(static_cast<int64_t>(current_time())));
      }

      class requests : public multi_index_wrapper<withdraws> {
      public:
         using multi_index_wrapper::multi_index_wrapper;

         requests(name receiver, name code, extended_asset value)
         : multi_index_wrapper(receiver, code, withdrawal_request::get_id(value))
         {}

         void refresh_schedule(time_point_sec base_time = current_time_point());
         void clear();

         inline name owner()const  { return scope(); }
      };
   };
}
