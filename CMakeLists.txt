cmake_minimum_required(VERSION 3.20)
project(Homework1)

set(CMAKE_CXX_STANDARD 20)

# Санитайзер (AddressSanitizer + LeakSanitizer): ловит утечки и ошибки памяти
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# Папки, в которых ищутся файлы из #include "..."
include_directories(src benchmark tests)

# Готовые программы Homework1 и Tests кладутся в корень проекта: запуск ./Homework1
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Где лежит программа тестов (меню запускает её)
add_compile_definitions(TESTS_PATH="${CMAKE_SOURCE_DIR}/Tests")

# Тесты (Google Test)
find_package(GTest REQUIRED)
add_executable(Tests
        tests/test_main.cpp
        tests/test_framework.hpp
        tests/test_unq_ptr.cpp
        tests/test_shrd_ptr.cpp
        tests/test_weak_ptr.cpp
        tests/test_dynamic_array.cpp
        tests/test_leaks.cpp
)
target_link_libraries(Tests GTest::gtest)

# Основная программа (меню): тесты + бенчмарк
add_executable(Homework1
        main.cpp
        src/ControlBlock.hpp
        src/UnqPtr.hpp
        src/ShrdPtr.hpp
        src/WeakPtr.hpp
        src/DynamicArray.hpp
        benchmark/Benchmark.hpp
        benchmark/Benchmark.cpp
)
add_dependencies(Homework1 Tests)
