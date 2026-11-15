#pragma once

#include <string>

#include "Exceptions.hpp"

template <class T>
class DynamicArray {
private:
    T* data;
    int size;

    void copyFrom(const DynamicArray<T>& other);
    void free();

public:
    DynamicArray(T* items, int count);
    explicit DynamicArray(int size = 0);
    DynamicArray(const DynamicArray<T>& other);
    DynamicArray<T>& operator=(const DynamicArray<T>& other);
    ~DynamicArray();

    T Get(int index) const;
    int GetSize() const;
    void Set(int index, T value);
    void Resize(int newSize);

    T& operator[](int index);
    const T& operator[](int index) const;
};

// ------------------ реализация ------------------

template <class T>
void DynamicArray<T>::copyFrom(const DynamicArray<T>& other) {
    size = other.size;
    data = new T[size];
    for (int i = 0; i < size; ++i)
        data[i] = other.data[i];
}

template <class T>
void DynamicArray<T>::free() {
    delete[] data;
}

template <class T>
DynamicArray<T>::DynamicArray(T* items, int count) {
    if (count < 0) throw std::invalid_argument("Размер массива не может быть отрицательным");
    size = count;
    data = new T[size];
    for (int i = 0; i < size; ++i)
        data[i] = items[i];
}

template <class T>
DynamicArray<T>::DynamicArray(int size) : size(size) {
    if (size < 0) throw std::invalid_argument("Размер массива не может быть отрицательным");
    data = new T[size]();
}

template <class T>
DynamicArray<T>::DynamicArray(const DynamicArray<T>& other) {
    copyFrom(other);
}

template <class T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T>& other) {
    if (this != &other) {
        free();
        copyFrom(other);
    }
    return *this;
}

template <class T>
DynamicArray<T>::~DynamicArray() {
    free();
}

template <class T>
T DynamicArray<T>::Get(int index) const {
    if (index < 0 || index >= size)
        throw IndexOutOfRange("Индекс вне диапазона");
    return data[index];
}

template <class T>
int DynamicArray<T>::GetSize() const {
    return size;
}

template <class T>
void DynamicArray<T>::Set(int index, T value) {
    if (index < 0 || index >= size)
        throw IndexOutOfRange("Индекс вне диапазона");
    data[index] = value;
}

template <class T>
void DynamicArray<T>::Resize(int newSize) {
    if (newSize < 0) throw std::invalid_argument("Новый размер массива не может быть отрицательным");
    T* newData = new T[newSize]();
    int copyCount = (newSize < size) ? newSize : size;
    for (int i = 0; i < copyCount; ++i)
        newData[i] = data[i];
    delete[] data;
    data = newData;
    size = newSize;
}

template <class T>
T& DynamicArray<T>::operator[](int index) {
    if (index < 0 || index >= size)
        throw IndexOutOfRange("Индекс вне диапазона");
    return data[index];
}

template <class T>
const T& DynamicArray<T>::operator[](int index) const {
    if (index < 0 || index >= size)
        throw IndexOutOfRange("Индекс вне диапазона");
    return data[index];
}
