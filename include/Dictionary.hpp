#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <utility>

#include "AvlTree.hpp"
#include "DynamicArray.hpp"
#include "Exceptions.hpp"

template <class TKey, class TValue>
struct KeyValuePair {
    TKey key{};
    TValue value{};
};

template <class TKey, class TValue>
std::ostream& operator<<(std::ostream& out, const KeyValuePair<TKey, TValue>& pair) {
    return out << pair.key << ": " << pair.value;
}

template <class TKey, class TValue>
class Dictionary {
public:
    using KeyComparator = std::function<int(const TKey&, const TKey&)>;
    using Pair = KeyValuePair<TKey, TValue>;

    Dictionary(KeyComparator compareKeys = DefaultCompare<TKey>)
        : tree_(MakePairComparator(std::move(compareKeys))) {}

    std::size_t GetCount() const { return tree_.GetCount(); }
    bool IsEmpty() const { return tree_.IsEmpty(); }
    int GetHeight() const { return tree_.GetHeight(); }

    bool ContainsKey(const TKey& key) const { return tree_.Contains(Probe(key)); }

    const TValue& Get(const TKey& key) const {
        const Pair* pair = tree_.Find(Probe(key));
        if (pair == nullptr) {
            throw KeyNotFound("В словаре нет такого ключа");
        }
        return pair->value;
    }

    bool TryGet(const TKey& key, TValue& value) const {
        const Pair* pair = tree_.Find(Probe(key));
        if (pair == nullptr) {
            return false;
        }
        value = pair->value;
        return true;
    }

    bool Add(const TKey& key, const TValue& value) { return tree_.AddOrReplace(Pair{key, value}); }

    void Remove(const TKey& key) {
        if (!ContainsKey(key)) {
            throw KeyNotFound("Удаление: в словаре нет такого ключа");
        }
        tree_.Remove(Probe(key));
    }

    void Clear() { tree_.Clear(); }

    DynamicArray<TKey> GetKeys() const {
        DynamicArray<Pair> pairs = tree_.ToArray();
        DynamicArray<TKey> keys(pairs.GetSize());
        for (int i = 0; i < pairs.GetSize(); ++i) {
            keys[i] = pairs[i].key;
        }
        return keys;
    }

    DynamicArray<TValue> GetValues() const {
        DynamicArray<Pair> pairs = tree_.ToArray();
        DynamicArray<TValue> values(pairs.GetSize());
        for (int i = 0; i < pairs.GetSize(); ++i) {
            values[i] = pairs[i].value;
        }
        return values;
    }

    std::unique_ptr<IEnumerator<Pair>> GetEnumerator() const { return tree_.GetEnumerator(); }

    void PrintTree(std::ostream& out) const { tree_.Print(out); }

    friend std::ostream& operator<<(std::ostream& out, const Dictionary& dictionary) {
        return out << dictionary.tree_;
    }

private:
    AvlTree<Pair> tree_;

    static Pair Probe(const TKey& key) { return Pair{key, TValue{}}; }

    // compareKeys принимаем ПО ЗНАЧЕНИЮ (это уже своя копия) и переносим её в лямбду через std::move:
    // лямбда хранит компаратор у себя, поэтому ссылка на чужой объект тут не нужна, а лишней копии нет.
    static typename AvlTree<Pair>::Comparator MakePairComparator(KeyComparator compareKeys) {
        if (!compareKeys) {
            throw TreeException("Компаратор ключей не задан");
        }
        return [compareKeys = std::move(compareKeys)](const Pair& a, const Pair& b) {
            return compareKeys(a.key, b.key);
        };
    }
};
