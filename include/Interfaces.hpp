#pragma once

#include <cstddef>
#include <memory>

template <class T>
class ICollection {
public:
    virtual ~ICollection() = default;

    virtual std::size_t GetCount() const = 0;
    virtual bool IsEmpty() const = 0;
    virtual bool Add(const T& value) = 0;
    virtual bool Contains(const T& value) const = 0;
    virtual void Remove(const T& value) = 0;
    virtual void Clear() = 0;
};

template <class T>
class IEnumerator {
public:
    virtual ~IEnumerator() = default;

    virtual bool Next() = 0;
    virtual bool HasNext() const = 0;
    virtual const T& GetCurrentItem() const = 0;
    virtual bool TryGetCurrentItem(T& item) const = 0;
    virtual void Reset() = 0;
};

template <class T>
class IEnumerable {
public:
    virtual ~IEnumerable() = default;

    virtual std::unique_ptr<IEnumerator<T>> GetEnumerator() const = 0;
};
