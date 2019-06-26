/**
 * @file
 */
#pragma once
#include <cstdint>

namespace eosio {

   /**
    * Hashes `data` using fasthash32.
    * @brief Hashes `data` using fasthash32.
    *
    * @param data - Data you want to hash
    * @param length - Data length
    * @param seed - Hash seed
    * @return uint32_t - Computed value
    */
   [[deprecated("use xxh32")]]
   uint32_t fasthash32(const char* data, uint32_t length, uint32_t seed = 0);

   /**
    * Hashes `data` using fasthash64.
    * @brief Hashes `data` using fasthash64.
    *
    * @param data - Data you want to hash
    * @param length - Data length
    * @param seed - Hash seed
    * @return uint64_t - Computed value
    */
   [[deprecated("use xxh64")]]
   uint64_t fasthash64(const char* data, uint32_t length, uint64_t seed = 0);

   /**
    * Hashes `data` using xxHash v3.
    * @brief Hashes `data` using xxHash v3.
    *
    * @param data - Data you want to hash
    * @param length - Data length
    * @param seed - Hash seed
    * @return uint64_t - Computed value
    */
   uint64_t xxh3_64(const char* data, uint32_t length, uint64_t seed = 0);

   /**
    * Hashes `data` using xxHash32
    * @brief Hashes `data` using xxHash32
    *
    * @param data - Data you want to hash
    * @param length - Data length
    * @param seed - Hash seed
    * @return uint32_t - Computed value
    */
   uint32_t xxh32(const char* data, uint32_t length, uint32_t seed = 0);

   /**
    * Hashes `data` using xxHash64
    * @brief Hashes `data` using xxHash64
    *
    * @param data - Data you want to hash
    * @param length - Data length
    * @param seed - Hash seed
    * @return uint64_t - Computed value
    */
   uint64_t xxh64(const char* data, uint32_t length, uint64_t seed = 0);
}
