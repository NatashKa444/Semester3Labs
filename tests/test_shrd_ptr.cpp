// Тесты ShrdPtr. В каждом тесте одна проверка.

#include "ShrdPtr.hpp"
#include "test_framework.hpp"

TEST(ShrdPtr, EmptyHasNoOwners) {
    ShrdPtr<int> p;
    CHECK_EQUAL("У пустого ShrdPtr нет владельцев", "ShrdPtr<int> p; p.UseCount()", p.UseCount(),
                (size_t)0);
}

TEST(ShrdPtr, MakeSharedValue) {
    ShrdPtr<int> p = MakeShared<int>(7);
    CHECK_EQUAL("MakeShared создаёт объект", "MakeShared<int>(7), *p", *p, 7);
}

TEST(ShrdPtr, MakeSharedOneOwner) {
    ShrdPtr<int> p = MakeShared<int>(7);
    CHECK_EQUAL("У нового объекта один владелец", "MakeShared<int>(7), p.UseCount()", p.UseCount(),
                (size_t)1);
}

TEST(ShrdPtr, CopyIncreasesCounter) {
    ShrdPtr<int> a = MakeShared<int>(1);
    ShrdPtr<int> b = a;
    ShrdPtr<int> c = b;
    CHECK_EQUAL("Копирование увеличивает счётчик", "b = a; c = b; a.UseCount()", a.UseCount(),
                (size_t)3);
}

TEST(ShrdPtr, DestroyCopyDecreasesCounter) {
    ShrdPtr<int> a = MakeShared<int>(1);
    {
        ShrdPtr<int> b = a;
    }
    CHECK_EQUAL("Уничтожение копии уменьшает счётчик", "{ ShrdPtr<int> b = a; } a.UseCount()",
                a.UseCount(), (size_t)1);
}

TEST(ShrdPtr, CopiesShareOneObject) {
    ShrdPtr<Counter> a = MakeShared<Counter>(1);
    ShrdPtr<Counter> b = a;
    b->value = 99;
    CHECK_EQUAL("Копии указывают на один объект", "b->value = 99; a->value", a->value, 99);
}

TEST(ShrdPtr, ObjectAliveWhileOwnerLeft) {
    ShrdPtr<Counter> a = MakeShared<Counter>(1);
    ShrdPtr<Counter> b = a;
    a.Reset();
    CHECK_EQUAL("Объект жив, пока есть владелец", "b = a; a.Reset(), объектов в памяти",
                Counter::alive, 1);
}

TEST(ShrdPtr, LastOwnerDeletes) {
    ShrdPtr<Counter> a = MakeShared<Counter>(1);
    ShrdPtr<Counter> b = a;
    a.Reset();
    b.Reset();
    CHECK_EQUAL("Последний владелец удаляет объект", "a.Reset(); b.Reset(), объектов в памяти",
                Counter::alive, 0);
}

TEST(ShrdPtr, AssignReleasesOld) {
    ShrdPtr<Counter> a = MakeShared<Counter>(1);
    ShrdPtr<Counter> b = MakeShared<Counter>(2);
    b = a;
    CHECK_EQUAL("Присваивание отпускает старый объект", "b = a, объектов в памяти", Counter::alive, 1);
}

TEST(ShrdPtr, SelfAssignment) {
    ShrdPtr<Counter> a = MakeShared<Counter>(1);
    ShrdPtr<Counter>& same = a;
    a = same;
    CHECK_EQUAL("Присваивание самому себе ничего не ломает", "a = a; a.UseCount()", a.UseCount(),
                (size_t)1);
}

TEST(ShrdPtr, MoveEmptiesSource) {
    ShrdPtr<int> a = MakeShared<int>(1);
    ShrdPtr<int> b = std::move(a);
    CHECK_EQUAL("После перемещения старый указатель пустой", "b = std::move(a); a.Get() == nullptr",
                a.Get() == nullptr, true);
}

TEST(ShrdPtr, MoveKeepsCounter) {
    ShrdPtr<int> a = MakeShared<int>(1);
    ShrdPtr<int> b = a;
    ShrdPtr<int> c = std::move(b);
    CHECK_EQUAL("Перемещение не меняет счётчик", "b = a; c = std::move(b); a.UseCount()",
                a.UseCount(), (size_t)2);
}

TEST(ShrdPtr, CopyFromEmpty) {
    ShrdPtr<int> first;
    ShrdPtr<int> second = first;
    CHECK_EQUAL("Копия пустого указателя тоже пустая", "ShrdPtr<int> first; second = first; second.UseCount()",
                second.UseCount(), (size_t)0);
}

TEST(ShrdPtr, DerivedToBase) {
    ShrdPtr<Dog> dog = MakeShared<Dog>();
    ShrdPtr<Animal> animal = dog;
    CHECK_EQUAL("ShrdPtr<Dog> превращается в ShrdPtr<Animal>",
                "ShrdPtr<Animal> animal = dog; animal->Sound()", animal->Sound(), std::string("Гав"));
}

TEST(ShrdPtr, DerivedAndBaseShareCounter) {
    ShrdPtr<Dog> dog = MakeShared<Dog>();
    ShrdPtr<Animal> animal = dog;
    CHECK_EQUAL("У ShrdPtr<Dog> и ShrdPtr<Animal> общий счётчик", "animal = dog; dog.UseCount()",
                dog.UseCount(), (size_t)2);
}

TEST(ShrdPtr, DereferenceEmptyThrows) {
    ShrdPtr<int> p;
    CHECK_THROW("Разыменование пустого указателя", "ShrdPtr<int> p; *p", *p);
}
