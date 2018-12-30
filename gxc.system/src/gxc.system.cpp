/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#include <gxc.system/gxc.system.hpp>

#include "native.cpp"

namespace gxc { namespace system {

contract::contract(name s, name code, datastream<const char*> ds)
: eosio::contract::contract(s, code, ds)
, _rammarket(_self, _self.value)
, _global(_self, _self.value) {
   _gstate = _global.exists() ? _global.get() : get_default_parameters();
}

contract::~contract() {
   _global.set(_gstate, _self);
}

gxc_global_state contract::get_default_parameters() {
   gxc_global_state dp;
   get_blockchain_parameters(dp);
   return dp;
}

time_point contract::current_time_point() {
   const static time_point ct{ microseconds{ static_cast<int64_t>( current_time() ) } };
   return ct;
}

block_timestamp contract::current_block_time() {
   const static block_timestamp cbt{ current_time_point() };
   return cbt;
}

symbol contract::core_symbol()const {
   const static auto sym = get_core_symbol( _rammarket );
   return sym;
}

void contract::setram( uint64_t max_ram_size ) {
   require_auth( _self );

   eosio_assert( _gstate.max_ram_size < max_ram_size, "ram may only be increased" ); /// decreasing ram might result market maker issues
   eosio_assert( max_ram_size < 1024ll*1024*1024*1024*1024, "ram size is unrealistic" );
   eosio_assert( max_ram_size > _gstate.total_ram_bytes_reserved, "attempt to set max below reserved" );

   auto delta = int64_t(max_ram_size) - int64_t(_gstate.max_ram_size);
   auto itr = _rammarket.find(ramcore_symbol.raw());

   /**
    *  Increase the amount of ram for sale based upon the change in max ram size.
    */
   _rammarket.modify( itr, same_payer, [&]( auto& m ) {
      m.base.balance.amount += delta;
   });

   _gstate.max_ram_size = max_ram_size;
}

void contract::update_ram_supply() {
   auto cbt = current_block_time();

   if( cbt <= _gstate.last_ram_increase ) return;

   auto itr = _rammarket.find(ramcore_symbol.raw());
   auto new_ram = (cbt.slot - _gstate.last_ram_increase.slot)*_gstate.new_ram_per_block;
   _gstate.max_ram_size += new_ram;

   /**
    *  Increase the amount of ram for sale based upon the change in max ram size.
    */
   _rammarket.modify( itr, same_payer, [&]( auto& m ) {
      m.base.balance.amount += new_ram;
   });
   _gstate.last_ram_increase = cbt;
}

/**
 *  Sets the rate of increase of RAM in bytes per block. It is capped by the uint16_t to
 *  a maximum rate of 3 TB per year.
 *
 *  If update_ram_supply hasn't been called for the most recent block, then new ram will
 *  be allocated at the old rate up to the present block before switching the rate.
 */
void contract::setramrate( uint16_t bytes_per_block ) {
   require_auth( _self );

   update_ram_supply();
   _gstate.new_ram_per_block = bytes_per_block;
}

void contract::setparams( const eosio::blockchain_parameters& params ) {
   require_auth( _self );
   (eosio::blockchain_parameters&)(_gstate) = params;
   eosio_assert( 3 <= _gstate.max_authority_depth, "max_authority_depth should be at least 3" );
   set_blockchain_parameters( params );
}

void contract::setpriv( name account, uint8_t ispriv ) {
   require_auth( _self );
   set_privileged( account.value, ispriv );
}

void contract::setalimits( name account, int64_t ram, int64_t net, int64_t cpu ) {
   require_auth( _self );
   user_resources_table userres( _self, account.value );
   auto ritr = userres.find( account.value );
   eosio_assert( ritr == userres.end(), "only supports unlimited accounts" );
   set_resource_limits( account.value, ram, net, cpu );
}

void contract::init(unsigned_int version, symbol core) {
   require_auth(_self);
   eosio_assert(version.value == 0, "unsupported version for init action");

   auto itr = _rammarket.find(ramcore_symbol.raw());
   eosio_assert(itr == _rammarket.end(), "system contract has already been initialized");

   auto system_token_supply = token::get_supply(_self, core.code());
   eosio_assert(system_token_supply.symbol == core, "specified core symbol does not exist (precision mismatch)");
   eosio_assert(system_token_supply.amount > 0, "system token supply must be greater than 0");

   _rammarket.emplace(_self, [&](auto& m) {
      m.supply.amount = 100'000'000'000'000ll;
      m.supply.symbol = ramcore_symbol;
      m.base.balance.amount = int64_t(_gstate.free_ram());
      m.base.balance.symbol = ram_symbol;
      m.quote.balance.amount = system_token_supply.amount / 1000;
      m.quote.balance.symbol = core;
   });

   auto owner = chain::authority {
      .threshold = 1,
      .keys = {},
      .accounts = {{{_self, active_permission}, 1}},
      .waits = {}
   };

   auto active = chain::authority {
      .threshold = 1,
      .keys = {},
      .accounts = {{{_self, active_permission}, 1}},
      .waits = {}
   };

   auto newact = [&](name acnt) -> chain::newaccount {
      return {
         .creator = _self,
         .name = acnt,
         .owner = owner,
         .active = active
      };
   };

   if (!is_account(ram_account)) {
      action({{_self, active_permission}}, _self, "newaccount"_n, newact(ram_account)).send();
   }
   if (!is_account(ramfee_account)) {
      action({{_self, active_permission}}, _self, "newaccount"_n, newact(ramfee_account)).send();
   }
   if (!is_account(stake_account)) {
      action({{_self, active_permission}}, _self, "newaccount"_n, newact(stake_account)).send();
   }
}

} }

GXC_DISPATCH(gxc::system::contract,
   (newaccount)(updateauth)(deleteauth)(linkauth)(unlinkauth)(canceldelay)(onerror)(setabi)(setcode)
   (init)(setram)(setramrate)(setparams)(setpriv)(setalimits)
   (buyrambytes)(buyram)(sellram)
)
