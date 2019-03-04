add_contract(gxc.reserve gxc.reserve ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.reserve.cpp)

target_include_directories(gxc.reserve
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../gxc.token/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.reserve
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
