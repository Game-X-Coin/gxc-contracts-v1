add_contract(@CONTRACT @CONTRACT ${CMAKE_CURRENT_SOURCE_DIR}/src/@CONTRACT.cpp)
target_include_directories(@CONTRACT.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(@CONTRACT.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
