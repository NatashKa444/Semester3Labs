#ifndef HOMEWORK1_UNQPTR_HPP
#define HOMEWORK1_UNQPTR_HPP


#include <stdexcept>
#include <utility>

template <typename T>
class UnqPtr;

template <typename T, typename... Args>
UnqPtr<T> MakeUnique(Args&&... args);

template <typename T>
class UnqPtr {
private:
    T* ptr_;

    UnqPtr(T* ptr);

    template <typename U>
    friend class UnqPtr;

    template <typename U, typename... Args>
    friend UnqPtr<U> MakeUnique(Args&&... args);

public:
    UnqPtr();
    ~UnqPtr();

    UnqPtr(const UnqPtr& other) = delete;
    UnqPtr& operator=(const UnqPtr& other) = delete;

    UnqPtr(UnqPtr&& other) noexcept;
    UnqPtr& operator=(UnqPtr&& other) noexcept;

    template <typename U>
    UnqPtr(UnqPtr<U>&& other) noexcept;

    T& operator*() const;
    T* operator->() const;
    T* Get() const;
    T* Release();
    void Reset();
};

template <typename T>
UnqPtr<T>::UnqPtr(T* ptr) : ptr_(ptr) {}

template <typename T>
UnqPtr<T>::UnqPtr() : ptr_(nullptr) {}

template <typename T>
UnqPtr<T>::~UnqPtr() {
    delete ptr_;
}

template <typename T>
UnqPtr<T>::UnqPtr(UnqPtr&& other) noexcept : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
}

template <typename T>
UnqPtr<T>& UnqPtr<T>::operator=(UnqPtr&& other) noexcept {
    if (this != &other) {
        delete ptr_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }
    return *this;
}

template <typename T>
template <typename U>
UnqPtr<T>::UnqPtr(UnqPtr<U>&& other) noexcept : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
}

template <typename T>
T& UnqPtr<T>::operator*() const {
    if (ptr_ == nullptr) {
        throw std::runtime_error("UnqPtr пустой");
    }
    return *ptr_;
}

template <typename T>
T* UnqPtr<T>::operator->() const {
    if (ptr_ == nullptr) {
        throw std::runtime_error("UnqPtr пустой");
    }
    return ptr_;
}

template <typename T>
T* UnqPtr<T>::Get() const {
    return ptr_;
}

template <typename T>
T* UnqPtr<T>::Release() {
    T* result = ptr_;
    ptr_ = nullptr;
    return result;
}

template <typename T>
void UnqPtr<T>::Reset() {
    delete ptr_;
    ptr_ = nullptr;
}

template <typename T, typename... Args>
UnqPtr<T> MakeUnique(Args&&... args) {
    return UnqPtr<T>(new T(std::forward<Args>(args)...));
}

#endif
