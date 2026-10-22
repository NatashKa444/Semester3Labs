// Тесты на утечки: счётчик Counter::alive должен вернуться к 0; дополнительно проверяет санитайзер.

#include "DynamicArray.hpp"
#include "ShrdPtr.hpp"
#include "UnqPtr.hpp"
#include "WeakPtr.hpp"
#include "test_framework.hpp"

TEST(Leaks, ManyUnqPtr) {
    {
        UnqPtr<Counter> items[1000];
        for (int i = 0; i < 1000; ++i) {
            items[i] = MakeUnique<Counter>(i);
        }
    }
    CHECK_EQUAL("1000 UnqPtr создаются и удаляются", "UnqPtr<Counter> items[1000], объектов в памяти после блока",
                Counter::alive, 0);
}

TEST(Leaks, ManyShrdPtrCopiesCounter) {
    ShrdPtr<Counter> origin = MakeShared<Counter>(1);
    ShrdPtr<Counter> copies[1000];
    for (int i = 0; i < 1000; ++i) {
        copies[i] = origin;
    }
    CHECK_EQUAL("1000 копий одного ShrdPtr", "copies[i] = origin; origin.UseCount()",
                origin.UseCount(), (size_t)1001);
}

TEST(Leaks, ManyShrdPtrCopiesNoLeak) {
    {
        ShrdPtr<Counter> origin = MakeShared<Counter>(1);
        ShrdPtr<Counter> copies[1000];
        for (int i = 0; i < 1000; ++i) {
            copies[i] = origin;
        }
    }
    CHECK_EQUAL("1000 копий ShrdPtr: объект удалён один раз", "все копии ушли, объектов в памяти", Counter::alive,
                0);
}

TEST(Leaks, ReassignManyTimes) {
    ShrdPtr<Counter> shared;
    UnqPtr<Counter> unique;
    for (int i = 0; i < 1000; ++i) {
        shared = MakeShared<Counter>(i);
        unique = MakeUnique<Counter>(i);
    }
    CHECK_EQUAL("Переприсваивание 1000 раз: старые объекты удаляются",
                "shared = MakeShared, unique = MakeUnique (1000 раз), объектов в памяти", Counter::alive, 2);
}

TEST(Leaks, ExceptionInside) {
    try {
        UnqPtr<Counter> a = MakeUnique<Counter>(1);
        ShrdPtr<Counter> b = MakeShared<Counter>(2);
        DynamicArray<Counter> c(5);
        throw std::runtime_error("ошибка");
    } catch (const std::runtime_error&) {
    }
    CHECK_EQUAL("Исключение посреди работы: всё удалено", "throw при живых UnqPtr, ShrdPtr, DynamicArray",
                Counter::alive, 0);
}

TEST(Leaks, WeakOutlivesObject) {
    WeakPtr<Counter> weak;
    {
        ShrdPtr<Counter> shared = MakeShared<Counter>(1);
        weak = shared;
    }
    CHECK_EQUAL("WeakPtr пережил объект: объект удалён", "{ shared } закончился, weak остался, объектов в памяти",
                Counter::alive, 0);
}
