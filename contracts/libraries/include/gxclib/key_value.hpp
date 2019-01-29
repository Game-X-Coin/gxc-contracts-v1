/**
 * @file
 * @copyright defined in gxc/LICENSE
 */
#pragma once

#include <string>

namespace gxc {
   struct key_value {
      std::string         key;
      std::vector<int8_t> value; 

      EOSLIB_SERIALIZE( key_value, (key)(value) )
   };
}
