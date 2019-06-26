target_sources(eoslib 
   PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/fast-hash/fasthash.c
      ${CMAKE_CURRENT_SOURCE_DIR}/xxHash/xxhash.c
)

target_include_directories(eoslib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
