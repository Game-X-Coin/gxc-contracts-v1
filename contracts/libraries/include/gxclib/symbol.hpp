/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

namespace eosio {

   class extended_symbol_code {
   public:
      constexpr extended_symbol_code(): value(0) {}

      constexpr explicit extended_symbol_code( uint128_t raw )
      : value(raw) {}

      extended_symbol_code( symbol_code s, name c ) {
         value = static_cast<uint128_t>(s.raw()) << 64 | c.value;
      }

      extended_symbol_code( symbol s, name c )
      : extended_symbol_code( s.code(), c ) {}

      constexpr uint128_t raw()const { return value; }

      constexpr explicit operator bool()const { return value != 0; }

      friend constexpr bool operator == ( const extended_symbol_code& a, const extended_symbol_code& b ) {
         return a.value == b.value;
      }

      friend constexpr bool operator != ( const extended_symbol_code& a, const extended_symbol_code& b ) {
         return a.value != b.value;
      }

      friend constexpr bool operator < ( const extended_symbol_code& a, const extended_symbol_code& b ) {
         return a.value < b.value;
      }

   private:
      uint128_t value = 0;
   };
}
