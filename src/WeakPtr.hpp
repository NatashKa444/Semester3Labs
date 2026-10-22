#ifndef HOMEWORK1_WEAKPTR_HPP
#define HOMEWORK1_WEAKPTR_HPP


#include "ControlBlock.hpp"
#include "ShrdPtr.hpp"

template <typename T>
class WeakPtr {
private:
    T* ptr_;
    ControlBlock* block_;

public:
    WeakPtr();
    WeakPtr(const ShrdPtr<T>& shared);
    ~WeakPtr();

    WeakPtr(const WeakPtr& other);
    WeakPtr& operator=(const WeakPtr& other);
    WeakPtr& operator=(const ShrdPtr<T>& shared);

    bool Expired() const;
    ShrdPtr<T> Lock() const;
    size_t UseCount() const;
    void Reset();

private:
    void Release();
};

template <typename T>
WeakPtr<T>::WeakPtr() : ptr_(nullptr), block_(nullptr) {}

template <typename T>
WeakPtr<T>::WeakPtr(const ShrdPtr<T>& shared) : ptr_(shared.ptr_), block_(shared.block_) {
    if (block_ != nullptr) {
        block_->AddWeak();
    }
}

template <typename T>
WeakPtr<T>::~WeakPtr() {
    Release();
}

template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(other.block_) {
    if (block_ != nullptr) {
        block_->AddWeak();
    }
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {
    if (this != &other) {
        Release();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->AddWeak();
        }
    }
    return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const ShrdPtr<T>& shared) {
    Release();
    ptr_ = shared.ptr_;
    block_ = shared.block_;
    if (block_ != nullptr) {
        block_->AddWeak();
    }
    return *this;
}

template <typename T>
bool WeakPtr<T>::Expired() const {
    return block_ == nullptr || block_->StrongCount() == 0;
}

template <typename T>
ShrdPtr<T> WeakPtr<T>::Lock() const {
    if (block_ != nullptr && block_->TryAddStrong()) {
        return ShrdPtr<T>(ptr_, block_);
    }
    return ShrdPtr<T>();
}

template <typename T>
size_t WeakPtr<T>::UseCount() const {
    if (block_ == nullptr) {
        return 0;
    }
    return block_->StrongCount();
}

template <typename T>
void WeakPtr<T>::Reset() {
    Release();
}

template <typename T>
void WeakPtr<T>::Release() {
    if (block_ != nullptr) {
        block_->ReleaseWeak();
    }
    ptr_ = nullptr;
    block_ = nullptr;
}

#endif
