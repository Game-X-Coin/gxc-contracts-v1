add_contract(gxc.token gxc.token ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.token.cpp)

target_include_directories(gxc.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
