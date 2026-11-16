#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "Benchmark.hpp"
#include "AvlTree.hpp"
#include "Dictionary.hpp"
#include "DynamicArray.hpp"

namespace {

std::string ReadLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) throw std::runtime_error("ввод закончился");
    return line;
}

long long ReadNumber(const std::string& prompt, long long min, long long max) {
    while (true) {
        std::string line = ReadLine(prompt);
        try {
            std::size_t used = 0;
            long long value = std::stoll(line, &used);
            if (used == line.size() && value >= min && value <= max) return value;
        } catch (const std::logic_error&) {
        }
        std::cout << "  Введите целое число от " << min << " до " << max << ".\n";
    }
}

template <class T>
T ReadValue(const std::string& prompt) {
    while (true) {
        try {
            return ValueCodec<T>::FromText(ReadLine(prompt));
        } catch (const TreeException& e) {
            std::cout << "  Ошибка: " << e.what() << "\n";
        }
    }
}

template <class T>
struct Choice {
    std::string name;
    T function;
};

template <class F, int N>
DynamicArray<Choice<F>> MakeChoices(Choice<F> (&items)[N]) {
    return DynamicArray<Choice<F>>(items, N);
}

template <class T>
struct Functions;

template <>
struct Functions<int> {
    static DynamicArray<Choice<std::function<int(const int&)>>> Maps() {
        Choice<std::function<int(const int&)>> items[] = {
            {"x * 2", [](const int& x) { return x * 2; }},
            {"x * x", [](const int& x) { return x * x; }},
            {"-x", [](const int& x) { return -x; }}};
        return MakeChoices(items);
    }
    static DynamicArray<Choice<std::function<bool(const int&)>>> Wheres() {
        Choice<std::function<bool(const int&)>> items[] = {
            {"чётные", [](const int& x) { return x % 2 == 0; }},
            {"положительные", [](const int& x) { return x > 0; }}};
        return MakeChoices(items);
    }
    static DynamicArray<Choice<std::function<int(const int&, const int&)>>> Reduces() {
        Choice<std::function<int(const int&, const int&)>> items[] = {
            {"сумма", [](const int& x, const int& acc) { return x + acc; }},
            {"максимум", [](const int& x, const int& acc) { return std::max(x, acc); }}};
        return MakeChoices(items);
    }
    static int Random(std::mt19937& rng, int range) { return std::uniform_int_distribution<int>(-range, range)(rng); }
    static constexpr const char* kName = "целые числа";
};

template <>
struct Functions<double> {
    static DynamicArray<Choice<std::function<double(const double&)>>> Maps() {
        Choice<std::function<double(const double&)>> items[] = {
            {"x / 2", [](const double& x) { return x / 2; }},
            {"округлить", [](const double& x) { return std::round(x); }}};
        return MakeChoices(items);
    }
    static DynamicArray<Choice<std::function<bool(const double&)>>> Wheres() {
        Choice<std::function<bool(const double&)>> items[] = {
            {"положительные", [](const double& x) { return x > 0; }},
            {"по модулю меньше 10", [](const double& x) { return std::abs(x) < 10; }}};
        return MakeChoices(items);
    }
    static DynamicArray<Choice<std::function<double(const double&, const double&)>>> Reduces() {
        Choice<std::function<double(const double&, const double&)>> items[] = {
            {"сумма", [](const double& x, const double& acc) { return x + acc; }},
            {"максимум", [](const double& x, const double& acc) { return std::max(x, acc); }}};
        return MakeChoices(items);
    }
    static double Random(std::mt19937& rng, int range) {
        return std::round(std::uniform_real_distribution<double>(-range, range)(rng) * 100) / 100;
    }
    static constexpr const char* kName = "вещественные числа";
};

template <>
struct Functions<std::string> {
    using S = std::string;
    static DynamicArray<Choice<std::function<S(const S&)>>> Maps() {
        Choice<std::function<S(const S&)>> items[] = {
            {"перевернуть", [](const S& s) { return S(s.rbegin(), s.rend()); }},
            {"первая буква", [](const S& s) { return s.substr(0, 1); }}};
        return MakeChoices(items);
    }
    static DynamicArray<Choice<std::function<bool(const S&)>>> Wheres() {
        Choice<std::function<bool(const S&)>> items[] = {
            {"длина больше 3", [](const S& s) { return s.size() > 3; }},
            {"начинается на 'a'", [](const S& s) { return !s.empty() && s[0] == 'a'; }}};
        return MakeChoices(items);
    }
    static DynamicArray<Choice<std::function<S(const S&, const S&)>>> Reduces() {
        Choice<std::function<S(const S&, const S&)>> items[] = {
            {"склеить через пробел", [](const S& x, const S& acc) { return acc.empty() ? x : acc + " " + x; }}};
        return MakeChoices(items);
    }
    static S Random(std::mt19937& rng, int range) {
        int length = 1 + static_cast<int>(rng() % static_cast<unsigned>(std::max(1, std::min(range, 8))));
        S s;
        for (int i = 0; i < length; ++i) s += static_cast<char>('a' + rng() % 26);
        return s;
    }
    static constexpr const char* kName = "строки";
};

template <class F>
F PickFunction(const DynamicArray<Choice<F>>& choices, const std::string& title) {
    std::cout << title << ":\n";
    for (int i = 0; i < choices.GetSize(); ++i) std::cout << "  " << i + 1 << ". " << choices[i].name << "\n";
    return choices[static_cast<int>(ReadNumber("> ", 1, choices.GetSize())) - 1].function;
}

template <class T>
class TreeMenu {
public:
    void Run() {
        while (true) {
            std::cout << "\n=== AVL-дерево (" << Functions<T>::kName << "): элементов " << tree_.GetCount()
                      << ", высота " << tree_.GetHeight() << " ===\n"
                      << " 1. Добавить элемент\n"
                      << " 2. Заполнить случайными числами\n"
                      << " 3. Удалить элемент\n"
                      << " 4. Найти элемент\n"
                      << " 5. Показать дерево\n"
                      << " 6. map   (применить функцию ко всем)\n"
                      << " 7. where (оставить подходящие)\n"
                      << " 8. reduce (свернуть в одно значение)\n"
                      << " 9. Извлечь поддерево\n"
                      << "10. Проверить, есть ли поддерево\n"
                      << "11. Сохранить в строку\n"
                      << "12. Прочитать из строки\n"
                      << "13. Очистить\n"
                      << "14. Обход в заданном порядке (КЛП, ЛКП, ПКЛ, ...)\n"
                      << "15. Сохранить в строку по формату, например {К}(Л)[П]\n"
                      << "16. Прочитать из строки по формату\n"
                      << "17. Пройти по дереву перечислителем (IEnumerator)\n"
                      << " 0. Назад\n";
            int choice = static_cast<int>(ReadNumber("> ", 0, 17));
            if (choice == 0) return;
            try {
                Handle(choice);
            } catch (const std::exception& e) {
                std::cout << "  Ошибка: " << e.what() << "\n";
            }
        }
    }

private:
    using Tree = AvlTree<T>;

    void Handle(int choice) {
        switch (choice) {
            case 1:
                std::cout << (tree_.Add(ReadValue<T>("Значение: ")) ? "  Добавлено.\n" : "  Такое уже есть.\n");
                break;
            case 2: FillRandom(); Show(tree_); break;
            case 3: tree_.Remove(ReadValue<T>("Удалить: ")); std::cout << "  Удалено.\n"; break;
            case 4: std::cout << (tree_.Contains(ReadValue<T>("Найти: ")) ? "  Есть в дереве.\n" : "  Нет в дереве.\n"); break;
            case 5: Show(tree_); break;
            case 6: Offer(tree_.Map(PickFunction(Functions<T>::Maps(), "Функция"))); break;
            case 7: Offer(tree_.Where(PickFunction(Functions<T>::Wheres(), "Условие"))); break;
            case 8: {
                auto choices = Functions<T>::Reduces();
                auto f = PickFunction(choices, "Свёртка");
                T start = ReadValue<T>("Начальное значение c (для суммы — 0, для склейки — пусто): ");
                std::cout << "  Результат: " << tree_.Reduce(f, start) << "\n";
                break;
            }
            case 9: Offer(tree_.ExtractSubtree(ReadValue<T>("Корень поддерева: "))); break;
            case 10: {
                std::cout << "Поддерево в формате строки, например 2(1()())(3()()):\n";
                Tree sub = Tree::Deserialize(ReadLine("> "));
                std::cout << (tree_.ContainsSubtree(sub) ? "  Да, такое поддерево есть.\n" : "  Нет, такого поддерева нет.\n");
                break;
            }
            case 11: std::cout << "  " << tree_.Serialize() << "\n"; break;
            case 12: {
                Tree read = Tree::Deserialize(ReadLine("Строка: "));
                CompareWithCurrent(read);
                tree_ = std::move(read);
                Show(tree_);
                break;
            }
            case 13: tree_.Clear(); std::cout << "  Очищено.\n"; break;
            case 14: {
                std::string order = ReadLine("Порядок (К — корень, Л — левое, П — правое), например КЛП: ");
                std::cout << "  ";
                tree_.Traverse(order, [](const T& x) { std::cout << x << " "; });
                std::cout << "\n";
                break;
            }
            case 15: {
                std::string format = ReadFormat();
                std::cout << "  " << tree_.Serialize(format) << "\n";
                break;
            }
            case 16: {
                std::string format = ReadFormat();
                Tree read = Tree::Deserialize(ReadLine("Строка: "), format);
                CompareWithCurrent(read);
                tree_ = std::move(read);
                Show(tree_);
                break;
            }
            case 17: {
                auto it = tree_.GetEnumerator();
                int step = 0;
                while (it->Next()) {
                    std::cout << "  шаг " << ++step << ": " << it->GetCurrentItem()
                              << (it->HasNext() ? "" : "  (последний)") << "\n";
                }
                if (step == 0) std::cout << "  Дерево пустое, перечислитель сразу закончился.\n";
                break;
            }
        }
    }

    static std::string ReadFormat() {
        std::string format = ReadLine("Формат (К — корень, Л — левое, П — правое; Enter — {К}(Л)[П]): ");
        return format.empty() ? std::string("{К}(Л)[П]") : format;
    }

    void FillRandom() {
        long long count = ReadNumber("Сколько элементов: ", 0, 1000000);
        long long range = ReadNumber("Числа от -R до R (для строк R — макс. длина), R = ", 1, 1000000000);
        long long seed = ReadNumber("Зерно генератора (любое число, одно и то же зерно даёт те же числа): ", 0,
                                    4294967295LL);
        std::mt19937 rng(static_cast<unsigned>(seed));
        for (long long i = 0; i < count; ++i) tree_.Add(Functions<T>::Random(rng, static_cast<int>(range)));
    }

    static void Show(const Tree& tree) {
        std::cout << "  По возрастанию: " << tree << "\n";
        if (tree.GetCount() <= 30) {
            std::cout << "  Как выглядит (Л — левый ребёнок, П — правый):\n";
            tree.Print(std::cout);
        }
    }

    void CompareWithCurrent(const Tree& other) const {
        if (other == tree_) {
            std::cout << "  Совпадает с текущим деревом (оператор ==).\n";
        } else if (other != tree_) {
            std::cout << "  Отличается от текущего дерева (оператор !=).\n";
        }
    }

    void Offer(Tree result) {
        std::cout << "  Результат:\n";
        Show(result);
        CompareWithCurrent(result);
        if (ReadLine("  Сделать его текущим деревом? (д/н): ") == "д") tree_ = std::move(result);
    }

    Tree tree_;
};

class DictionaryMenu {
public:
    void Run() {
        while (true) {
            std::cout << "\n=== Словарь (слово -> перевод): записей " << dictionary_.GetCount() << ", высота дерева "
                      << dictionary_.GetHeight() << " ===\n"
                      << " 1. Добавить или изменить запись\n"
                      << " 2. Найти перевод по слову\n"
                      << " 3. Проверить, есть ли слово\n"
                      << " 4. Удалить слово\n"
                      << " 5. Показать словарь\n"
                      << " 6. Заполнить примером\n"
                      << " 7. Очистить\n"
                      << " 0. Назад\n";
            int choice = static_cast<int>(ReadNumber("> ", 0, 7));
            if (choice == 0) return;
            try {
                Handle(choice);
            } catch (const std::exception& e) {
                std::cout << "  Ошибка: " << e.what() << "\n";
            }
        }
    }

private:
    void Handle(int choice) {
        switch (choice) {
            case 1: {
                std::string key = ReadLine("Слово: ");
                std::string value = ReadLine("Перевод: ");
                std::cout << (dictionary_.Add(key, value) ? "  Добавлено.\n" : "  Слово уже было, перевод заменён.\n");
                break;
            }
            case 2: std::cout << "  " << dictionary_.Get(ReadLine("Слово: ")) << "\n"; break;
            case 3:
                std::cout << (dictionary_.ContainsKey(ReadLine("Слово: ")) ? "  Есть в словаре.\n" : "  Нет в словаре.\n");
                break;
            case 4: dictionary_.Remove(ReadLine("Слово: ")); std::cout << "  Удалено.\n"; break;
            case 5: Show(); break;
            case 6: {
                const char* pairs[][2] = {{"cat", "кошка"},   {"dog", "собака"}, {"apple", "яблоко"},
                                          {"house", "дом"},   {"tree", "дерево"}, {"book", "книга"},
                                          {"sun", "солнце"}, {"water", "вода"}};
                for (const auto& pair : pairs) dictionary_.Add(pair[0], pair[1]);
                Show();
                break;
            }
            case 7: dictionary_.Clear(); std::cout << "  Очищено.\n"; break;
        }
    }

    void Show() const {
        std::cout << "  По алфавиту: " << dictionary_ << "\n";
        if (dictionary_.GetCount() <= 30) {
            std::cout << "  Внутреннее AVL-дерево (Л — левый ребёнок, П — правый):\n";
            dictionary_.PrintTree(std::cout);
        }
    }

    Dictionary<std::string, std::string> dictionary_;
};

DynamicArray<BenchmarkResult> lastResults;

void BenchmarkMenu() {
    std::string line = ReadLine("Размеры N через пробел (Enter — 1000 3000 10000 30000 100000 300000 1000000): ");
    DynamicArray<long long> sizes;
    std::istringstream in(line);
    std::string word;
    while (in >> word) {
        long long n = 0;
        std::size_t used = 0;
        try {
            n = std::stoll(word, &used);
        } catch (const std::logic_error&) {
            used = 0;
        }
        if (used == word.size() && n >= 1 && n <= 10000000) {
            sizes.Resize(sizes.GetSize() + 1);
            sizes[sizes.GetSize() - 1] = n;
        } else {
            std::cout << "  Пропускаю «" << word << "»: нужно целое число от 1 до 10000000\n";
        }
    }
    if (sizes.GetSize() == 0) {
        long long defaultSizes[] = {1000, 3000, 10000, 30000, 100000, 300000, 1000000};
        sizes = DynamicArray<long long>(defaultSizes, 7);
    }
    lastResults = RunBenchmark(sizes);
    PrintResults(lastResults);
    if (SaveResultsCsv(lastResults, "results.csv")) {
        DrawPlot("results.csv", "benchmark.png");
    }
}

void ChartsMenu() {
    if (lastResults.GetSize() == 0) {
        std::cout << "  Замеров ещё не было. Сначала выберите пункт 5.\n";
        return;
    }
    PrintResults(lastResults);
    if (SaveResultsCsv(lastResults, "results.csv")) {
        DrawPlot("results.csv", "benchmark.png");
    }
}

}

int main() {
    try {
        while (true) {
            std::cout << "\n===== ЛР-1: AVL-дерево и словарь =====\n"
                      << "1. Дерево целых чисел\n"
                      << "2. Дерево вещественных чисел\n"
                      << "3. Дерево строк\n"
                      << "4. Словарь (слово -> перевод)\n"
                      << "5. Замеры скорости (таблица + график benchmark.png через Python)\n"
                      << "6. Показать последние замеры и перерисовать график\n"
                      << "0. Выход\n";
            switch (ReadNumber("> ", 0, 6)) {
                case 0: return 0;
                case 1: TreeMenu<int>().Run(); break;
                case 2: TreeMenu<double>().Run(); break;
                case 3: TreeMenu<std::string>().Run(); break;
                case 4: DictionaryMenu().Run(); break;
                case 5: BenchmarkMenu(); break;
                case 6: ChartsMenu(); break;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "\nВыход: " << e.what() << "\n";
        return 1;
    }
}
