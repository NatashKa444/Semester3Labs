#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

#include "Benchmark.hpp"
#include "DynamicArray.hpp"
#include "ShrdPtr.hpp"
#include "UnqPtr.hpp"
#include "WeakPtr.hpp"

#ifndef TESTS_PATH
#define TESTS_PATH "./Tests"
#endif

void RunAllTests() {
    std::string command = std::string(TESTS_PATH);
    std::system(command.c_str());
}

int main() {
    int* p = new int[100]; //пример для санитайзера

    
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n===== Homework1: умные указатели =====\n"
                  << "1. Запустить все тесты\n"
                  << "2. Бенчмарк (скорость и память)\n"
                  << "0. Выход\n"
                  << "Выберите пункт: ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                break;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Ошибка: нужно ввести число 0, 1 или 2.\n";
            choice = -1;
            continue;
        }
        if (choice == 1) {
            RunAllTests();
        } else if (choice == 2) {
            RunBenchmark(1000000);
        } else if (choice != 0) {
            std::cout << "Ошибка: нет такого пункта. Введите 0, 1 или 2.\n";
        }
    }
    return 0;
}
