cmake_minimum_required (VERSION 3.8)

project (mcd_client_stub)

if (MSVC)
    add_compile_options(/W4)
endif()

add_library (comm "${CMAKE_CURRENT_LIST_DIR}/src/comm.cpp")
target_include_directories (comm PUBLIC include)
target_compile_features (comm PUBLIC cxx_std_20)
set_target_properties (comm PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library (adapter "${CMAKE_CURRENT_LIST_DIR}/src/adapter.cpp")
target_include_directories (adapter PUBLIC include)
target_compile_features (adapter PUBLIC cxx_std_20)
set_target_properties (adapter PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library (adapter_passthrough "${CMAKE_CURRENT_LIST_DIR}/src/core_passthrough.cpp")
target_include_directories (adapter_passthrough PUBLIC include)
target_compile_features (adapter_passthrough PUBLIC cxx_std_20)
set_target_properties (adapter_passthrough PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library (mcd_client_stub SHARED "${CMAKE_CURRENT_LIST_DIR}/src/mcdstub.cpp")
target_compile_features (mcd_client_stub PUBLIC cxx_std_20)
target_link_libraries (mcd_client_stub PRIVATE comm adapter adapter_passthrough)
