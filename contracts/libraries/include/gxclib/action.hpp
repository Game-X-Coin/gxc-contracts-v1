/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once
#include <eosio/action.hpp>

namespace eosio {

name rootname(name n) {
   auto mask = (uint64_t) -1;
   for (auto i = 0; i < 12; ++i) {
      if (n.value & (0x1FULL << (4 + 5 * (11 - i))))
         continue;
      mask <<= 4 + 5 * (11 - i);
      break;
   }
   return name(n.value & mask);
}

name basename(name n) {
   if (is_account(n))
      return n;
   else
      return rootname(n);
}

inline bool has_vauth(name n) {
   return internal_use_do_not_use::has_auth(basename(n).value);
}

inline void require_vauth(name n) {
   internal_use_do_not_use::require_auth(basename(n).value);
}

}
