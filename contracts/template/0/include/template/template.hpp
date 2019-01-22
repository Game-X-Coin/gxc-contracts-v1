/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/eosio.hpp>

using namespace eosio;

@NAMESPACE_BEGIN
class [[eosio::contract("@CONTRACT")]] @CLASS : public contract {
public:
   using contract::contract;

   [[eosio::action]] void @ACTION (name account_name);
};
@NAMESPACE_END
