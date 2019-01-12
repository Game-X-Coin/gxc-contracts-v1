/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <eosiolib/name.hpp>
#include <boost/utility/string_view.hpp>

namespace gxc {

using eosio::name;

bool starts_with(const name& input, const name& test) {
   uint64_t mask = 0xF800000000000000ull;
   auto maxlen = 12;
   auto v = input.value;
   auto c = test.value;

   for (auto i = 0; i < maxlen; ++i, v <<= 5, c<<= 5) {
      if ((v & mask) == (c & mask)) continue;

      if (c & mask) return false;
      else break;
   }

   return true;
}

bool starts_with(const name& input, const boost::string_view& test) {
   return boost::string_view(input.to_string()).starts_with(test);
}

bool has_dot(eosio::name input) {
   uint64_t mask = 0xF800000000000000ull;
   auto v = input.value;
   auto len = input.length();
   auto has_dot = false;

   for (auto i = 0; i < len; ++i, v <<= 5) {
      has_dot |= !(v & mask);
   }
   return has_dot;
}

}
