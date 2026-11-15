cmake_minimum_required(VERSION 3.14)
project(Laba1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(include)

find_package(GTest REQUIRED)

# программа с меню и замерами: с оптимизацией, без санитайзера (иначе замеры в разы медленнее)
add_executable(Laba1 src/main.cpp src/Benchmark.cpp)
target_compile_options(Laba1 PRIVATE -Wall -Wextra -O2)

# тесты: с AddressSanitizer и UndefinedBehaviorSanitizer, они ловят утечки и ошибки памяти
add_executable(Tests src/tests.cpp)
target_compile_options(Tests PRIVATE -Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer)
target_link_options(Tests PRIVATE -fsanitize=address,undefined)
target_link_libraries(Tests GTest::gtest)
