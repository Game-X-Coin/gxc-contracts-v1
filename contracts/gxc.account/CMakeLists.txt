add_contract(gxc.account gxc.account ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.account.cpp)

target_include_directories(gxc.account
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.account
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
