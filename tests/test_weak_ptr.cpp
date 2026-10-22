// Тесты WeakPtr. В каждом тесте одна проверка.

#include "WeakPtr.hpp"
#include "test_framework.hpp"

struct Node {
    static int alive;
    ShrdPtr<Node> next;
    WeakPtr<Node> prev;
    Node() { ++alive; }
    ~Node() { --alive; }
};
int Node::alive = 0;

TEST(WeakPtr, EmptyIsExpired) {
    WeakPtr<int> w;
    CHECK_EQUAL("Пустой WeakPtr считается истёкшим", "WeakPtr<int> w; w.Expired()", w.Expired(), true);
}

TEST(WeakPtr, DoesNotIncreaseCounter) {
    ShrdPtr<int> shared = MakeShared<int>(5);
    WeakPtr<int> w = shared;
    CHECK_EQUAL("WeakPtr не увеличивает число владельцев", "WeakPtr<int> w = shared; shared.UseCount()",
                shared.UseCount(), (size_t)1);
}

TEST(WeakPtr, NotExpiredWhileAlive) {
    ShrdPtr<int> shared = MakeShared<int>(5);
    WeakPtr<int> w = shared;
    CHECK_EQUAL("Пока объект жив, WeakPtr не истёк", "w = shared; w.Expired()", w.Expired(), false);
}

TEST(WeakPtr, LockGivesObject) {
    ShrdPtr<int> shared = MakeShared<int>(5);
    WeakPtr<int> w = shared;
    ShrdPtr<int> locked = w.Lock();
    CHECK_EQUAL("Lock даёт доступ к объекту", "ShrdPtr<int> locked = w.Lock(); *locked", *locked, 5);
}

TEST(WeakPtr, LockAddsOwner) {
    ShrdPtr<int> shared = MakeShared<int>(5);
    WeakPtr<int> w = shared;
    ShrdPtr<int> locked = w.Lock();
    CHECK_EQUAL("Lock создаёт временного владельца", "locked = w.Lock(); shared.UseCount()",
                shared.UseCount(), (size_t)2);
}

TEST(WeakPtr, ExpiredAfterOwnerGone) {
    WeakPtr<int> w;
    {
        ShrdPtr<int> shared = MakeShared<int>(1);
        w = shared;
    }
    CHECK_EQUAL("WeakPtr истекает, когда владельцев нет", "{ shared } закончился; w.Expired()",
                w.Expired(), true);
}

TEST(WeakPtr, DoesNotKeepObjectAlive) {
    WeakPtr<Counter> w;
    {
        ShrdPtr<Counter> shared = MakeShared<Counter>(1);
        w = shared;
    }
    CHECK_EQUAL("WeakPtr не удерживает объект", "{ shared } закончился, объектов в памяти",
                Counter::alive, 0);
}

TEST(WeakPtr, LockExpiredGivesEmpty) {
    WeakPtr<int> w;
    {
        ShrdPtr<int> shared = MakeShared<int>(1);
        w = shared;
    }
    ShrdPtr<int> locked = w.Lock();
    CHECK_EQUAL("Lock у истёкшего WeakPtr даёт пустой указатель", "w.Lock().Get() == nullptr",
                locked.Get() == nullptr, true);
}

TEST(WeakPtr, CycleIsBroken) {
    {
        ShrdPtr<Node> a = MakeShared<Node>();
        ShrdPtr<Node> b = MakeShared<Node>();
        a->next = b;
        b->prev = a;
    }
    CHECK_EQUAL("Цикл a <-> b не даёт утечки", "a->next = b; b->prev = a; узлов в памяти", Node::alive, 0);
}
