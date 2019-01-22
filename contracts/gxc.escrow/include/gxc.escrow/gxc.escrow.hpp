/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#include <gxclib/system.hpp>
#include <gxclib/game.hpp>
#include <gxc.token/gxc.token.hpp>

using namespace eosio;
using gxc::token::get_float_amount;

namespace gxc {

class [[eosio::contract("gxc.escrow")]] escrow : public eosio::contract {
public:
   using contract::contract;

   [[eosio::action]] void deposit (name issuer, asset derv, asset base);
   [[eosio::action]] void claim (name account_name, asset quantity, name issuer);

   struct [[eosio::table, eosio::contract("gxc.escrow")]] escrowrow {
      asset derv;
      asset base;
      name  issuer;

      uint64_t primary_key()const { return derv.symbol.code().raw(); }

      EOSLIB_SERIALIZE(escrowrow, (derv)(base)(issuer))
   };

   typedef multi_index<"escrow"_n, escrowrow> escrowtable;
};

}
