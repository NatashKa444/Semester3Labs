#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "DynamicArray.hpp"
#include "Exceptions.hpp"
#include "Interfaces.hpp"
#include "ValueCodec.hpp"

template <class T>
int DefaultCompare(const T& a, const T& b) {
    if (a < b) {
        return -1;
    }
    if (b < a) {
        return 1;
    }
    return 0;
}

template <class T>
class AvlTree : public ICollection<T>, public IEnumerable<T> {
private:
    struct Node;
    using NodePtr = std::unique_ptr<Node>;

    struct Node {
        T value;
        int height = 1;
        NodePtr left;
        NodePtr right;
        Node(const T& v) : value(v) {}
    };

public:
    using Comparator = std::function<int(const T&, const T&)>;

    AvlTree(Comparator compare = DefaultCompare<T>);
    AvlTree(std::initializer_list<T> items, Comparator compare = DefaultCompare<T>);
    AvlTree(const AvlTree& other);
    AvlTree(AvlTree&& other);
    AvlTree& operator=(AvlTree other);
    ~AvlTree() = default;

    std::size_t GetCount() const override;
    bool IsEmpty() const override;
    int GetHeight() const;
    const T& GetRoot() const;
    bool IsBalanced() const;

    bool Add(const T& value) override;
    bool AddOrReplace(const T& value);
    bool Contains(const T& value) const override;
    const T* Find(const T& value) const;
    bool TryFind(const T& value, T& found) const;
    void Remove(const T& value) override;
    void Clear() override;

    class TreeEnumerator;
    std::unique_ptr<IEnumerator<T>> GetEnumerator() const override;

    DynamicArray<T> ToArray() const;
    void Traverse(const std::string& order, const std::function<void(const T&)>& action) const;

    AvlTree Map(const std::function<T(const T&)>& f) const;
    AvlTree Where(const std::function<bool(const T&)>& condition) const;
    T Reduce(const std::function<T(const T&, const T&)>& f, const T& start) const;

    AvlTree ExtractSubtree(const T& value) const;
    bool ContainsSubtree(const AvlTree& sub) const;

    std::string Serialize() const;
    static AvlTree Deserialize(const std::string& text, Comparator compare = DefaultCompare<T>);

    std::string Serialize(const std::string& format) const;
    static AvlTree Deserialize(const std::string& text, const std::string& format,
                               Comparator compare = DefaultCompare<T>);

    void Print(std::ostream& out) const;

    bool operator==(const AvlTree& other) const;
    bool operator!=(const AvlTree& other) const;

    template <class U>
    friend std::ostream& operator<<(std::ostream& out, const AvlTree<U>& tree);

private:
    NodePtr root_;
    std::size_t count_ = 0;
    Comparator compare_;

    static int Height(const Node* node);
    static int BalanceFactor(const Node* node);
    static void UpdateHeight(Node& node);
    static void RotateLeft(NodePtr& node);
    static void RotateRight(NodePtr& node);
    static void Rebalance(NodePtr& node);

    bool Insert(NodePtr& node, const T& value, bool replace);
    void RemoveFrom(NodePtr& node, const T& value);
    static NodePtr DetachMin(NodePtr& node);

    const Node* FindNode(const T& value) const;
    bool IsOrdered() const;
    static bool CheckBalance(const Node* node);

    static void InOrder(const Node* node, const std::function<void(const T&)>& action);
    static void TraverseNode(const Node* node, const DynamicArray<char>& steps,
                             const std::function<void(const T&)>& action);
    static void PreOrder(const Node* node, const std::function<void(const T&)>& action);
    static NodePtr CopyNodes(const Node* node);
    static std::size_t CountNodes(const Node* node);
    bool SameTrees(const Node* a, const Node* b) const;
    static void WriteNode(const Node* node, std::string& out);
    static NodePtr ReadNode(const std::string& text, std::size_t& pos);
    static NodePtr ReadChild(const std::string& text, std::size_t& pos);
    static void PrintChildren(std::ostream& out, const Node* node, const std::string& prefix);

    struct Format {
        char open[3];
        char part[3];
        char close[3];
    };
    static Format ParseFormat(const std::string& format);
    static bool IsSpecial(const Format& f, char c);
    static void WriteFormatted(const Node* node, const Format& f, std::string& out);
    static NodePtr ReadFormatted(const std::string& text, std::size_t& pos, const Format& f);
    AvlTree& CheckRead(const std::string& text, std::size_t pos);
};

template <class T>
class AvlTree<T>::TreeEnumerator : public IEnumerator<T> {
public:
    TreeEnumerator(const Node* root) : root_(root) { Reset(); }

    bool Next() override {
        if (stackSize_ == 0) {
            current_ = nullptr;
            return false;
        }
        current_ = stack_[--stackSize_];
        PushLeftChain(current_->right.get());
        return true;
    }

    bool HasNext() const override { return stackSize_ > 0; }

    const T& GetCurrentItem() const override {
        if (current_ == nullptr) {
            throw IndexOutOfRange("Перечислитель вне коллекции: сначала вызовите Next()");
        }
        return current_->value;
    }

    bool TryGetCurrentItem(T& item) const override {
        if (current_ == nullptr) {
            return false;
        }
        item = current_->value;
        return true;
    }

    void Reset() override {
        stackSize_ = 0;
        current_ = nullptr;
        PushLeftChain(root_);
    }

private:
    const Node* root_;
    const Node* current_ = nullptr;
    DynamicArray<const Node*> stack_;
    int stackSize_ = 0;

    void PushLeftChain(const Node* node) {
        while (node != nullptr) {
            if (stackSize_ == stack_.GetSize()) {
                stack_.Resize(stack_.GetSize() * 2 + 4);
            }
            stack_[stackSize_++] = node;
            node = node->left.get();
        }
    }
};

template <class T>
std::unique_ptr<IEnumerator<T>> AvlTree<T>::GetEnumerator() const {
    return std::make_unique<TreeEnumerator>(root_.get());
}

template <class T>
AvlTree<T>::AvlTree(Comparator compare) : compare_(compare) {
    if (!compare_) {
        throw TreeException("Компаратор не задан");
    }
}

template <class T>
AvlTree<T>::AvlTree(std::initializer_list<T> items, Comparator compare) : AvlTree(compare) {
    for (const T& item : items) {
        Add(item);
    }
}

template <class T>
AvlTree<T>::AvlTree(const AvlTree& other)
    : root_(CopyNodes(other.root_.get())), count_(other.count_), compare_(other.compare_) {}

template <class T>
AvlTree<T>::AvlTree(AvlTree&& other)
    : root_(std::move(other.root_)), count_(other.count_), compare_(other.compare_) {
    other.count_ = 0;
}

template <class T>
AvlTree<T>& AvlTree<T>::operator=(AvlTree other) {
    std::swap(root_, other.root_);
    std::swap(count_, other.count_);
    std::swap(compare_, other.compare_);
    return *this;
}

template <class T>
std::size_t AvlTree<T>::GetCount() const {
    return count_;
}

template <class T>
bool AvlTree<T>::IsEmpty() const {
    return count_ == 0;
}

template <class T>
int AvlTree<T>::GetHeight() const {
    return Height(root_.get());
}

template <class T>
const T& AvlTree<T>::GetRoot() const {
    if (!root_) {
        throw EmptyTree("Дерево пустое");
    }
    return root_->value;
}

template <class T>
bool AvlTree<T>::IsBalanced() const {
    return CheckBalance(root_.get());
}

template <class T>
bool AvlTree<T>::Add(const T& value) {
    bool added = Insert(root_, value, false);
    if (added) {
        ++count_;
    }
    return added;
}

template <class T>
bool AvlTree<T>::AddOrReplace(const T& value) {
    bool added = Insert(root_, value, true);
    if (added) {
        ++count_;
    }
    return added;
}

template <class T>
bool AvlTree<T>::Contains(const T& value) const {
    return FindNode(value) != nullptr;
}

template <class T>
const T* AvlTree<T>::Find(const T& value) const {
    const Node* node = FindNode(value);
    return node == nullptr ? nullptr : &node->value;
}

template <class T>
bool AvlTree<T>::TryFind(const T& value, T& found) const {
    const T* item = Find(value);
    if (item == nullptr) {
        return false;
    }
    found = *item;
    return true;
}

template <class T>
void AvlTree<T>::Remove(const T& value) {
    RemoveFrom(root_, value);
    --count_;
}

template <class T>
void AvlTree<T>::Clear() {
    root_.reset();
    count_ = 0;
}

template <class T>
DynamicArray<T> AvlTree<T>::ToArray() const {
    DynamicArray<T> result(static_cast<int>(count_));
    int index = 0;
    InOrder(root_.get(), [&](const T& x) { result[index++] = x; });
    return result;
}

template <class T>
void AvlTree<T>::Traverse(const std::string& order, const std::function<void(const T&)>& action) const {
    DynamicArray<char> steps;
    for (std::size_t i = 0; i < order.size();) {
        char step = 0;
        if (order.compare(i, 2, "К") == 0 || order[i] == 'N') {
            step = 'N';
        } else if (order.compare(i, 2, "Л") == 0 || order[i] == 'L') {
            step = 'L';
        } else if (order.compare(i, 2, "П") == 0 || order[i] == 'R') {
            step = 'R';
        } else {
            throw TreeException("Неизвестный символ в порядке обхода: " + order);
        }
        i += (order[i] == step) ? 1 : 2;
        steps.Resize(steps.GetSize() + 1);
        steps[steps.GetSize() - 1] = step;
    }
    if (steps.GetSize() == 0) {
        throw TreeException("Порядок обхода пустой");
    }
    TraverseNode(root_.get(), steps, action);
}

template <class T>
AvlTree<T> AvlTree<T>::Map(const std::function<T(const T&)>& f) const {
    AvlTree result(compare_);
    PreOrder(root_.get(), [&](const T& x) { result.Add(f(x)); });
    return result;
}

template <class T>
AvlTree<T> AvlTree<T>::Where(const std::function<bool(const T&)>& condition) const {
    AvlTree result(compare_);
    PreOrder(root_.get(), [&](const T& x) {
        if (condition(x)) {
            result.Add(x);
        }
    });
    return result;
}

template <class T>
T AvlTree<T>::Reduce(const std::function<T(const T&, const T&)>& f, const T& start) const {
    T acc = start;
    InOrder(root_.get(), [&](const T& x) { acc = f(x, acc); });
    return acc;
}

template <class T>
AvlTree<T> AvlTree<T>::ExtractSubtree(const T& value) const {
    const Node* node = FindNode(value);
    if (node == nullptr) {
        throw ElementNotFound("Извлечение поддерева: элемента нет в дереве");
    }
    AvlTree result(compare_);
    result.root_ = CopyNodes(node);
    result.count_ = CountNodes(result.root_.get());
    return result;
}

template <class T>
bool AvlTree<T>::ContainsSubtree(const AvlTree& sub) const {
    if (!sub.root_) {
        return true;
    }
    const Node* node = FindNode(sub.root_->value);
    return node != nullptr && SameTrees(node, sub.root_.get());
}

template <class T>
std::string AvlTree<T>::Serialize() const {
    std::string out;
    WriteNode(root_.get(), out);
    return out;
}

template <class T>
AvlTree<T> AvlTree<T>::Deserialize(const std::string& text, Comparator compare) {
    AvlTree result(compare);
    std::size_t pos = 0;
    result.root_ = ReadNode(text, pos);
    result.count_ = CountNodes(result.root_.get());
    result.CheckRead(text, pos);
    return result;
}

template <class T>
AvlTree<T>& AvlTree<T>::CheckRead(const std::string& text, std::size_t pos) {
    if (pos != text.size()) {
        throw ParseError("Лишние символы в конце строки", pos);
    }
    if (!IsOrdered()) {
        throw TreeException("Это не дерево поиска: слева должно быть меньше, справа больше");
    }
    if (!IsBalanced()) {
        throw TreeException("Это не AVL-дерево: в каком-то узле высоты поддеревьев отличаются больше чем на 1");
    }
    return *this;
}

template <class T>
std::string AvlTree<T>::Serialize(const std::string& format) const {
    Format f = ParseFormat(format);
    std::string out;
    WriteFormatted(root_.get(), f, out);
    return out;
}

template <class T>
AvlTree<T> AvlTree<T>::Deserialize(const std::string& text, const std::string& format, Comparator compare) {
    Format f = ParseFormat(format);
    AvlTree result(compare);
    std::size_t pos = 0;
    if (!text.empty()) {
        result.root_ = ReadFormatted(text, pos, f);
    }
    result.count_ = CountNodes(result.root_.get());
    result.CheckRead(text, pos);
    return result;
}

template <class T>
typename AvlTree<T>::Format AvlTree<T>::ParseFormat(const std::string& format) {
    Format f{};
    std::size_t i = 0;
    bool used[3] = {false, false, false};
    for (int g = 0; g < 3; ++g) {
        if (i >= format.size()) {
            throw TreeException("Формат: нужно три группы вида {К}(Л)[П], а дан \"" + format + "\"");
        }
        f.open[g] = format[i++];
        char part = 0;
        if (format.compare(i, 2, "К") == 0 || format.compare(i, 1, "N") == 0) {
            part = 'N';
        } else if (format.compare(i, 2, "Л") == 0 || format.compare(i, 1, "L") == 0) {
            part = 'L';
        } else if (format.compare(i, 2, "П") == 0 || format.compare(i, 1, "R") == 0) {
            part = 'R';
        } else {
            throw TreeException("Формат: после скобки ожидалась буква К, Л или П в \"" + format + "\"");
        }
        i += (format[i] == part) ? 1 : 2;
        int index = part == 'N' ? 0 : (part == 'L' ? 1 : 2);
        if (used[index]) {
            throw TreeException("Формат: буквы К, Л, П должны встретиться по одному разу");
        }
        used[index] = true;
        f.part[g] = part;
        if (i >= format.size()) {
            throw TreeException("Формат: не хватает закрывающей скобки в \"" + format + "\"");
        }
        f.close[g] = format[i++];
    }
    if (i != format.size()) {
        throw TreeException("Формат: лишние символы после трёх групп в \"" + format + "\"");
    }
    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            if (f.open[a] == f.close[b]) {
                throw TreeException("Формат: открывающая и закрывающая скобки должны быть разными");
            }
        }
        for (char c : {f.open[a], f.close[a]}) {
            unsigned char u = static_cast<unsigned char>(c);
            if (u >= 128 || std::isalnum(u) || std::isspace(u) || c == '\\' || c == '-' || c == '.') {
                throw TreeException("Формат: скобкой должен быть знак вроде ( ) [ ] { } < >");
            }
        }
    }
    return f;
}

template <class T>
bool AvlTree<T>::IsSpecial(const Format& f, char c) {
    if (c == '\\') {
        return true;
    }
    for (int g = 0; g < 3; ++g) {
        if (c == f.open[g] || c == f.close[g]) {
            return true;
        }
    }
    return false;
}

template <class T>
void AvlTree<T>::WriteFormatted(const Node* node, const Format& f, std::string& out) {
    if (node == nullptr) {
        return;
    }
    for (int g = 0; g < 3; ++g) {
        out += f.open[g];
        if (f.part[g] == 'N') {
            for (char c : ValueCodec<T>::ToText(node->value)) {
                if (IsSpecial(f, c)) {
                    out += '\\';
                }
                out += c;
            }
        } else {
            WriteFormatted(f.part[g] == 'L' ? node->left.get() : node->right.get(), f, out);
        }
        out += f.close[g];
    }
}

template <class T>
typename AvlTree<T>::NodePtr AvlTree<T>::ReadFormatted(const std::string& text, std::size_t& pos, const Format& f) {
    NodePtr children[2];
    std::string raw;
    std::size_t valuePos = 0;
    for (int g = 0; g < 3; ++g) {
        if (pos >= text.size() || text[pos] != f.open[g]) {
            throw ParseError(std::string("Нужна '") + f.open[g] + "'", pos);
        }
        ++pos;
        if (f.part[g] == 'N') {
            valuePos = pos;
            while (pos < text.size() && text[pos] != f.close[g]) {
                if (text[pos] == '\\') {
                    ++pos;
                    if (pos >= text.size()) {
                        throw ParseError("Строка оборвалась после '\\'", pos);
                    }
                } else if (IsSpecial(f, text[pos])) {
                    throw ParseError(std::string("Неожиданный символ '") + text[pos] + "' внутри значения", pos);
                }
                raw += text[pos++];
            }
        } else if (pos < text.size() && text[pos] != f.close[g]) {
            children[f.part[g] == 'L' ? 0 : 1] = ReadFormatted(text, pos, f);
        }
        if (pos >= text.size() || text[pos] != f.close[g]) {
            throw ParseError(std::string("Нужна '") + f.close[g] + "'", pos);
        }
        ++pos;
    }
    NodePtr node;
    try {
        node = std::make_unique<Node>(ValueCodec<T>::FromText(raw));
    } catch (const TreeException& e) {
        throw ParseError(e.what(), valuePos);
    }
    node->left = std::move(children[0]);
    node->right = std::move(children[1]);
    UpdateHeight(*node);
    return node;
}

template <class T>
void AvlTree<T>::Print(std::ostream& out) const {
    if (!root_) {
        out << "(пустое дерево)\n";
        return;
    }
    out << root_->value << "\n";
    PrintChildren(out, root_.get(), "");
}

template <class T>
bool AvlTree<T>::operator==(const AvlTree& other) const {
    if (count_ != other.count_) {
        return false;
    }
    DynamicArray<T> a = ToArray();
    DynamicArray<T> b = other.ToArray();
    for (int i = 0; i < a.GetSize(); ++i) {
        if (compare_(a[i], b[i]) != 0) {
            return false;
        }
    }
    return true;
}

template <class T>
bool AvlTree<T>::operator!=(const AvlTree& other) const {
    return !(*this == other);
}

template <class U>
std::ostream& operator<<(std::ostream& out, const AvlTree<U>& tree) {
    out << "{";
    bool first = true;
    AvlTree<U>::InOrder(tree.root_.get(), [&](const U& x) {
        out << (first ? "" : ", ") << x;
        first = false;
    });
    return out << "}";
}

template <class T>
int AvlTree<T>::Height(const Node* node) {
    return node == nullptr ? 0 : node->height;
}

template <class T>
int AvlTree<T>::BalanceFactor(const Node* node) {
    return node == nullptr ? 0 : Height(node->left.get()) - Height(node->right.get());
}

template <class T>
void AvlTree<T>::UpdateHeight(Node& node) {
    node.height = 1 + std::max(Height(node.left.get()), Height(node.right.get()));
}

template <class T>
void AvlTree<T>::RotateLeft(NodePtr& node) {
    NodePtr b = std::move(node->right);
    node->right = std::move(b->left);
    UpdateHeight(*node);
    b->left = std::move(node);
    node = std::move(b);
    UpdateHeight(*node);
}

template <class T>
void AvlTree<T>::RotateRight(NodePtr& node) {
    NodePtr b = std::move(node->left);
    node->left = std::move(b->right);
    UpdateHeight(*node);
    b->right = std::move(node);
    node = std::move(b);
    UpdateHeight(*node);
}

template <class T>
void AvlTree<T>::Rebalance(NodePtr& node) {
    UpdateHeight(*node);
    int balance = BalanceFactor(node.get());
    if (balance > 1) {
        if (BalanceFactor(node->left.get()) < 0) {
            RotateLeft(node->left);
        }
        RotateRight(node);
    } else if (balance < -1) {
        if (BalanceFactor(node->right.get()) > 0) {
            RotateRight(node->right);
        }
        RotateLeft(node);
    }
}

template <class T>
bool AvlTree<T>::Insert(NodePtr& node, const T& value, bool replace) {
    if (!node) {
        node = std::make_unique<Node>(value);
        return true;
    }
    int c = compare_(value, node->value);
    bool added = false;
    if (c < 0) {
        added = Insert(node->left, value, replace);
    } else if (c > 0) {
        added = Insert(node->right, value, replace);
    } else {
        if (replace) {
            node->value = value;
        }
        return false;
    }
    if (added) {
        Rebalance(node);
    }
    return added;
}

template <class T>
void AvlTree<T>::RemoveFrom(NodePtr& node, const T& value) {
    if (!node) {
        throw ElementNotFound("Удаление: элемента нет в дереве");
    }
    int c = compare_(value, node->value);
    if (c < 0) {
        RemoveFrom(node->left, value);
    } else if (c > 0) {
        RemoveFrom(node->right, value);
    } else if (!node->left) {
        node = std::move(node->right);
    } else if (!node->right) {
        node = std::move(node->left);
    } else {
        NodePtr minNode = DetachMin(node->right);
        minNode->left = std::move(node->left);
        minNode->right = std::move(node->right);
        node = std::move(minNode);
    }
    if (node) {
        Rebalance(node);
    }
}

template <class T>
typename AvlTree<T>::NodePtr AvlTree<T>::DetachMin(NodePtr& node) {
    if (!node->left) {
        NodePtr minNode = std::move(node);
        node = std::move(minNode->right);
        return minNode;
    }
    NodePtr minNode = DetachMin(node->left);
    Rebalance(node);
    return minNode;
}

template <class T>
const typename AvlTree<T>::Node* AvlTree<T>::FindNode(const T& value) const {
    const Node* node = root_.get();
    while (node != nullptr) {
        int c = compare_(value, node->value);
        if (c < 0) {
            node = node->left.get();
        } else if (c > 0) {
            node = node->right.get();
        } else {
            return node;
        }
    }
    return nullptr;
}

template <class T>
bool AvlTree<T>::IsOrdered() const {
    DynamicArray<T> items = ToArray();
    for (int i = 1; i < items.GetSize(); ++i) {
        if (compare_(items[i - 1], items[i]) >= 0) {
            return false;
        }
    }
    return true;
}

template <class T>
bool AvlTree<T>::CheckBalance(const Node* node) {
    if (node == nullptr) {
        return true;
    }
    int left = Height(node->left.get());
    int right = Height(node->right.get());
    bool heightOk = node->height == 1 + std::max(left, right);
    bool balanceOk = left - right >= -1 && left - right <= 1;
    return heightOk && balanceOk && CheckBalance(node->left.get()) && CheckBalance(node->right.get());
}

template <class T>
void AvlTree<T>::InOrder(const Node* node, const std::function<void(const T&)>& action) {
    if (node == nullptr) {
        return;
    }
    InOrder(node->left.get(), action);
    action(node->value);
    InOrder(node->right.get(), action);
}

template <class T>
void AvlTree<T>::TraverseNode(const Node* node, const DynamicArray<char>& steps,
                              const std::function<void(const T&)>& action) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < steps.GetSize(); ++i) {
        if (steps[i] == 'N') {
            action(node->value);
        } else if (steps[i] == 'L') {
            TraverseNode(node->left.get(), steps, action);
        } else {
            TraverseNode(node->right.get(), steps, action);
        }
    }
}

template <class T>
void AvlTree<T>::PreOrder(const Node* node, const std::function<void(const T&)>& action) {
    if (node == nullptr) {
        return;
    }
    action(node->value);
    PreOrder(node->left.get(), action);
    PreOrder(node->right.get(), action);
}

template <class T>
typename AvlTree<T>::NodePtr AvlTree<T>::CopyNodes(const Node* node) {
    if (node == nullptr) {
        return nullptr;
    }
    NodePtr copy = std::make_unique<Node>(node->value);
    copy->height = node->height;
    copy->left = CopyNodes(node->left.get());
    copy->right = CopyNodes(node->right.get());
    return copy;
}

template <class T>
std::size_t AvlTree<T>::CountNodes(const Node* node) {
    return node == nullptr ? 0 : 1 + CountNodes(node->left.get()) + CountNodes(node->right.get());
}

template <class T>
bool AvlTree<T>::SameTrees(const Node* a, const Node* b) const {
    if (a == nullptr || b == nullptr) {
        return a == b;
    }
    bool equal = compare_(a->value, b->value) == 0;
    return equal && SameTrees(a->left.get(), b->left.get()) && SameTrees(a->right.get(), b->right.get());
}

template <class T>
void AvlTree<T>::WriteNode(const Node* node, std::string& out) {
    if (node == nullptr) {
        return;
    }
    for (char c : ValueCodec<T>::ToText(node->value)) {
        if (c == '(' || c == ')' || c == '\\') {
            out += '\\';
        }
        out += c;
    }
    out += '(';
    WriteNode(node->left.get(), out);
    out += ")(";
    WriteNode(node->right.get(), out);
    out += ')';
}

template <class T>
typename AvlTree<T>::NodePtr AvlTree<T>::ReadNode(const std::string& text, std::size_t& pos) {
    if (pos >= text.size() || text[pos] == ')') {
        return nullptr;
    }

    std::size_t start = pos;
    std::string raw;
    while (pos < text.size() && text[pos] != '(') {
        if (text[pos] == ')') {
            throw ParseError("Лишняя ')'", pos);
        }
        if (text[pos] == '\\') {
            ++pos;
            if (pos >= text.size()) {
                throw ParseError("Строка оборвалась после '\\'", pos);
            }
        }
        raw += text[pos++];
    }
    if (pos >= text.size()) {
        throw ParseError("После значения нужна '('", pos);
    }

    NodePtr node;
    try {
        node = std::make_unique<Node>(ValueCodec<T>::FromText(raw));
    } catch (const TreeException& e) {
        throw ParseError(e.what(), start);
    }
    node->left = ReadChild(text, pos);
    node->right = ReadChild(text, pos);
    UpdateHeight(*node);
    return node;
}

template <class T>
typename AvlTree<T>::NodePtr AvlTree<T>::ReadChild(const std::string& text, std::size_t& pos) {
    if (pos >= text.size() || text[pos] != '(') {
        throw ParseError("Нужна '('", pos);
    }
    ++pos;
    NodePtr child = ReadNode(text, pos);
    if (pos >= text.size() || text[pos] != ')') {
        throw ParseError("Нужна ')'", pos);
    }
    ++pos;
    return child;
}

template <class T>
void AvlTree<T>::PrintChildren(std::ostream& out, const Node* node, const std::string& prefix) {
    const Node* children[2] = {node->left.get(), node->right.get()};
    const char* names[2] = {"Л: ", "П: "};
    for (int i = 0; i < 2; ++i) {
        if (children[i] == nullptr) {
            continue;
        }
        bool last = (i == 1 || !node->right);
        out << prefix << (last ? "└── " : "├── ") << names[i] << children[i]->value << "\n";
        PrintChildren(out, children[i], prefix + (last ? "    " : "│   "));
    }
}
