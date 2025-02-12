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

add_library (qmp "${CMAKE_CURRENT_LIST_DIR}/src/qmp.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/comm_qmp.cpp")
target_include_directories (qmp PUBLIC include)
target_compile_features (qmp PUBLIC cxx_std_20)
set_target_properties (qmp PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library (rpc "${CMAKE_CURRENT_LIST_DIR}/src/mcd_rpc.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/comm_rpc.cpp")
target_include_directories (rpc PUBLIC include)
target_compile_features (rpc PUBLIC cxx_std_20)
set_target_properties (rpc PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library (mcd_client_stub SHARED "${CMAKE_CURRENT_LIST_DIR}/src/mcdstub.cpp")
target_compile_features (mcd_client_stub PUBLIC cxx_std_20)
# QMP support:
target_link_libraries (mcd_client_stub PRIVATE comm adapter adapter_passthrough qmp)
# RPC support:
# target_link_libraries (mcd_client_stub PRIVATE comm adapter adapter_passthrough rpc)
