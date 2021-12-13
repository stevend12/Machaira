# This is the CMakeLists file for the wxSwordProject program
cmake_minimum_required(VERSION 3.0.0)

project(Machaira)

# wxWidgets - UI
find_package(wxWidgets REQUIRED COMPONENTS html net core base adv)
include(${wxWidgets_USE_FILE})

# Set compiler flags
set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++17 -Wno-deprecated-declarations")
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
  set(GCC_COVERAGE_COMPILE_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} \
    -L/usr/local/lib/ -I/usr/local/include/sword/")
endif()
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
  set(GCC_COVERAGE_COMPILE_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} \
    -I C:/Users/sdolly/msys64/mingw64/include/sword/")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

# Copy relevant resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Res DESTINATION ${CMAKE_BINARY_DIR})

# Add sources and headers
set(SOURCE
  ${SOURCE}
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/SwordBackend.cpp
)
set(HEADERS
  ${HEADERS}
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/SwordBackend.hpp
)

add_executable(Machaira ${SOURCE} ${HEADERS})
target_link_libraries(Machaira ${wxWidgets_LIBRARIES} sword)
