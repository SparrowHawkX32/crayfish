cmake_minimum_required(VERSION 3.11)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  message(STATUS "Building in Debug mode")
  add_compile_options(-Werror -Wall -Wextra -fsanitize=undefined,address)
endif()

project(crayfish LANGUAGES C)

add_executable(crayfish src/main.c src/util.c src/level.c)

target_include_directories(crayfish PUBLIC src)
target_link_libraries(crayfish PUBLIC raylib m) # MATH
