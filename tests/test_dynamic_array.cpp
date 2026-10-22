// Тесты DynamicArray. В каждом тесте одна проверка.

#include "DynamicArray.hpp"
#include "test_framework.hpp"

TEST(DynamicArray, SizeAfterCreate) {
    DynamicArray<int> arr(3);
    CHECK_EQUAL("Размер нового массива", "DynamicArray<int> arr(3); arr.GetSize()", arr.GetSize(),
                (size_t)3);
}

TEST(DynamicArray, NewElementsAreZero) {
    DynamicArray<int> arr(3);
    CHECK_EQUAL("Новые элементы равны 0", "DynamicArray<int> arr(3); arr.Get(0)", arr.Get(0), 0);
}

TEST(DynamicArray, SetAndGet) {
    DynamicArray<int> arr(3);
    arr.Set(1, 20);
    CHECK_EQUAL("Set записывает элемент", "arr.Set(1, 20); arr.Get(1)", arr.Get(1), 20);
}

TEST(DynamicArray, AppendIncreasesSize) {
    DynamicArray<int> arr(1);
    arr.Append(20);
    CHECK_EQUAL("Append увеличивает размер", "arr(1); arr.Append(20); arr.GetSize()", arr.GetSize(),
                (size_t)2);
}

TEST(DynamicArray, AppendKeepsOldElements) {
    DynamicArray<int> arr(1);
    arr.Set(0, 10);
    arr.Append(20);
    CHECK_EQUAL("Append не теряет старые элементы", "arr = {10}; arr.Append(20); arr.Get(0)",
                arr.Get(0), 10);
}

TEST(DynamicArray, AppendAddsToEnd) {
    DynamicArray<int> arr(1);
    arr.Append(20);
    CHECK_EQUAL("Append добавляет в конец", "arr.Append(20); arr.Get(1)", arr.Get(1), 20);
}

TEST(DynamicArray, GetOutOfRangeThrows) {
    DynamicArray<int> arr(2);
    CHECK_THROW("Get за границей массива", "DynamicArray<int> arr(2); arr.Get(5)", arr.Get(5));
}

TEST(DynamicArray, SetOutOfRangeThrows) {
    DynamicArray<int> arr(2);
    CHECK_THROW("Set за границей массива", "DynamicArray<int> arr(2); arr.Set(5, 1)", arr.Set(5, 1));
}

TEST(DynamicArray, NoLeak) {
    {
        DynamicArray<Counter> arr(10);
        arr.Append(Counter(5));
    }
    CHECK_EQUAL("Все элементы удаляются вместе с массивом", "{ DynamicArray<Counter> arr(10); Append }, объектов в памяти",
                Counter::alive, 0);
}
