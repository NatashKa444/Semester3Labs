#ifndef HOMEWORK1_TEST_FRAMEWORK_HPP
#define HOMEWORK1_TEST_FRAMEWORK_HPP


#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

extern int g_testNumber;

template <typename T>
std::string ToText(const T& value);

std::string ShortPath(const std::string& path);

void PrintTest(const std::string& name, const std::string& input, const std::string& expected,
               const std::string& actual, bool passed, const char* file, int line);

template <typename T>
std::string ToText(const T& value) {
    std::ostringstream out;
    out << std::boolalpha << value;
    return out.str();
}

inline std::string ShortPath(const std::string& path) {
    size_t position = path.find("tests/");
    if (position == std::string::npos) {
        return path;
    }
    return path.substr(position);
}

inline void PrintTest(const std::string& name, const std::string& input,
                      const std::string& expected, const std::string& actual, bool passed,
                      const char* file, int line) {
    ++g_testNumber;
    std::cout << "Тест " << g_testNumber << ". " << name << "\n"
              << "  вход:      " << input << "\n"
              << "  ожидалось: " << expected << "\n"
              << "  получено:  " << actual << "\n"
              << "  где:       " << ShortPath(file) << ":" << line << "\n"
              << "  результат: " << (passed ? "пройден" : "НЕ ПРОЙДЕН") << "\n\n";
}

#define CHECK_EQUAL(name, input, actual, expected)                                          \
    {                                                                                       \
        auto actualValue = (actual);                                                        \
        auto expectedValue = (expected);                                                    \
        bool passed = (actualValue == expectedValue);                                       \
        PrintTest(name, input, ToText(expectedValue), ToText(actualValue), passed, __FILE__, \
                  __LINE__);                                                                \
        if (!passed) {                                                                      \
            ADD_FAILURE_AT(__FILE__, __LINE__) << name;                                     \
        }                                                                                   \
    }

#define CHECK_THROW(name, input, expression)                                              \
    {                                                                                     \
        bool thrown = false;                                                              \
        std::string message = "исключения не было";                                       \
        try {                                                                             \
            expression;                                                                   \
        } catch (const std::exception& error) {                                           \
            thrown = true;                                                                \
            message = std::string("исключение \"") + error.what() + "\"";                 \
        }                                                                                 \
        PrintTest(name, input, "исключение", message, thrown, __FILE__, __LINE__);        \
        if (!thrown) {                                                                    \
            ADD_FAILURE_AT(__FILE__, __LINE__) << name;                                   \
        }                                                                                 \
    }

struct Counter {
    static int alive;
    int value;
    Counter() : value(0) { ++alive; }
    Counter(int v) : value(v) { ++alive; }
    Counter(const Counter& other) : value(other.value) { ++alive; }
    ~Counter() { --alive; }
};

struct Animal {
    virtual ~Animal() {}
    virtual std::string Sound() const { return "..."; }
};

struct Dog : Animal {
    std::string Sound() const override { return "Гав"; }
};

struct Cat : Animal {
    std::string Sound() const override { return "Мяу"; }
};

#endif
