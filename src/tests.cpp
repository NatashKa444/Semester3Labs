#include <gtest/gtest.h>

#include <clocale>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>

#include "AvlTree.hpp"
#include "Dictionary.hpp"
#include "DynamicArray.hpp"

using IntTree = AvlTree<int>;

static int testNumber = 0;

template <class T>
std::string Show(const T& value) {
    std::ostringstream out;
    out << std::boolalpha << value;
    return out.str();
}

template <class T>
std::string Show(const DynamicArray<T>& values) {
    std::string result = "[";
    for (int i = 0; i < values.GetSize(); ++i) {
        result += (i > 0 ? ", " : "") + Show(values[i]);
    }
    return result + "]";
}

#define TEST_START(name, input, expected)                            \
    ++testNumber;                                                    \
    std::cout << "\n===== Тест " << testNumber << ". " << name << " =====\n"; \
    std::cout << "Файл: " << __FILE__ << ", строка: " << __LINE__ << "\n"; \
    std::cout << "Вход: " << input << "\n";                          \
    std::cout << "Ожидалось: " << expected << "\n"

#define CHECK(actual, expected)                                      \
    {                                                                \
        auto result = (actual);                                      \
        std::cout << "Получено: " << Show(result) << "\n";           \
        if (result == (expected)) {                                  \
            std::cout << "РЕЗУЛЬТАТ: ПРОЙДЕН\n";                     \
        } else {                                                     \
            std::cout << "РЕЗУЛЬТАТ: НЕ ПРОЙДЕН\n";                  \
        }                                                            \
        EXPECT_EQ(result, expected);                                 \
    }

#define CHECK_THROW(expr, ExceptionType)                             \
    try {                                                            \
        expr;                                                        \
        std::cout << "Получено: исключения нет\n";                   \
        std::cout << "РЕЗУЛЬТАТ: НЕ ПРОЙДЕН\n";                      \
        ADD_FAILURE() << "ожидалось исключение " #ExceptionType;     \
    } catch (const ExceptionType& e) {                               \
        std::cout << "Получено: исключение (" << e.what() << ")\n";  \
        std::cout << "РЕЗУЛЬТАТ: ПРОЙДЕН\n";                         \
    }

struct Counted {
    int value = 0;
    static int alive;

    Counted() { ++alive; }
    Counted(int v) : value(v) { ++alive; }
    Counted(const Counted& other) : value(other.value) { ++alive; }
    Counted& operator=(const Counted& other) = default;
    ~Counted() { --alive; }

    bool operator<(const Counted& other) const { return value < other.value; }
};

int Counted::alive = 0;

std::ostream& operator<<(std::ostream& out, const Counted& c) {
    return out << c.value;
}

std::istream& operator>>(std::istream& in, Counted& c) {
    return in >> c.value;
}

AvlTree<Counted> MakeCountedTree(int count) {
    AvlTree<Counted> tree;
    for (int i = 0; i < count; ++i) {
        tree.Add(Counted((i * 37) % count));
    }
    return tree;
}

TEST(Basic, test1_empty_count) {
    TEST_START("Количество элементов пустого дерева", "IntTree()", 0);
    IntTree t;
    CHECK(t.GetCount(), 0u);
}

TEST(Basic, test2_empty_height) {
    TEST_START("Высота пустого дерева", "IntTree()", 0);
    IntTree t;
    CHECK(t.GetHeight(), 0);
}

TEST(Basic, test3_empty_root) {
    TEST_START("Корень пустого дерева", "IntTree().GetRoot()", "исключение EmptyTree");
    IntTree t;
    CHECK_THROW(t.GetRoot(), EmptyTree);
}

TEST(Basic, test4_add_returns_true) {
    TEST_START("Добавление нового элемента", "{}, Add(5)", "true");
    IntTree t;
    CHECK(t.Add(5), true);
}

TEST(Basic, test5_add_duplicate) {
    TEST_START("Добавление повтора", "{1, 2, 3}, Add(2)", "false");
    IntTree t{1, 2, 3};
    CHECK(t.Add(2), false);
}

TEST(Basic, test6_add_duplicate_count) {
    TEST_START("Количество после добавления повтора", "{1, 2, 3}, Add(2)", 3);
    IntTree t{1, 2, 3};
    t.Add(2);
    CHECK(t.GetCount(), 3u);
}

TEST(Basic, test7_sorted_order) {
    TEST_START("Элементы выдаются по возрастанию", "Add 5, 3, 8, 1", "[1, 3, 5, 8]");
    IntTree t{5, 3, 8, 1};
    CHECK(Show(t.ToArray()), std::string("[1, 3, 5, 8]"));
}

TEST(Basic, test8_root) {
    TEST_START("Корень дерева из трёх", "Add 5, 3, 8", 5);
    IntTree t{5, 3, 8};
    CHECK(t.GetRoot(), 5);
}

TEST(Basic, test9_height_balanced) {
    TEST_START("Высота дерева из трёх", "Add 5, 3, 8", 2);
    IntTree t{5, 3, 8};
    CHECK(t.GetHeight(), 2);
}

TEST(Basic, test10_height_sorted_input) {
    TEST_START("Отсортированный ввод не даёт «палку»", "Add 1, 2, 3, 4", 3);
    IntTree t{1, 2, 3, 4};
    CHECK(t.GetHeight(), 3);
}

TEST(Basic, test11_contains_yes) {
    TEST_START("Поиск существующего", "{5, 3, 8}, Contains(3)", "true");
    IntTree t{5, 3, 8};
    CHECK(t.Contains(3), true);
}

TEST(Basic, test12_contains_no) {
    TEST_START("Поиск отсутствующего", "{5, 3, 8}, Contains(4)", "false");
    IntTree t{5, 3, 8};
    CHECK(t.Contains(4), false);
}

TEST(Basic, test13_contains_empty) {
    TEST_START("Поиск в пустом дереве", "{}, Contains(1)", "false");
    IntTree t;
    CHECK(t.Contains(1), false);
}

TEST(Basic, test14_remove_leaf) {
    TEST_START("Удаление листа", "{5, 3, 8}, Remove(3)", "[5, 8]");
    IntTree t{5, 3, 8};
    t.Remove(3);
    CHECK(Show(t.ToArray()), std::string("[5, 8]"));
}

TEST(Basic, test15_remove_one_child) {
    TEST_START("Удаление узла с одним ребёнком", "{5, 3, 1}, Remove(3)", "[1, 5]");
    IntTree t{5, 3, 1};
    t.Remove(3);
    CHECK(Show(t.ToArray()), std::string("[1, 5]"));
}

TEST(Basic, test16_remove_two_children) {
    TEST_START("Удаление корня с двумя детьми", "{50, 30, 70, 20, 40, 60, 80}, Remove(50)",
               "[20, 30, 40, 60, 70, 80]");
    IntTree t{50, 30, 70, 20, 40, 60, 80};
    t.Remove(50);
    CHECK(Show(t.ToArray()), std::string("[20, 30, 40, 60, 70, 80]"));
}

TEST(Basic, test17_remove_two_children_new_root) {
    TEST_START("Новый корень после удаления", "{50, 30, 70, 20, 40, 60, 80}, Remove(50)",
               "60 (минимум правого поддерева)");
    IntTree t{50, 30, 70, 20, 40, 60, 80};
    t.Remove(50);
    CHECK(t.GetRoot(), 60);
}

TEST(Basic, test18_remove_last) {
    TEST_START("Удаление единственного элемента", "{7}, Remove(7)", "пустое дерево");
    IntTree t{7};
    t.Remove(7);
    CHECK(t.IsEmpty(), true);
}

TEST(Basic, test19_remove_missing) {
    TEST_START("Удаление отсутствующего", "{1, 2, 3}, Remove(10)", "исключение ElementNotFound");
    IntTree t{1, 2, 3};
    CHECK_THROW(t.Remove(10), ElementNotFound);
}

TEST(Basic, test20_remove_from_empty) {
    TEST_START("Удаление из пустого", "{}, Remove(1)", "исключение ElementNotFound");
    IntTree t;
    CHECK_THROW(t.Remove(1), ElementNotFound);
}

TEST(Basic, test21_clear) {
    TEST_START("Очистка", "{1, 2, 3}, Clear()", 0);
    IntTree t{1, 2, 3};
    t.Clear();
    CHECK(t.GetCount(), 0u);
}

TEST(Basic, test22_random_vs_marks) {
    TEST_START("20000 случайных вставок и удалений", "сравнение с массивом отметок есть/нет", "true (элементы совпадают)");
    std::mt19937 rng(42);
    IntTree t;
    DynamicArray<bool> present(2000);
    for (int step = 0; step < 20000; ++step) {
        int x = static_cast<int>(rng() % 2000);
        if (rng() % 3 == 0 && present[x]) {
            t.Remove(x);
            present[x] = false;
        } else {
            t.Add(x);
            present[x] = true;
        }
    }
    std::string expected = "[";
    for (int x = 0; x < 2000; ++x) {
        if (present[x]) {
            expected += (expected.size() > 1 ? ", " : "") + std::to_string(x);
        }
    }
    CHECK(Show(t.ToArray()) == expected + "]", true);
}

TEST(Basic, test23_copy_independent) {
    TEST_START("Копия не зависит от оригинала", "a={1, 2, 3}, b=a, b.Add(4)", "в a 3 элемента");
    IntTree a{1, 2, 3};
    IntTree b = a;
    b.Add(4);
    CHECK(a.GetCount(), 3u);
}

TEST(Basic, test24_assign) {
    TEST_START("Присваивание", "a={1}, b={5, 6}, a=b", "[5, 6]");
    IntTree a{1};
    IntTree b{5, 6};
    a = b;
    CHECK(Show(a.ToArray()), std::string("[5, 6]"));
}

TEST(MapWhereReduce, test25_map) {
    TEST_START("map умножение на 10", "{1, 2, 3}, x*10", "[10, 20, 30]");
    IntTree t{1, 2, 3};
    CHECK(Show(t.Map([](const int& x) { return x * 10; }).ToArray()), std::string("[10, 20, 30]"));
}

TEST(MapWhereReduce, test26_map_keeps_source) {
    TEST_START("map не меняет исходное дерево", "{1, 2, 3}, x*10", "[1, 2, 3]");
    IntTree t{1, 2, 3};
    t.Map([](const int& x) { return x * 10; });
    CHECK(Show(t.ToArray()), std::string("[1, 2, 3]"));
}

TEST(MapWhereReduce, test27_map_balanced) {
    TEST_START("Результат map тоже сбалансирован", "1..100, x*x", "true");
    IntTree t;
    for (int i = 1; i <= 100; ++i) {
        t.Add(i);
    }
    CHECK(t.Map([](const int& x) { return x * x; }).IsBalanced(), true);
}

TEST(MapWhereReduce, test28_map_duplicates) {
    TEST_START("map с одинаковыми результатами", "{-2, -1, 1, 2}, |x|", "[1, 2]");
    IntTree t{-2, -1, 1, 2};
    CHECK(Show(t.Map([](const int& x) { return x < 0 ? -x : x; }).ToArray()), std::string("[1, 2]"));
}

TEST(MapWhereReduce, test29_where) {
    TEST_START("where чётные", "{1, 2, 3, 4, 5, 6}", "[2, 4, 6]");
    IntTree t{1, 2, 3, 4, 5, 6};
    CHECK(Show(t.Where([](const int& x) { return x % 2 == 0; }).ToArray()), std::string("[2, 4, 6]"));
}

TEST(MapWhereReduce, test30_where_nothing) {
    TEST_START("where ничего не подходит", "{1, 2, 3}, false", "пустое дерево");
    IntTree t{1, 2, 3};
    CHECK(t.Where([](const int&) { return false; }).IsEmpty(), true);
}

TEST(MapWhereReduce, test31_reduce_sum) {
    TEST_START("reduce сумма", "{1, 2, 3, 4}, x+acc, c=0", 10);
    IntTree t{1, 2, 3, 4};
    CHECK(t.Reduce([](const int& x, const int& acc) { return x + acc; }, 0), 10);
}

TEST(MapWhereReduce, test32_reduce_empty) {
    TEST_START("reduce пустого дерева", "{}, x+acc, c=7", 7);
    IntTree t;
    CHECK(t.Reduce([](const int& x, const int& acc) { return x + acc; }, 7), 7);
}

TEST(MapWhereReduce, test33_reduce_example) {
    TEST_START("reduce пример из задания", "{1, 2, 3}, 2x+3acc, c=4", 144);
    IntTree t{1, 2, 3};
    CHECK(t.Reduce([](const int& x, const int& acc) { return 2 * x + 3 * acc; }, 4), 144);
}

TEST(Subtrees, test34_extract) {
    TEST_START("Извлечение поддерева", "{4, 2, 6, 1, 3, 5, 7}, ExtractSubtree(2)", "[1, 2, 3]");
    IntTree t{4, 2, 6, 1, 3, 5, 7};
    CHECK(Show(t.ExtractSubtree(2).ToArray()), std::string("[1, 2, 3]"));
}

TEST(Subtrees, test35_extract_root) {
    TEST_START("Корень извлечённого поддерева", "{4, 2, 6, 1, 3, 5, 7}, ExtractSubtree(6)", 6);
    IntTree t{4, 2, 6, 1, 3, 5, 7};
    CHECK(t.ExtractSubtree(6).GetRoot(), 6);
}

TEST(Subtrees, test36_extract_is_copy) {
    TEST_START("Поддерево — это копия", "sub = ExtractSubtree(2), sub.Add(100)", "в исходном нет 100");
    IntTree t{4, 2, 6, 1, 3, 5, 7};
    IntTree sub = t.ExtractSubtree(2);
    sub.Add(100);
    CHECK(t.Contains(100), false);
}

TEST(Subtrees, test37_extract_missing) {
    TEST_START("Извлечение по отсутствующему", "{4, 2, 6}, ExtractSubtree(42)", "исключение ElementNotFound");
    IntTree t{4, 2, 6};
    CHECK_THROW(t.ExtractSubtree(42), ElementNotFound);
}

TEST(Subtrees, test38_contains_subtree_yes) {
    TEST_START("Поиск поддерева: есть", "{4, 2, 6, 1, 3, 5, 7}, поддерево {6, 5, 7}", "true");
    IntTree t{4, 2, 6, 1, 3, 5, 7};
    CHECK(t.ContainsSubtree(IntTree{6, 5, 7}), true);
}

TEST(Subtrees, test39_contains_subtree_leaf) {
    TEST_START("Поиск поддерева из одного листа", "{4, 2, 6, 1, 3, 5, 7}, поддерево {5}", "true");
    IntTree t{4, 2, 6, 1, 3, 5, 7};
    CHECK(t.ContainsSubtree(IntTree{5}), true);
}

TEST(Subtrees, test40_contains_subtree_other_shape) {
    TEST_START("Поиск поддерева: другая форма", "{4, 2, 6, 1, 3, 5, 7}, поддерево {6, 5}", "false");
    IntTree t{4, 2, 6, 1, 3, 5, 7};
    CHECK(t.ContainsSubtree(IntTree{6, 5}), false);
}

TEST(Subtrees, test41_contains_subtree_other_values) {
    TEST_START("Поиск поддерева: другие значения", "{4, 2, 6, 1, 3, 5, 7}, поддерево {2, 1, 30}", "false");
    IntTree t{4, 2, 6, 1, 3, 5, 7};
    CHECK(t.ContainsSubtree(IntTree{2, 1, 30}), false);
}

TEST(Subtrees, test42_contains_empty_subtree) {
    TEST_START("Пустое поддерево", "{4, 2, 6}, поддерево {}", "true");
    IntTree t{4, 2, 6};
    CHECK(t.ContainsSubtree(IntTree()), true);
}

TEST(Strings, test43_serialize) {
    TEST_START("Сохранение в строку", "{2, 1, 3}", "2(1()())(3()())");
    IntTree t{2, 1, 3};
    CHECK(t.Serialize(), std::string("2(1()())(3()())"));
}

TEST(Strings, test44_serialize_empty) {
    TEST_START("Сохранение пустого дерева", "{}", "пустая строка");
    IntTree t;
    CHECK(t.Serialize(), std::string(""));
}

TEST(Strings, test45_deserialize) {
    TEST_START("Чтение из строки", "\"-5(-10()())()\"", "[-10, -5]");
    CHECK(Show(IntTree::Deserialize("-5(-10()())()").ToArray()), std::string("[-10, -5]"));
}

TEST(Strings, test46_save_and_load) {
    TEST_START("Сохранить и прочитать обратно", "{50, 30, 70, 20, 40, 60, 80, 65}", "та же строка");
    IntTree t{50, 30, 70, 20, 40, 60, 80, 65};
    CHECK(IntTree::Deserialize(t.Serialize()).Serialize(), t.Serialize());
}

TEST(Strings, test47_no_brackets) {
    TEST_START("Строка без скобок", "\"5\"", "исключение ParseError");
    CHECK_THROW(IntTree::Deserialize("5"), ParseError);
}

TEST(Strings, test48_no_right) {
    TEST_START("Нет правого поддерева", "\"5()\"", "исключение ParseError");
    CHECK_THROW(IntTree::Deserialize("5()"), ParseError);
}

TEST(Strings, test49_garbage_at_end) {
    TEST_START("Мусор в конце", "\"5()()x\"", "исключение ParseError");
    CHECK_THROW(IntTree::Deserialize("5()()x"), ParseError);
}

TEST(Strings, test50_not_a_number) {
    TEST_START("Не число", "\"abc()()\"", "исключение ParseError");
    CHECK_THROW(IntTree::Deserialize("abc()()"), ParseError);
}

TEST(Strings, test51_wrong_order) {
    TEST_START("Нарушен порядок", "\"5(9()())()\" (9 слева от 5)", "исключение TreeException");
    CHECK_THROW(IntTree::Deserialize("5(9()())()"), TreeException);
}

TEST(OtherTypes, test52_double) {
    TEST_START("Вещественные числа: сумма", "{2.5, -1.25, 0.5}", 1.75);
    AvlTree<double> t{2.5, -1.25, 0.5};
    CHECK(t.Reduce([](const double& x, const double& acc) { return x + acc; }, 0.0), 1.75);
}

TEST(OtherTypes, test53_double_save_load) {
    TEST_START("Вещественные числа: строка туда и обратно", "{2.5, -1.25, 0.1}", "те же элементы");
    AvlTree<double> t{2.5, -1.25, 0.1};
    CHECK(Show(AvlTree<double>::Deserialize(t.Serialize()).ToArray()), Show(t.ToArray()));
}

TEST(OtherTypes, test54_strings_order) {
    TEST_START("Строки по алфавиту", "{груша, яблоко, апельсин}", "[апельсин, груша, яблоко]");
    AvlTree<std::string> t{"груша", "яблоко", "апельсин"};
    CHECK(Show(t.ToArray()), std::string("[апельсин, груша, яблоко]"));
}

TEST(OtherTypes, test55_strings_with_brackets) {
    TEST_START("Строки со скобками внутри", "{a(b), c\\d}", "те же строки после чтения");
    AvlTree<std::string> t{"a(b)", "c\\d"};
    CHECK(Show(AvlTree<std::string>::Deserialize(t.Serialize()).ToArray()), Show(t.ToArray()));
}

TEST(OtherTypes, test56_print_operator) {
    TEST_START("Оператор <<", "{3, 1, 2}", "{1, 2, 3}");
    std::ostringstream out;
    out << IntTree{3, 1, 2};
    CHECK(out.str(), std::string("{1, 2, 3}"));
}

int Descending(const int& a, const int& b) {
    return a < b ? 1 : (a > b ? -1 : 0);  // без b - a: разность может переполнить int
}

int CompareAsNumbers(const std::string& a, const std::string& b) {
    long long x = std::stoll(a);
    long long y = std::stoll(b);
    return x < y ? -1 : (x > y ? 1 : 0);
}

TEST(Comparator, test57_descending) {
    TEST_START("Компаратор по убыванию", "{1, 5, 3}, компаратор b - a", "[5, 3, 1]");
    IntTree t({1, 5, 3}, Descending);
    CHECK(Show(t.ToArray()), std::string("[5, 3, 1]"));
}

TEST(Comparator, test58_strings_default) {
    TEST_START("Строки-числа без компаратора", "{\"10\", \"9\", \"2\"}", "[10, 2, 9] (по алфавиту)");
    AvlTree<std::string> t{"10", "9", "2"};
    CHECK(Show(t.ToArray()), std::string("[10, 2, 9]"));
}

TEST(Comparator, test59_strings_as_numbers) {
    TEST_START("Строки-числа с компаратором", "{\"10\", \"9\", \"2\"}, сравнение как чисел", "[2, 9, 10]");
    AvlTree<std::string> t({"10", "9", "2"}, CompareAsNumbers);
    CHECK(Show(t.ToArray()), std::string("[2, 9, 10]"));
}

TEST(Comparator, test60_map_keeps_comparator) {
    TEST_START("map сохраняет компаратор", "{1, 2, 3} по убыванию, x*10", "[30, 20, 10]");
    IntTree t({1, 2, 3}, Descending);
    CHECK(Show(t.Map([](const int& x) { return x * 10; }).ToArray()), std::string("[30, 20, 10]"));
}

TEST(Comparator, test61_empty_comparator) {
    TEST_START("Пустой компаратор", "IntTree(nullptr)", "исключение TreeException");
    CHECK_THROW(IntTree t(nullptr), TreeException);
}

TEST(Comparator, test62_try_find_yes) {
    TEST_START("TryFind существующего", "{5, 3, 8}, TryFind(8)", "true, найдено 8");
    IntTree t{5, 3, 8};
    int found = 0;
    bool ok = t.TryFind(8, found);
    CHECK(ok && found == 8, true);
}

TEST(Comparator, test63_try_find_no) {
    TEST_START("TryFind отсутствующего", "{5, 3, 8}, TryFind(4)", "false");
    IntTree t{5, 3, 8};
    int found = 0;
    CHECK(t.TryFind(4, found), false);
}

TEST(Comparator, test64_traverse_klp) {
    TEST_START("Обход КЛП", "{2, 1, 3}", "2 1 3");
    IntTree t{2, 1, 3};
    std::string visited;
    t.Traverse("КЛП", [&](const int& x) { visited += std::to_string(x) + " "; });
    CHECK(visited, std::string("2 1 3 "));
}

TEST(Comparator, test65_traverse_pkl) {
    TEST_START("Обход ПКЛ (по убыванию)", "{2, 1, 3}", "3 2 1");
    IntTree t{2, 1, 3};
    std::string visited;
    t.Traverse("ПКЛ", [&](const int& x) { visited += std::to_string(x) + " "; });
    CHECK(visited, std::string("3 2 1 "));
}

TEST(Comparator, test66_traverse_latin) {
    TEST_START("Обход латиницей LNR", "{2, 1, 3}", "1 2 3");
    IntTree t{2, 1, 3};
    std::string visited;
    t.Traverse("LNR", [&](const int& x) { visited += std::to_string(x) + " "; });
    CHECK(visited, std::string("1 2 3 "));
}

TEST(Comparator, test67_traverse_root_twice) {
    TEST_START("Обход NLRN (корень дважды, пример из лекции 7)", "{2, 1, 3}", "2 1 1 3 3 2");
    IntTree t{2, 1, 3};
    std::string visited;
    t.Traverse("NLRN", [&](const int& x) { visited += std::to_string(x) + " "; });
    CHECK(visited, std::string("2 1 1 3 3 2 "));
}

TEST(Comparator, test68_traverse_bad_order) {
    TEST_START("Неверный порядок обхода", "Traverse(\"КЛX\")", "исключение TreeException");
    IntTree t{2, 1, 3};
    CHECK_THROW(t.Traverse("КЛX", [](const int&) {}), TreeException);
}

TEST(Leaks, test69_destructor) {
    TEST_START("Деструктор освобождает все узлы", "дерево из 100 элементов вышло из области видимости",
               "лишних живых элементов 0");
    int before = Counted::alive;
    { AvlTree<Counted> t = MakeCountedTree(100); }
    CHECK(Counted::alive - before, 0);
}

TEST(Leaks, test70_remove_all) {
    TEST_START("Удаление всех элементов", "100 элементов, Remove каждого", "лишних живых элементов 0");
    int before = Counted::alive;
    AvlTree<Counted> t = MakeCountedTree(100);
    for (int i = 0; i < 100; ++i) {
        t.Remove(Counted(i));
    }
    CHECK(Counted::alive - before, 0);
}

TEST(Leaks, test71_clear) {
    TEST_START("Clear освобождает узлы", "100 элементов, Clear()", "лишних живых элементов 0");
    int before = Counted::alive;
    AvlTree<Counted> t = MakeCountedTree(100);
    t.Clear();
    CHECK(Counted::alive - before, 0);
}

TEST(Leaks, test72_copy_and_assign) {
    TEST_START("Копирование и присваивание", "a, b = a, a = c, всё вышло из области видимости",
               "лишних живых элементов 0");
    int before = Counted::alive;
    {
        AvlTree<Counted> a = MakeCountedTree(50);
        AvlTree<Counted> b = a;
        AvlTree<Counted> c = MakeCountedTree(30);
        a = c;
    }
    CHECK(Counted::alive - before, 0);
}

TEST(Leaks, test73_map_where_extract) {
    TEST_START("map, where, извлечение поддерева", "результаты вышли из области видимости", "лишних живых элементов 0");
    int before = Counted::alive;
    {
        AvlTree<Counted> t = MakeCountedTree(50);
        AvlTree<Counted> m = t.Map([](const Counted& x) { return Counted(x.value * 2); });
        AvlTree<Counted> w = t.Where([](const Counted& x) { return x.value % 2 == 0; });
        AvlTree<Counted> s = t.ExtractSubtree(Counted(10));
    }
    CHECK(Counted::alive - before, 0);
}

TEST(Leaks, test74_bad_string) {
    TEST_START("Ошибка при чтении строки", "\"1(0()())(2()(x()()))\" — ошибка в глубине", "лишних живых элементов 0");
    int before = Counted::alive;
    try {
        AvlTree<Counted>::Deserialize("1(0()())(2()(x()()))");
    } catch (const ParseError&) {
    }
    CHECK(Counted::alive - before, 0);
}

TEST(Avl, test75_rotate_left) {
    TEST_START("Поворот налево (случай ПП)", "Add 1, 2, 3", "2(1()())(3()())");
    IntTree t{1, 2, 3};
    CHECK(t.Serialize(), std::string("2(1()())(3()())"));
}

TEST(Avl, test76_rotate_right) {
    TEST_START("Поворот направо (случай ЛЛ)", "Add 3, 2, 1", "2(1()())(3()())");
    IntTree t{3, 2, 1};
    CHECK(t.Serialize(), std::string("2(1()())(3()())"));
}

TEST(Avl, test77_left_right) {
    TEST_START("Двойной поворот (случай ЛП)", "Add 3, 1, 2", "2(1()())(3()())");
    IntTree t{3, 1, 2};
    CHECK(t.Serialize(), std::string("2(1()())(3()())"));
}

TEST(Avl, test78_right_left) {
    TEST_START("Двойной поворот (случай ПЛ)", "Add 1, 3, 2", "2(1()())(3()())");
    IntTree t{1, 3, 2};
    CHECK(t.Serialize(), std::string("2(1()())(3()())"));
}

TEST(Avl, test79_sorted_1000_height) {
    TEST_START("1000 чисел по возрастанию", "Add 1..1000", "высота 10 (обычное дерево дало бы 1000)");
    IntTree t;
    for (int i = 1; i <= 1000; ++i) {
        t.Add(i);
    }
    CHECK(t.GetHeight(), 10);
}

TEST(Avl, test80_remove_rebalances) {
    TEST_START("Удаление вызывает поворот", "{2, 1, 3, 4}, Remove(1)", "3(2()())(4()())");
    IntTree t{2, 1, 3, 4};
    t.Remove(1);
    CHECK(t.Serialize(), std::string("3(2()())(4()())"));
}

TEST(Avl, test81_random_stays_balanced) {
    TEST_START("20000 случайных вставок и удалений", "после каждого шага", "дерево всегда сбалансировано");
    std::mt19937 rng(7);
    IntTree t;
    bool balanced = true;
    for (int step = 0; step < 20000; ++step) {
        int x = static_cast<int>(rng() % 500);
        if (t.Contains(x)) {
            t.Remove(x);
        } else {
            t.Add(x);
        }
        balanced = balanced && t.IsBalanced();
    }
    CHECK(balanced, true);
}

TEST(Avl, test82_deserialize_not_balanced) {
    TEST_START("Чтение несбалансированной строки", "\"1()(2()(3()()))\" (палка)", "исключение TreeException");
    CHECK_THROW(IntTree::Deserialize("1()(2()(3()()))"), TreeException);
}

TEST(Avl, test83_move_takes_nodes) {
    TEST_START("Перемещение забирает узлы", "a={1, 2, 3}, b=std::move(a)", "в a 0 элементов");
    IntTree a{1, 2, 3};
    IntTree b = std::move(a);
    CHECK(a.GetCount(), 0u);
}

using StringDict = Dictionary<std::string, int>;

TEST(Dict, test84_add_get) {
    TEST_START("Добавить и получить", "{кот: 4, паук: 8}, Get(паук)", 8);
    StringDict d;
    d.Add("кот", 4);
    d.Add("паук", 8);
    CHECK(d.Get("паук"), 8);
}

TEST(Dict, test85_add_existing_replaces) {
    TEST_START("Повторный ключ заменяет значение", "Add(кот, 4), Add(кот, 5), Get(кот)", 5);
    StringDict d;
    d.Add("кот", 4);
    d.Add("кот", 5);
    CHECK(d.Get("кот"), 5);
}

TEST(Dict, test86_add_existing_count) {
    TEST_START("Повторный ключ не увеличивает количество", "Add(кот, 4), Add(кот, 5)", 1);
    StringDict d;
    d.Add("кот", 4);
    d.Add("кот", 5);
    CHECK(d.GetCount(), 1u);
}

TEST(Dict, test87_get_missing) {
    TEST_START("Получить отсутствующий ключ", "{кот: 4}, Get(пёс)", "исключение KeyNotFound");
    StringDict d;
    d.Add("кот", 4);
    CHECK_THROW(d.Get("пёс"), KeyNotFound);
}

TEST(Dict, test88_contains_key) {
    TEST_START("ContainsKey", "{кот: 4}, ContainsKey(пёс)", "false");
    StringDict d;
    d.Add("кот", 4);
    CHECK(d.ContainsKey("пёс"), false);
}

TEST(Dict, test89_try_get) {
    TEST_START("TryGet существующего", "{кот: 4}, TryGet(кот)", "true, значение 4");
    StringDict d;
    d.Add("кот", 4);
    int value = 0;
    bool ok = d.TryGet("кот", value);
    CHECK(ok && value == 4, true);
}

TEST(Dict, test90_remove) {
    TEST_START("Удаление ключа", "{кот: 4, пёс: 4}, Remove(кот)", "[пёс]");
    StringDict d;
    d.Add("кот", 4);
    d.Add("пёс", 4);
    d.Remove("кот");
    CHECK(Show(d.GetKeys()), std::string("[пёс]"));
}

TEST(Dict, test91_remove_missing) {
    TEST_START("Удаление отсутствующего ключа", "{}, Remove(кот)", "исключение KeyNotFound");
    StringDict d;
    CHECK_THROW(d.Remove("кот"), KeyNotFound);
}

TEST(Dict, test92_keys_sorted) {
    TEST_START("Ключи по алфавиту", "Add груша, яблоко, апельсин", "[апельсин, груша, яблоко]");
    StringDict d;
    d.Add("груша", 1);
    d.Add("яблоко", 2);
    d.Add("апельсин", 3);
    CHECK(Show(d.GetKeys()), std::string("[апельсин, груша, яблоко]"));
}

TEST(Dict, test93_values_by_key_order) {
    TEST_START("Значения в порядке ключей", "Add груша:1, яблоко:2, апельсин:3", "[3, 1, 2]");
    StringDict d;
    d.Add("груша", 1);
    d.Add("яблоко", 2);
    d.Add("апельсин", 3);
    CHECK(Show(d.GetValues()), std::string("[3, 1, 2]"));
}

TEST(Dict, test94_key_comparator) {
    TEST_START("Компаратор ключей: строки как числа", "Add \"10\", \"9\", \"2\"", "[2, 9, 10]");
    Dictionary<std::string, int> d([](const std::string& a, const std::string& b) {
        long long x = std::stoll(a);
        long long y = std::stoll(b);
        return x < y ? -1 : (x > y ? 1 : 0);
    });
    d.Add("10", 0);
    d.Add("9", 0);
    d.Add("2", 0);
    CHECK(Show(d.GetKeys()), std::string("[2, 9, 10]"));
}

TEST(Dict, test95_sorted_keys_height) {
    TEST_START("1000 ключей по возрастанию", "Add 1..1000", "высота дерева 10");
    Dictionary<int, int> d;
    for (int i = 1; i <= 1000; ++i) {
        d.Add(i, i * i);
    }
    CHECK(d.GetHeight(), 10);
}

TEST(Dict, test96_leaks) {
    TEST_START("Словарь не теряет память", "100 записей, удалить половину, выйти из области видимости",
               "лишних живых элементов 0");
    int before = Counted::alive;
    {
        Dictionary<int, Counted> d;
        for (int i = 0; i < 100; ++i) {
            d.Add(i, Counted(i));
        }
        for (int i = 0; i < 100; i += 2) {
            d.Remove(i);
        }
    }
    CHECK(Counted::alive - before, 0);
}

TEST(Format, test97_serialize_format) {
    TEST_START("Сохранение по формату {К}(Л)[П]", "{2, 1, 3}", "{2}({1}()[])[{3}()[]]");
    IntTree t{2, 1, 3};
    CHECK(t.Serialize("{К}(Л)[П]"), std::string("{2}({1}()[])[{3}()[]]"));
}

TEST(Format, test98_serialize_other_order) {
    TEST_START("Сохранение по формату (Л)[П]<К> (обход ЛПК)", "{2, 1, 3}", "(()[]<1>)[()[]<3>]<2>");
    IntTree t{2, 1, 3};
    CHECK(t.Serialize("(Л)[П]<К>"), std::string("(()[]<1>)[()[]<3>]<2>"));
}

TEST(Format, test99_serialize_latin) {
    TEST_START("Формат латинскими буквами [N](L)(R)", "{2, 1}", "[2]([1]()())()");
    IntTree t{2, 1};
    CHECK(t.Serialize("[N](L)(R)"), std::string("[2]([1]()())()"));
}

TEST(Format, test100_serialize_empty) {
    TEST_START("Пустое дерево по формату", "{}", "пустая строка");
    IntTree t;
    CHECK(t.Serialize("{К}(Л)[П]"), std::string(""));
}

TEST(Format, test101_deserialize_format) {
    TEST_START("Чтение по формату {К}(Л)[П]", "\"{2}({1}()[])[{3}()[]]\"", "[1, 2, 3]");
    CHECK(Show(IntTree::Deserialize("{2}({1}()[])[{3}()[]]", "{К}(Л)[П]").ToArray()), std::string("[1, 2, 3]"));
}

TEST(Format, test102_save_load_all_orders) {
    TEST_START("Сохранить и прочитать в формате (П){К}[Л]", "{50, 30, 70, 20, 40, 60, 80, 65}", "та же строка");
    IntTree t{50, 30, 70, 20, 40, 60, 80, 65};
    std::string text = t.Serialize("(П){К}[Л]");
    CHECK(IntTree::Deserialize(text, "(П){К}[Л]").Serialize(), t.Serialize());
}

TEST(Format, test103_strings_with_brackets) {
    TEST_START("Строки со скобками внутри значения", "{\"a{b}\", \"(x)\"}", "читается обратно то же дерево");
    AvlTree<std::string> t{"a{b}", "(x)"};
    std::string text = t.Serialize("{К}(Л)[П]");
    CHECK(AvlTree<std::string>::Deserialize(text, "{К}(Л)[П]") == t, true);
}

TEST(Format, test104_bad_format_letter) {
    TEST_START("Неверный формат: буква Х", "\"{К}(Х)[П]\"", "исключение TreeException");
    IntTree t{1};
    CHECK_THROW(t.Serialize("{К}(Х)[П]"), TreeException);
}

TEST(Format, test105_bad_format_twice) {
    TEST_START("Неверный формат: К два раза", "\"{К}(К)[П]\"", "исключение TreeException");
    IntTree t{1};
    CHECK_THROW(t.Serialize("{К}(К)[П]"), TreeException);
}

TEST(Format, test106_bad_format_short) {
    TEST_START("Неверный формат: только две группы", "\"{К}(Л)\"", "исключение TreeException");
    IntTree t{1};
    CHECK_THROW(t.Serialize("{К}(Л)"), TreeException);
}

TEST(Format, test107_wrong_bracket) {
    TEST_START("Строка не по формату: не та скобка", "\"{2}[]()\" при формате {К}(Л)[П]", "исключение ParseError");
    CHECK_THROW(IntTree::Deserialize("{2}[]()", "{К}(Л)[П]"), ParseError);
}

TEST(Format, test108_garbage_at_end) {
    TEST_START("Мусор в конце строки по формату", "\"{2}()[]x\"", "исключение ParseError");
    CHECK_THROW(IntTree::Deserialize("{2}()[]x", "{К}(Л)[П]"), ParseError);
}

TEST(Format, test109_not_search_tree) {
    TEST_START("По формату прочитано не дерево поиска", "\"{5}({9}()[])[]\"", "исключение TreeException");
    CHECK_THROW(IntTree::Deserialize("{5}({9}()[])[]", "{К}(Л)[П]"), TreeException);
}

TEST(Enumerator, test110_enumerate_sorted) {
    TEST_START("Перечислитель идёт по возрастанию", "{5, 3, 8, 1, 4}", "1 3 4 5 8 ");
    IntTree t{5, 3, 8, 1, 4};
    std::string out;
    auto it = t.GetEnumerator();
    while (it->Next()) {
        out += std::to_string(it->GetCurrentItem()) + " ";
    }
    CHECK(out, std::string("1 3 4 5 8 "));
}

TEST(Enumerator, test111_empty_tree) {
    TEST_START("Перечислитель пустого дерева", "{}, Next()", "false");
    IntTree t;
    auto it = t.GetEnumerator();
    CHECK(it->Next(), false);
}

TEST(Enumerator, test112_current_before_next) {
    TEST_START("Текущий элемент до первого Next()", "{1, 2}", "исключение IndexOutOfRange");
    IntTree t{1, 2};
    auto it = t.GetEnumerator();
    CHECK_THROW(it->GetCurrentItem(), IndexOutOfRange);
}

TEST(Enumerator, test113_has_next) {
    TEST_START("HasNext на последнем элементе", "{1, 2}, два раза Next()", "false");
    IntTree t{1, 2};
    auto it = t.GetEnumerator();
    it->Next();
    it->Next();
    CHECK(it->HasNext(), false);
}

TEST(Enumerator, test114_try_get_after_end) {
    TEST_START("TryGetCurrentItem после конца", "{7}, Next() два раза", "false");
    IntTree t{7};
    auto it = t.GetEnumerator();
    it->Next();
    it->Next();
    int value = 0;
    CHECK(it->TryGetCurrentItem(value), false);
}

TEST(Enumerator, test115_reset) {
    TEST_START("Reset возвращает в начало", "{1, 2, 3}, Next() x3, Reset(), Next()", "1");
    IntTree t{1, 2, 3};
    auto it = t.GetEnumerator();
    it->Next();
    it->Next();
    it->Next();
    it->Reset();
    it->Next();
    CHECK(it->GetCurrentItem(), 1);
}

TEST(Enumerator, test116_count_1000) {
    TEST_START("Перечислитель проходит все 1000 элементов по порядку", "0..999 по возрастанию", "true");
    IntTree t;
    for (int i = 0; i < 1000; ++i) {
        t.Add(i);
    }
    auto it = t.GetEnumerator();
    int expected = 0;
    bool ok = true;
    while (it->Next()) {
        ok = ok && it->GetCurrentItem() == expected;
        ++expected;
    }
    CHECK(ok && expected == 1000, true);
}

TEST(Enumerator, test117_dictionary) {
    TEST_START("Перечислитель словаря идёт по ключам", "{b: 2, a: 1, c: 3}", "a=1 b=2 c=3 ");
    Dictionary<std::string, int> d;
    d.Add("b", 2);
    d.Add("a", 1);
    d.Add("c", 3);
    std::string out;
    auto it = d.GetEnumerator();
    while (it->Next()) {
        out += it->GetCurrentItem().key + "=" + std::to_string(it->GetCurrentItem().value) + " ";
    }
    CHECK(out, std::string("a=1 b=2 c=3 "));
}

TEST(Enumerator, test118_through_icollection) {
    TEST_START("Работа с деревом через интерфейс ICollection", "ICollection<int>& c = tree; Add 3, 1, 2; Remove 1",
               "2");
    IntTree t;
    ICollection<int>& c = t;
    c.Add(3);
    c.Add(1);
    c.Add(2);
    c.Remove(1);
    CHECK(c.GetCount(), 2u);
}

TEST(Enumerator, test119_leaks) {
    TEST_START("Утечки: перечислитель и чтение по формату", "500 элементов Counted", "0 лишних объектов");
    int before = Counted::alive;
    {
        AvlTree<Counted> t = MakeCountedTree(500);
        auto it = t.GetEnumerator();
        while (it->Next()) {
        }
        AvlTree<Counted> copy = AvlTree<Counted>::Deserialize(t.Serialize("{К}(Л)[П]"), "{К}(Л)[П]");
        try {
            AvlTree<Counted>::Deserialize("{1}({2}()[])[", "{К}(Л)[П]");
        } catch (const TreeException&) {
        }
    }
    CHECK(Counted::alive - before, 0);
}

TEST(Operators, test120_equal_different_insert_order) {
    TEST_START("Оператор ==", "a={1, 2, 3}, b={3, 1, 2} (другой порядок вставки)", "a == b -> true");
    IntTree a{1, 2, 3};
    IntTree b{3, 1, 2};
    CHECK(a == b, true);
}

TEST(Operators, test121_equal_false_for_different) {
    TEST_START("Оператор == для разных деревьев", "a={1, 2, 3}, b={1, 2, 4}", "a == b -> false");
    IntTree a{1, 2, 3};
    IntTree b{1, 2, 4};
    CHECK(a == b, false);
}

TEST(Operators, test122_not_equal) {
    TEST_START("Оператор !=", "a={1, 2, 3}, b={1, 2}", "a != b -> true");
    IntTree a{1, 2, 3};
    IntTree b{1, 2};
    CHECK(a != b, true);
}

TEST(Operators, test123_not_equal_false_for_equal) {
    TEST_START("Оператор != для одинаковых деревьев", "a={5, 7}, b=копия a", "a != b -> false");
    IntTree a{5, 7};
    IntTree b = a;
    CHECK(a != b, false);
}

TEST(Edge, test124_zero_and_negatives) {
    TEST_START("Ноль и отрицательные", "{0, -1, 1, -5}, Remove(0)", "[-5, -1, 1]");
    IntTree t{0, -1, 1, -5};
    t.Remove(0);
    CHECK(Show(t.ToArray()), std::string("[-5, -1, 1]"));
}

TEST(Edge, test125_int_limits_order) {
    TEST_START("Граничные значения int", "{INT_MAX, 0, INT_MIN, -1}", "[-2147483648, -1, 0, 2147483647]");
    IntTree t{std::numeric_limits<int>::max(), 0, std::numeric_limits<int>::min(), -1};
    CHECK(Show(t.ToArray()), std::string("[-2147483648, -1, 0, 2147483647]"));
}

TEST(Edge, test126_int_limits_roundtrip) {
    TEST_START("INT_MIN и INT_MAX в строку и обратно", "{INT_MIN, INT_MAX}", "то же дерево");
    IntTree t{std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};
    CHECK(IntTree::Deserialize(t.Serialize()) == t, true);
}

TEST(Edge, test127_overflow_in_string) {
    TEST_START("Переполнение int при чтении", "\"99999999999()()\" (больше INT_MAX)", "исключение ParseError");
    CHECK_THROW(IntTree::Deserialize("99999999999()()"), ParseError);
}

class ShortPrinter : public ::testing::EmptyTestEventListener {
    void OnTestPartResult(const ::testing::TestPartResult& result) override {
        if (!result.failed()) {
            return;
        }
        if (result.line_number() > 0) {
            std::cout << "\033[31mПроверка не прошла: " << result.file_name() << ", строка " << result.line_number()
                      << "\033[0m\n";
        } else {
            std::cout << "\033[31m" << result.summary() << "\033[0m\n";
        }
    }

    void OnTestEnd(const ::testing::TestInfo& info) override {
        if (info.result()->Passed()) {
            std::cout << "\033[32m[  OK  ]\033[0m " << info.test_suite_name() << "." << info.name() << "\n";
        } else {
            std::cout << "\033[31m[ НЕ ПРОЙДЕН ]\033[0m " << info.test_suite_name() << "." << info.name() << "\n";
        }
    }

    void OnTestProgramEnd(const ::testing::UnitTest& unit) override {
        std::cout << "\n========================================\n";
        std::cout << "Всего тестов: " << unit.total_test_count() << "\n";
        std::cout << "\033[32mПройдено: " << unit.successful_test_count() << "\033[0m\n";
        if (unit.failed_test_count() > 0) {
            std::cout << "\033[31mНе пройдено: " << unit.failed_test_count() << "\033[0m\n";
            for (int i = 0; i < unit.total_test_suite_count(); ++i) {
                const ::testing::TestSuite* suite = unit.GetTestSuite(i);
                for (int j = 0; j < suite->total_test_count(); ++j) {
                    const ::testing::TestInfo* test = suite->GetTestInfo(j);
                    if (test->result()->Failed()) {
                        std::cout << "\033[31m  - " << suite->name() << "." << test->name() << "\033[0m\n";
                    }
                }
            }
        }
    }
};

int main(int argc, char** argv) {
    setlocale(LC_ALL, "Russian");
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new ShortPrinter);

    return RUN_ALL_TESTS();
}
