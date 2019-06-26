#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <cmath>

using namespace eosio;
using std::string;

namespace eosio {

constexpr name active_permission{"active"_n};

struct token {
   static constexpr name token_account = "gxc.token"_n;

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

   typedef eosio::multi_index< "stat"_n, currency_stats > stats;

   extended_asset get_supply( const symbol_code& sym_code ) {
      stats statstable( token_account, contract.value );
      const auto& st = statstable.get( sym_code.raw() );
      return {st.supply, st.issuer};
   }

   token(name c, name auth = name()) {
      contract = c;
      if (auth != name())
         auths.emplace_back(permission_level(auth, active_permission));
   }

   void transfer(name from, name to, extended_asset value, string memo = "") {
      if (auths.empty())
         auths.emplace_back(permission_level(contract, active_permission));
      action_wrapper<"transfer"_n, &token::transfer>(token_account, auths).send(from, to, value, memo);
   }

   void approve(name owner, name spender, extended_asset value) {
      if (auths.empty())
         auths.emplace_back(permission_level(owner, active_permission));
      action_wrapper<"approve"_n, &token::approve>(token_account, auths).send(owner, spender, value);
   }

   name contract; // issuer
   std::vector<permission_level> auths;
};

struct reserve {
   static constexpr name reserve_account = "gxc.reserve"_n;

   struct [[eosio::table("reserve"), eosio::contract("gxc.reserve")]] currency_reserves {
      extended_asset derivative;
      asset underlying;
      double rate;

      uint64_t primary_key()const { return derivative.quantity.symbol.code().raw(); }

      EOSLIB_SERIALIZE( currency_reserves, (derivative)(underlying)(rate) )
   };

   typedef multi_index<"reserve"_n, currency_reserves> reserves;

   operator bool() { return has_reserve(); }

   bool has_reserve() {
      reserves rsv(reserve_account, derivative.get_contract().value);
      auto it = rsv.find(derivative.get_symbol().code().raw());
      return it != rsv.end();
   }

   double get_rate() {
      reserves rsv(reserve_account, derivative.get_contract().value);
      auto it = rsv.find(derivative.get_symbol().code().raw());
      if (it != rsv.end())
         return it->rate;
      return 0.;
   }

   reserve(extended_symbol d, name auth = name()) {
      derivative = d;
      if (auth != name())
         auths.emplace_back(permission_level(auth, active_permission));
   }

   void claim(name owner, extended_asset value) {
      if (auths.empty())
         auths.emplace_back(permission_level(owner, active_permission));
      action_wrapper<"claim"_n, &reserve::claim>(reserve_account, auths).send(owner, value);
   }

   extended_symbol derivative;
   std::vector<permission_level> auths;
};

struct connector {
   extended_symbol smart; // 16
   asset balance;         // 32
   double weight = .5;    // 40

   struct converted {
      extended_asset value;
      asset delta;
      double ratio;
   };

   converted convert_to_smart(const extended_asset& from, const extended_symbol& to, bool reverse = false) {
      const double S = token(to.get_contract()).get_supply(to.get_symbol().code()).quantity.amount; 
      const double C = balance.amount;
      const double dC = from.quantity.amount;

      double dS = S * (std::pow(1. + dC / C, weight) - 1.);
      if (dS < 0) dS = 0;

      auto conversion_rate = ((int64_t)dS) / dS;
      auto delta = asset{ from.quantity.amount - int64_t(from.quantity.amount * (1 - conversion_rate)), from.quantity.symbol };

      if (!reverse) balance += delta;
      else balance -= delta;

      return { {int64_t(dS), to}, delta, conversion_rate };
   }

   converted convert_from_smart(const extended_asset& from, const extended_symbol& to, bool reverse = false) {
      const double C = balance.amount;
      const double S = token(from.contract).get_supply(from.quantity.symbol.code()).quantity.amount;
      const double dS = -from.quantity.amount;

      double dC = C * (std::pow(1. + dS / S, double(1) / weight) - 1.);
      if (dC > 0) dC = 0;

      auto delta = asset{ int64_t(-dC), balance.symbol };

      if (!reverse) balance -= delta;
      else balance += delta;

      return { {int64_t(-dC), to}, delta, ((int64_t)-dC) / (-dC) };
   }

   converted convert_to_exact_smart(const extended_symbol& from, const extended_asset& to) {
      return convert_from_smart(to, from, true);
   }

   converted convert_exact_from_smart(const extended_symbol& from, const extended_asset& to) {
      return convert_to_smart(to, from, true);
   }

   uint64_t primary_key() const { return smart.get_symbol().code().raw(); }

   EOSLIB_SERIALIZE(connector, (smart)(balance)(weight))
};

} /// namespace eosio
