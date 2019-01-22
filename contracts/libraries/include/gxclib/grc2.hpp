/**
 * @file
 * @copyright defined gxc/LICENSE
 */
#pragma once

#include <eosiolib/name.hpp>
#include <eosiolib/asset.hpp>

namespace gxc {

/**
 * GRC-2 standard token interface
 *
 * Multiple issuers can create token in one contract.
 * There can be tokens having same symbol but different issuers,
 * `issuer` should be given to `transfer` action.
 */

using eosio::name;
using eosio::asset;
using std::string;

struct grc2 {
   /**
    * Creates new token
    */
   void create (name issuer, asset maximum_supply);

   /**
    * Transfers token from `from` to `to`
    *
    * Following behaviors should be implemented:
    * * transfer from null account: issue new token
    * * transfer to null account: retire existing token
    */
   void transfer (name from, name to, asset quantity, string memo, name issuer);
};

}
