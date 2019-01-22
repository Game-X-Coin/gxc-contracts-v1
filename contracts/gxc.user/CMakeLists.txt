add_contract(gxc.user gxc.user ${CMAKE_CURRENT_SOURCE_DIR}/src/gxc.user.cpp)

target_include_directories(gxc.user
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/../libraries/include
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(gxc.user
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
