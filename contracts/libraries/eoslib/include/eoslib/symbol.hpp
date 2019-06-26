#pragma once

#include <eosio/symbol.hpp>
#include <eosio/check.hpp>

namespace eosio {

   class extended_symbol_code {
   public:
      constexpr extended_symbol_code(): value(0)
      {}

      constexpr explicit extended_symbol_code( uint128_t raw )
      : value(raw)
      {}

      constexpr explicit extended_symbol_code( symbol_code s, name c )
      : value( static_cast<uint128_t>(c.value) << 64 | s.raw() )
      {}

      constexpr explicit extended_symbol_code( std::string_view str )
      : value(0)
      {
         auto at_pos = str.find('@');
         if (at_pos == std::string_view::npos) {
            eosio::check(false, "extended symbol should contain '@'");
         }
         *this = extended_symbol_code(symbol_code(str.substr(0, at_pos)), name(str.substr(at_pos+1)));
      }

      constexpr uint128_t raw()const { return value; }

      constexpr explicit operator bool()const { return value != 0; }

      std::string to_string()const {
         return get_symbol_code().to_string() + "@" + get_contract().to_string();
      }

      inline void print()const {
         auto str = to_string();
         if (str.size())
            printl(str.data(), str.size());
      }

      symbol_code get_symbol_code()const {
         return symbol_code(value & std::numeric_limits<uint64_t>::max());
      }

      name get_contract()const {
         return name(static_cast<uint64_t>(value >> 64));
      }

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
