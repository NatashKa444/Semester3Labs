#ifndef HOMEWORK1_CONTROLBLOCK_HPP
#define HOMEWORK1_CONTROLBLOCK_HPP

// Блок управления для ShrdPtr/WeakPtr: адрес объекта, счётчики владельцев и функция удаления.

#include <cstddef>

template <typename T>
void DeleteObject(void* object);

class ControlBlock {
private:
    void* ptr_;
    size_t strong_;
    size_t weak_;
    void (*deleter_)(void*);

public:
    template <typename T>
    ControlBlock(T* object);

    void AddStrong();
    void ReleaseStrong();
    void AddWeak();
    void ReleaseWeak();
    bool TryAddStrong();
    size_t StrongCount() const;
};

template <typename T>
void DeleteObject(void* object) {
    delete static_cast<T*>(object);
}

template <typename T>
ControlBlock::ControlBlock(T* object)
    : ptr_(object), strong_(1), weak_(1), deleter_(&DeleteObject<T>) {}

inline void ControlBlock::AddStrong() {
    ++strong_;
}

inline void ControlBlock::ReleaseStrong() {
    --strong_;
    if (strong_ == 0) {
        deleter_(ptr_);
        ptr_ = nullptr;
        ReleaseWeak();
    }
}

inline void ControlBlock::AddWeak() {
    ++weak_;
}

inline void ControlBlock::ReleaseWeak() {
    --weak_;
    if (weak_ == 0) {
        delete this;
    }
}

inline bool ControlBlock::TryAddStrong() {
    if (strong_ == 0) {
        return false;
    }
    ++strong_;
    return true;
}

inline size_t ControlBlock::StrongCount() const {
    return strong_;
}

#endif
