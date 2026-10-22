#ifndef HOMEWORK1_SHRDPTR_HPP
#define HOMEWORK1_SHRDPTR_HPP

// ShrdPtr — несколько владельцев одного объекта, счётчик в ControlBlock (аналог std::shared_ptr).

#include <stdexcept>
#include <utility>

#include "ControlBlock.hpp"

template <typename T>
class ShrdPtr;
template <typename T>
class WeakPtr;

template <typename T, typename... Args>
ShrdPtr<T> MakeShared(Args&&... args);

template <typename T>
class ShrdPtr {
private:
    T* ptr_;
    ControlBlock* block_;

    ShrdPtr(T* ptr, ControlBlock* block);

    template <typename U>
    friend class ShrdPtr;
    template <typename U>
    friend class WeakPtr;

    template <typename U, typename... Args>
    friend ShrdPtr<U> MakeShared(Args&&... args);

public:
    ShrdPtr();
    ~ShrdPtr();

    ShrdPtr(const ShrdPtr& other);
    ShrdPtr& operator=(const ShrdPtr& other);
    ShrdPtr(ShrdPtr&& other) noexcept;
    ShrdPtr& operator=(ShrdPtr&& other) noexcept;

    template <typename U>
    ShrdPtr(const ShrdPtr<U>& other);

    T& operator*() const;
    T* operator->() const;
    T* Get() const;
    size_t UseCount() const;
    void Reset();

private:
    void Release();
};

template <typename T>
ShrdPtr<T>::ShrdPtr(T* ptr, ControlBlock* block) : ptr_(ptr), block_(block) {}

template <typename T>
ShrdPtr<T>::ShrdPtr() : ptr_(nullptr), block_(nullptr) {}

template <typename T>
ShrdPtr<T>::~ShrdPtr() {
    Release();
}

template <typename T>
ShrdPtr<T>::ShrdPtr(const ShrdPtr& other) : ptr_(other.ptr_), block_(other.block_) {
    if (block_ != nullptr) {
        block_->AddStrong();
    }
}

template <typename T>
ShrdPtr<T>& ShrdPtr<T>::operator=(const ShrdPtr& other) {
    if (this != &other) {
        Release();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->AddStrong();
        }
    }
    return *this;
}

template <typename T>
ShrdPtr<T>::ShrdPtr(ShrdPtr&& other) noexcept : ptr_(other.ptr_), block_(other.block_) {
    other.ptr_ = nullptr;
    other.block_ = nullptr;
}

template <typename T>
ShrdPtr<T>& ShrdPtr<T>::operator=(ShrdPtr&& other) noexcept {
    if (this != &other) {
        Release();
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }
    return *this;
}

template <typename T>
template <typename U>
ShrdPtr<T>::ShrdPtr(const ShrdPtr<U>& other) : ptr_(other.ptr_), block_(other.block_) {
    if (block_ != nullptr) {
        block_->AddStrong();
    }
}

template <typename T>
T& ShrdPtr<T>::operator*() const {
    if (ptr_ == nullptr) {
        throw std::runtime_error("ShrdPtr пустой");
    }
    return *ptr_;
}

template <typename T>
T* ShrdPtr<T>::operator->() const {
    if (ptr_ == nullptr) {
        throw std::runtime_error("ShrdPtr пустой");
    }
    return ptr_;
}

template <typename T>
T* ShrdPtr<T>::Get() const {
    return ptr_;
}

template <typename T>
size_t ShrdPtr<T>::UseCount() const {
    if (block_ == nullptr) {
        return 0;
    }
    return block_->StrongCount();
}

template <typename T>
void ShrdPtr<T>::Reset() {
    Release();
}

template <typename T>
void ShrdPtr<T>::Release() {
    if (block_ != nullptr) {
        block_->ReleaseStrong();
    }
    ptr_ = nullptr;
    block_ = nullptr;
}

template <typename T, typename... Args>
ShrdPtr<T> MakeShared(Args&&... args) {
    T* object = new T(std::forward<Args>(args)...);
    ControlBlock* block = new ControlBlock(object);
    return ShrdPtr<T>(object, block);
}

#endif
