// Запуск всех тестов Google Test и вывод итога.

#include <gtest/gtest.h>

#include <iostream>

#include "test_framework.hpp"

int g_testNumber = 0;
int Counter::alive = 0;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    delete listeners.Release(listeners.default_result_printer());

    int result = RUN_ALL_TESTS();

    const ::testing::UnitTest* tests = ::testing::UnitTest::GetInstance();
    std::cout << "==============================\n"
              << "Всего тестов: " << tests->test_to_run_count() << "\n"
              << "Пройдено:     " << tests->successful_test_count() << "\n"
              << "Не пройдено:  " << tests->failed_test_count() << "\n"
              << "==============================\n";
    return result;
}
