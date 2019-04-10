/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <gxclib/dispatcher.hpp>
#include <gxclib/system.hpp>
#include <gxclib/chain_types.hpp>
#include <gxclib/exchange_state.hpp>
#include <gxclib/name.hpp>
#include <gxclib/privileged.hpp>

using namespace eosio;

namespace gxc { namespace system {

struct [[eosio::table("global"), eosio::contract("gxc.system")]] gxc_global_state : gxc::blockchain_parameters {
   uint64_t free_ram()const { return max_ram_size - total_ram_bytes_reserved; }

   uint64_t max_ram_size = 64ll * 1024 * 1024 * 1024; // 64 GiB
   uint64_t total_ram_bytes_reserved = 0;
   int64_t  total_ram_stake = 0;
   uint16_t new_ram_per_block = 0;
   block_timestamp last_ram_increase;
   block_timestamp last_block_num;
   uint8_t  revision = 0;

   EOSLIB_SERIALIZE_DERIVED(gxc_global_state, gxc::blockchain_parameters,
                            (max_ram_size)(total_ram_bytes_reserved)(total_ram_stake)
                            (new_ram_per_block)(last_ram_increase)(last_block_num)(revision)
   )
};

class [[eosio::contract("gxc.system")]] contract : public eosio::contract {
public:
   contract(name s, name code, datastream<const char*> ds);
   ~contract();

   static constexpr name ram_account {"gxc.ram"_n};
   static constexpr name ramfee_account {"gxc.ramfee"_n};
   static constexpr name stake_account {"gxc.stake"_n};
   static constexpr name user_account {"gxc.user"_n};
   static constexpr symbol ramcore_symbol = symbol(symbol_code("RAMCORE"), 4);
   static constexpr symbol ram_symbol = symbol(symbol_code("RAM"), 0);

   static constexpr int64_t ram_gift_bytes = 4 * 1024; // 4KiB

   static symbol get_core_symbol(name system_account = system::account) {
      rammarket rm(system_account, system_account.value);
      const static auto sym = get_core_symbol( rm );
      return sym;
   }

   // system contract actions
   [[eosio::action]] void init (unsigned_int version, symbol core);
   [[eosio::action]] void onblock (ignore<block_header> header);
   [[eosio::action]] void setalimits (name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight);
   [[eosio::action]] void buyram (name payer, name receiver, asset quant);
   [[eosio::action]] void buyrambytes (name payer, name receiver, uint32_t bytes);
   [[eosio::action]] void sellram (name account, int64_t bytes);
   [[eosio::action]] void refund (name owner);
   [[eosio::action]] void delegatebw (name from, name receiver, asset stake_net_quantity, asset stake_cpu_quantity, bool transfer);
   [[eosio::action]] void undelegatebw (name from, name receiver, asset unstake_net_quantity, asset unstake_cpu_quantity);

   [[eosio::action]] void setram (uint64_t max_ram_size);
   [[eosio::action]] void setramrate (uint16_t bytes_per_block);
   [[eosio::action]] void setparams (const gxc::blockchain_parameters& params);
   [[eosio::action]] void setpriv (name account, uint8_t is_priv);
   [[eosio::action]] void updtrevision (uint8_t revision) {}
   [[eosio::action]] void genaccount (name creator, name name, authority owner, authority active, std::string nickname);

   // native action handelrs
   [[eosio::action]]
   void newaccount (name creator,
                    name name,
                    ignore<authority> owner,
                    ignore<authority> active);

   typedef action_wrapper<"newaccount"_n, &contract::newaccount> action_newaccount;

   [[eosio::action]]
   void updateauth (name account,
                    name permission,
                    ignore<name> parent,
                    ignore<authority> auth) {}

   [[eosio::action]]
   void deleteauth (ignore<name> account,
                    ignore<name> permission) {}

   [[eosio::action]]
   void linkauth (ignore<name> account,
                  ignore<name> code,
                  ignore<name> type,
                  ignore<name> requirement) {}

   [[eosio::action]]
   void unlinkauth (ignore<name> account,
                    ignore<name> code,
                    ignore<name> type) {}

   [[eosio::action]]
   void canceldelay (ignore<permission_level> canceling_auth, ignore<capi_checksum256> trx_id) {}

   [[eosio::action]]
   void onerror (ignore<uint128_t> sender_id, ignore<std::vector<char>> sent_trx) {}

   [[eosio::action]]
   void setabi (name account, const std::vector<char>& abi);

   [[eosio::action]]
   void setcode (name account, uint8_t vmtype, uint8_t vmversion, const std::vector<char>& code);

   struct [[eosio::table("abihash"), eosio::contract("gxc.system")]] abi_hash {
      name              owner;
      capi_checksum256  hash;
      uint64_t primary_key()const { return owner.value; }

      EOSLIB_SERIALIZE(abi_hash, (owner)(hash))
   };

private:
   using global_state_singleton = eosio::singleton<"global"_n, gxc_global_state>;
   rammarket               _rammarket;
   global_state_singleton  _global;
   gxc_global_state        _gstate;

   static symbol get_core_symbol(const rammarket& rm) {
      auto itr = rm.find(ramcore_symbol.raw());
      check(itr != rm.end(), "system contract must first be initialized");
      return itr->quote.balance.symbol;
   }

   static bool is_admin(const name& name) {
      return (name == system::account) || has_dot(name);
   }

   static gxc_global_state get_default_parameters();

   symbol core_symbol()const;

   void update_ram_supply();

   //defined in delegate_bandwidth.cpp
   void changebw( name from, name receiver,
                  asset stake_net_quantity, asset stake_cpu_quantity, bool transfer );
};

} }
