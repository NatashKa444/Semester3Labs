// Тесты UnqPtr. В каждом тесте одна проверка.

#include "UnqPtr.hpp"
#include "test_framework.hpp"

TEST(UnqPtr, EmptyByDefault) {
    UnqPtr<int> p;
    CHECK_EQUAL("UnqPtr по умолчанию пустой", "UnqPtr<int> p; p.Get() == nullptr",
                p.Get() == nullptr, true);
}

TEST(UnqPtr, MakeUniqueInt) {
    UnqPtr<int> p = MakeUnique<int>(42);
    CHECK_EQUAL("MakeUnique создаёт int", "MakeUnique<int>(42), *p", *p, 42);
}

TEST(UnqPtr, MakeUniqueDouble) {
    UnqPtr<double> p = MakeUnique<double>(3.5);
    CHECK_EQUAL("MakeUnique создаёт double", "MakeUnique<double>(3.5), *p", *p, 3.5);
}

TEST(UnqPtr, MakeUniqueString) {
    UnqPtr<std::string> p = MakeUnique<std::string>(3, 'x');
    CHECK_EQUAL("MakeUnique передаёт аргументы в конструктор", "MakeUnique<std::string>(3, 'x'), *p",
                *p, std::string("xxx"));
}

TEST(UnqPtr, DeletedAtEndOfBlock) {
    {
        UnqPtr<Counter> p = MakeUnique<Counter>(1);
    }
    CHECK_EQUAL("Объект удаляется при выходе из блока", "{ UnqPtr<Counter> p = ...; }, объектов в памяти",
                Counter::alive, 0);
}

TEST(UnqPtr, ResetDeletesObject) {
    UnqPtr<Counter> p = MakeUnique<Counter>(1);
    p.Reset();
    CHECK_EQUAL("Reset удаляет объект", "p.Reset(), объектов в памяти", Counter::alive, 0);
}

TEST(UnqPtr, ResetMakesEmpty) {
    UnqPtr<int> p = MakeUnique<int>(1);
    p.Reset();
    CHECK_EQUAL("После Reset указатель пустой", "p.Reset(); p.Get() == nullptr", p.Get() == nullptr,
                true);
}

TEST(UnqPtr, ReleaseMakesEmpty) {
    UnqPtr<int> p = MakeUnique<int>(5);
    int* raw = p.Release();
    bool empty = (p.Get() == nullptr);
    delete raw;
    CHECK_EQUAL("После Release указатель пустой", "int* raw = p.Release(); p.Get() == nullptr",
                empty, true);
}

TEST(UnqPtr, ReleaseKeepsObjectAlive) {
    UnqPtr<Counter> p = MakeUnique<Counter>(5);
    Counter* raw = p.Release();
    int aliveAfterRelease = Counter::alive;
    delete raw;
    CHECK_EQUAL("Release не удаляет объект", "Counter* raw = p.Release(), объектов в памяти",
                aliveAfterRelease, 1);
}

TEST(UnqPtr, NoLeakOnException) {
    try {
        UnqPtr<Counter> p = MakeUnique<Counter>(1);
        throw std::runtime_error("ошибка");
    } catch (const std::runtime_error&) {
    }
    CHECK_EQUAL("При исключении объект всё равно удаляется", "throw внутри блока, объектов в памяти",
                Counter::alive, 0);
}

TEST(UnqPtr, MoveEmptiesSource) {
    UnqPtr<int> a = MakeUnique<int>(10);
    UnqPtr<int> b = std::move(a);
    CHECK_EQUAL("После перемещения старый указатель пустой", "b = std::move(a); a.Get() == nullptr",
                a.Get() == nullptr, true);
}

TEST(UnqPtr, MoveTransfersObject) {
    UnqPtr<int> a = MakeUnique<int>(10);
    UnqPtr<int> b = std::move(a);
    CHECK_EQUAL("После перемещения объект у нового указателя", "b = std::move(a); *b", *b, 10);
}

TEST(UnqPtr, MoveAssignmentDeletesOld) {
    UnqPtr<Counter> a = MakeUnique<Counter>(1);
    UnqPtr<Counter> b = MakeUnique<Counter>(2);
    b = std::move(a);
    CHECK_EQUAL("Перемещающее присваивание удаляет старый объект b",
                "b = std::move(a), объектов в памяти", Counter::alive, 1);
}

TEST(UnqPtr, UseAfterMoveThrows) {
    UnqPtr<int> a = MakeUnique<int>(1);
    UnqPtr<int> b = std::move(a);
    CHECK_THROW("Использование указателя после перемещения", "b = std::move(a); *a", *a);
}

TEST(UnqPtr, DerivedToBase) {
    UnqPtr<Animal> animal = MakeUnique<Dog>();
    CHECK_EQUAL("UnqPtr<Dog> превращается в UnqPtr<Animal>",
                "UnqPtr<Animal> animal = MakeUnique<Dog>(); animal->Sound()", animal->Sound(),
                std::string("Гав"));
}

TEST(UnqPtr, ReplaceDerived) {
    UnqPtr<Animal> animal = MakeUnique<Dog>();
    animal = MakeUnique<Cat>();
    CHECK_EQUAL("Собаку заменили на кошку", "animal = MakeUnique<Cat>(); animal->Sound()",
                animal->Sound(), std::string("Мяу"));
}

TEST(UnqPtr, DereferenceEmptyThrows) {
    UnqPtr<int> p;
    CHECK_THROW("Разыменование пустого указателя", "UnqPtr<int> p; *p", *p);
}

TEST(UnqPtr, ArrowOnEmptyThrows) {
    UnqPtr<Counter> p;
    CHECK_THROW("Оператор -> у пустого указателя", "UnqPtr<Counter> p; p->value", p->value);
}
