#pragma once
#include <eosio/privileged.hpp>
#include <eosio/serialize.hpp>
#include <eosio/crypto.hpp>

namespace gxc {
   /**
    * @defgroup privilegedcppapi Privileged C++ API
    * @ingroup privilegedapi
    * @brief Defines C++ Privileged API
    *
    * @{
    */

   /**
    * Tunable blockchain configuration that can be changed via consensus
    *
    * @brief Tunable blockchain configuration that can be changed via consensus
    */
   struct blockchain_parameters {

      uint64_t max_block_net_usage;

      uint32_t target_block_net_usage_pct;

      uint32_t max_transaction_net_usage;

      /**
       * The base amount of net usage billed for a transaction to cover incidentals
       * @brief The base amount of net usage billed for a transaction to cover incidentals
       */
      uint32_t base_per_transaction_net_usage;

      uint32_t net_usage_leeway;

      uint32_t context_free_discount_net_usage_num;

      uint32_t context_free_discount_net_usage_den;

      uint32_t max_block_cpu_usage;

      uint32_t target_block_cpu_usage_pct;

      uint32_t max_transaction_cpu_usage;

      uint32_t min_transaction_cpu_usage;

      /**
       * Maximum lifetime of a transacton
       *
       * @brief Maximum lifetime of a transacton
       */
      uint32_t max_transaction_lifetime;

      uint32_t deferred_trx_expiration_window;

      uint32_t max_transaction_delay;

      /**
       * Maximum size of inline action
       *
       * @brief Maximum size of inline action
       */
      uint32_t max_inline_action_size;

      /**
       * Maximum depth of inline action
       *
       * @brief Maximum depth of inline action
       */
      uint16_t max_inline_action_depth;

      /**
       * Maximum authority depth
       *
       * @brief Maximum authority depth
       */
      uint16_t max_authority_depth;

      int64_t net_weight_modifier;
      int64_t cpu_weight_modifier;


      EOSLIB_SERIALIZE( blockchain_parameters,
                        (max_block_net_usage)(target_block_net_usage_pct)
                        (max_transaction_net_usage)(base_per_transaction_net_usage)(net_usage_leeway)
                        (context_free_discount_net_usage_num)(context_free_discount_net_usage_den)

                        (max_block_cpu_usage)(target_block_cpu_usage_pct)
                        (max_transaction_cpu_usage)(min_transaction_cpu_usage)

                        (max_transaction_lifetime)(deferred_trx_expiration_window)(max_transaction_delay)
                        (max_inline_action_size)(max_inline_action_depth)(max_authority_depth)

                        (net_weight_modifier)(cpu_weight_modifier)
      )
   };

   /**
    * @brief Set the blockchain parameters
    * Set the blockchain parameters
    * @param params - New blockchain parameters to set
    */
   void set_blockchain_parameters(const gxc::blockchain_parameters& params) {
      char buf[sizeof(gxc::blockchain_parameters)];
      eosio::datastream<char *> ds( buf, sizeof(buf) );
      ds << params;
      eosio::internal_use_do_not_use::set_blockchain_parameters_packed( buf, ds.tellp() );
   }

   /**
    * @brief Retrieve the blolckchain parameters
    * Retrieve the blolckchain parameters
    * @param params - It will be replaced with the retrieved blockchain params
    */
   void get_blockchain_parameters(gxc::blockchain_parameters& params) {
      char buf[sizeof(gxc::blockchain_parameters)];
      size_t size = eosio::internal_use_do_not_use::get_blockchain_parameters_packed( buf, sizeof(buf) );
      eosio::check( size <= sizeof(buf), "buffer is too small" );
      eosio::datastream<const char*> ds( buf, size_t(size) );
      ds >> params;
   }

   ///@} priviledgedcppapi
}
