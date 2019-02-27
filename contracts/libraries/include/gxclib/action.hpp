/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once
#include <eosiolib/action.h>

namespace eosio {

name rootname(const name& n) {
   auto mask = (uint64_t) -1;
   for (auto i = 0; i < 12; ++i) {
      if (n.value & (0x1FULL << (4 + 5 * (11 - i))))
         continue;
      mask <<= 4 + 5 * (11 - i);
      break;
   }
   return name(n.value & mask);
}

inline bool has_vauth(const name& n) {
   return ::has_auth(rootname(n).value);
}

inline void require_vauth(const name& n) {
   ::require_auth(rootname(n).value);
}

}
