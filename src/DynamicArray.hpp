#ifndef HOMEWORK1_DYNAMICARRAY_HPP
#define HOMEWORK1_DYNAMICARRAY_HPP


#include <stdexcept>
#include <utility>

#include "UnqPtr.hpp"

template <typename T>
class DynamicArray {
private:
    UnqPtr<T>* items_;
    size_t size_;

public:
    DynamicArray();
    DynamicArray(size_t size);
    ~DynamicArray();

    DynamicArray(const DynamicArray& other) = delete;
    DynamicArray& operator=(const DynamicArray& other) = delete;

    const T& Get(size_t index) const;
    void Set(size_t index, const T& value);
    size_t GetSize() const;
    void Append(const T& value);
};

template <typename T>
DynamicArray<T>::DynamicArray() : items_(nullptr), size_(0) {}

template <typename T>
DynamicArray<T>::DynamicArray(size_t size) : items_(new UnqPtr<T>[size]), size_(size) {
    for (size_t i = 0; i < size_; ++i) {
        items_[i] = MakeUnique<T>();
    }
}

template <typename T>
DynamicArray<T>::~DynamicArray() {
    delete[] items_;
}

template <typename T>
const T& DynamicArray<T>::Get(size_t index) const {
    if (index >= size_) {
        throw std::out_of_range("DynamicArray: индекс вне массива");
    }
    return *items_[index];
}

template <typename T>
void DynamicArray<T>::Set(size_t index, const T& value) {
    if (index >= size_) {
        throw std::out_of_range("DynamicArray: индекс вне массива");
    }
    items_[index] = MakeUnique<T>(value);
}

template <typename T>
size_t DynamicArray<T>::GetSize() const {
    return size_;
}

template <typename T>
void DynamicArray<T>::Append(const T& value) {
    UnqPtr<T>* bigger = new UnqPtr<T>[size_ + 1];
    for (size_t i = 0; i < size_; ++i) {
        bigger[i] = std::move(items_[i]);
    }
    bigger[size_] = MakeUnique<T>(value);
    delete[] items_;
    items_ = bigger;
    ++size_;
}

#endif
