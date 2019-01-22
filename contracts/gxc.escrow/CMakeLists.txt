add_contract(gxc.escrow gxc.escrow ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.escrow.cpp)

target_include_directories(gxc.escrow
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../gxc.token/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.escrow
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
