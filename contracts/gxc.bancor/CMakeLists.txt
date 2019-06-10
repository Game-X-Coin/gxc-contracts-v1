add_contract(gxc.bancor gxc.bancor ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.bancor.cpp)

target_include_directories(gxc.bancor
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.bancor
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
