/**
 *  @file
 *  @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/binary_extension.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/transaction.hpp>

#include <gxclib/token.hpp>
#include <gxclib/game.hpp>
#include <gxclib/dispatcher.hpp>

using namespace eosio;
using std::string;

namespace gxc {

class [[eosio::contract("gxc.stdtoken")]] stdtoken : public contract  {
public:
   using contract::contract;

   void create (name issuer, asset maximum_supply);
   void retire (asset quantity, string memo, name issuer);
   void transfer (name from, name to, asset quantity, string memo, name issuer);
   [[eosio::action]] void deposit (name owner, asset quantity, name issuer);
   [[eosio::action]] void withdraw (name owner, asset quantity, name issuer);
   void drawout (name owner);

private:
   void issue (name to, asset quantity, string memo, name issuer);

   struct [[eosio::table("accounts"), eosio::contract("gxc.stdtoken")]] accountsrow {
      asset     balance;
      asset     deposit;
      name      issuer;
      uint64_t  id;

      uint64_t  primary_key()const { return id; }
      uint128_t secondary_key()const { return token::extended_symbol_code(balance.symbol.code(), issuer); }

      EOSLIB_SERIALIZE(accountsrow, (balance)(deposit)(issuer)(id));
   };

   struct [[eosio::table("stat"), eosio::contract("gxc.stdtoken")]] statsrow {
      asset supply;
      asset max_supply;
      name  issuer;

      uint64_t primary_key()const { return supply.symbol.code().raw(); }

      EOSLIB_SERIALIZE(statsrow, (supply)(max_supply)(issuer))
   };

   typedef eosio::multi_index<"accounts"_n, accountsrow,
      indexed_by <"extsymbol"_n, const_mem_fun<accountsrow, uint128_t, &accountsrow::secondary_key> >> accountstable;
   typedef eosio::multi_index<"stat"_n, statsrow > statstable;

   void sub_balance(name owner, asset value, name issuer);
   void add_balance(name owner, asset value, name issuer);
   void sub_deposit(name owner, asset value, name issuer);
   void add_deposit(name owner, asset value, name issuer);

   inline void _check_args(asset quantity, std::optional<string> memo = std::nullopt) {
      eosio_assert(quantity.symbol.is_valid(), "invalid symbol name");
      eosio_assert(quantity.is_valid(), "invalid quantity");
      eosio_assert(quantity.amount > 0, "must be positive quantity");
      eosio_assert(!memo || (*memo).size() <= 256, "memo has more than 256 bytes");
   }

   struct [[eosio::table("withdraws"), eosio::contract("gxc.stdtoken")]] withdrawrow {
      asset quantity;
      name issuer;
      time_point_sec request_time;
      uint64_t id;

      uint64_t primary_key()const { return id; }
      uint128_t secondary_key()const { return token::extended_symbol_code(quantity.symbol.code(), issuer); }
      uint64_t tertiary_key()const { return static_cast<uint64_t>(request_time.utc_seconds); }

      EOSLIB_SERIALIZE(withdrawrow, (quantity)(issuer)(request_time)(id))
   };

   typedef eosio::multi_index<"withdraws"_n, withdrawrow,
      indexed_by <"extsymbol"_n, const_mem_fun<withdrawrow, uint128_t, &withdrawrow::secondary_key>>,
      indexed_by <"reqtime"_n, const_mem_fun<withdrawrow, uint64_t, &withdrawrow::tertiary_key>> > withdrawtable;

   void refresh_drawout(withdrawtable& witd, name owner, time_point_sec ctp = current_time_point());

   static time_point current_time_point() {
      return time_point(microseconds(static_cast<int64_t>(current_time())));
   }

   constexpr static uint32_t withdraw_delay_sec = 10;
};

}
