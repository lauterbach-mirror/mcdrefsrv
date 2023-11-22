# CMakeList.txt : CMake project for mcd_shared_library, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source for the shared library.
add_library (mcd_shared_library SHARED "mcdlib.cpp" "mcdlib.h" "helpers.cpp" "mcd_types.h" "mcd_api.h" "mcd_shared_defines.h")
