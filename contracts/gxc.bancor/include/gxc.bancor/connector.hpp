#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <cmath>

using namespace eosio;
using std::string;

namespace eosio {

constexpr name token_account = "gxc.token"_n;

struct token {
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

   static constexpr name active_permission{"active"_n};

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

   name contract; // issuer
   std::vector<permission_level> auths;
};

struct connector {
   extended_symbol smart; // 16
   asset balance;         // 32
   double weight = .5;    // 40

   struct converted {
      extended_asset value;
      double ratio;
   };

   converted convert_to_smart(const extended_asset& from, const extended_symbol& to) {
      const double S = token(to.get_contract()).get_supply(to.get_symbol().code()).quantity.amount; 
      const double C = balance.amount;
      const double dC = from.quantity.amount;

      double dS = S * (std::pow(1. + dC / C, weight) - 1.);
      if (dS < 0) dS = 0;

      auto conversion_rate = ((int64_t)dS) / dS;
      balance += { from.quantity.amount - int64_t(from.quantity.amount * (1 - conversion_rate)), from.quantity.symbol };

      return { {int64_t(dS), to}, conversion_rate };
   }

   converted convert_from_smart(const extended_asset& from, const extended_symbol& to) {
      const double C = balance.amount;
      const double S = token(from.contract).get_supply(from.quantity.symbol.code()).quantity.amount;
      const double dS = -from.quantity.amount;

      double dC = C * (std::pow(1. + dS / S, double(1) / weight) - 1.);
      if (dC > 0) dC = 0;

      balance.amount -= int64_t(-dC);

      return { {int64_t(-dC), to}, ((int64_t)-dC) / (-dC) };
   }

   uint64_t primary_key() const { return smart.get_symbol().code().raw(); }

   EOSLIB_SERIALIZE(connector, (smart)(balance)(weight))
};

} /// namespace eosio
