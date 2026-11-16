#include "Benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

#include "AvlTree.hpp"

namespace {

using Clock = std::chrono::steady_clock;

double ElapsedNs(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::nano>(end - start).count();
}

void Shuffle(DynamicArray<int>& values, std::mt19937& rng) {
    for (int i = values.GetSize() - 1; i > 0; --i) {
        int j = static_cast<int>(rng() % static_cast<unsigned>(i + 1));
        std::swap(values[i], values[j]);
    }
}

}

// Замер одиночных операций на дереве РОВНО из N элементов.
// 1) Строим дерево из N случайно перемешанных ЧЁТНЫХ чисел 0, 2, 4, ..., 2N-2 (не замеряется).
// 2) SINGLE_OPS раз:
//    а) вставка: берём случайное НЕЧЁТНОЕ число (его в дереве нет), засекаем только Add,
//       затем без замера удаляем его — в дереве снова ровно N элементов;
//    б) удаление: берём случайное существующее чётное число, засекаем только Remove,
//       затем без замера вставляем его обратно — в дереве снова ровно N элементов.
// Всё повторяется REPEATS раз на разных случайных деревьях, берётся среднее по всем замерам.
BenchmarkResult MeasureTree(long long size) {
    if (size < 1) {
        throw std::invalid_argument("Размер дерева должен быть положительным");
    }
    std::mt19937 rng(2024);
    int n = static_cast<int>(size);

    DynamicArray<int> values(n);
    for (int i = 0; i < n; ++i) {
        values[i] = 2 * i;
    }

    double insertTotal = 0;
    double removeTotal = 0;
    for (int repeat = 0; repeat < REPEATS; ++repeat) {
        Shuffle(values, rng);
        AvlTree<int> tree;
        for (int i = 0; i < n; ++i) {
            tree.Add(values[i]);
        }

        for (int k = 0; k < SINGLE_OPS; ++k) {
            int key = 2 * static_cast<int>(rng() % static_cast<unsigned>(n)) + 1;  // нечётное: его в дереве нет
            auto start = Clock::now();
            tree.Add(key);  // замеряем ОДНУ вставку в дерево из N элементов
            auto end = Clock::now();
            insertTotal += ElapsedNs(start, end);
            tree.Remove(key);  // без замера: снова ровно N

            int old = values[static_cast<int>(rng() % static_cast<unsigned>(n))];  // существующее чётное
            start = Clock::now();
            tree.Remove(old);  // замеряем ОДНО удаление из дерева из N элементов
            end = Clock::now();
            removeTotal += ElapsedNs(start, end);
            tree.Add(old);  // без замера: снова ровно N
        }

        if (tree.GetCount() != static_cast<std::size_t>(n)) {
            throw std::logic_error("После замеров размер дерева изменился");
        }
    }

    BenchmarkResult result;
    result.size = size;
    result.measurements = REPEATS * SINGLE_OPS;
    result.insertNs = insertTotal / result.measurements;
    result.removeNs = removeTotal / result.measurements;
    return result;
}

DynamicArray<BenchmarkResult> RunBenchmark(const DynamicArray<long long>& sizes) {
    DynamicArray<BenchmarkResult> results(sizes.GetSize());
    if (sizes.GetSize() > 0) {
        MeasureTree(sizes[0]);  // разогрев: первый прогон медленнее (процессор, кеш, память), его не считаем
    }
    for (int i = 0; i < sizes.GetSize(); ++i) {
        std::cout << "Замер N = " << sizes[i] << " ..." << std::endl;
        results[i] = MeasureTree(sizes[i]);
    }
    return results;
}

void PrintResults(const DynamicArray<BenchmarkResult>& results) {
    std::cout << "\nВремя ОДНОЙ операции на дереве из N элементов, нс\n"
              << "(" << REPEATS << " разных деревьев, на каждом " << SINGLE_OPS
              << " вставок и " << SINGLE_OPS << " удалений; дерево всегда ровно из N элементов; "
              << "каждая операция засекается отдельно, берётся среднее)\n";
    std::cout << "         N   замеров   вставка 1 эл.   удаление 1 эл.\n";
    std::cout << std::fixed << std::setprecision(1);
    for (int i = 0; i < results.GetSize(); ++i) {
        const BenchmarkResult& r = results[i];
        std::cout << std::setw(10) << r.size << std::setw(10) << r.measurements << std::setw(16) << r.insertNs
                  << std::setw(17) << r.removeNs << "\n";
    }
    std::cout.unsetf(std::ios::fixed);
}

bool SaveResultsCsv(const DynamicArray<BenchmarkResult>& results, const std::string& path) {
    std::ofstream out(path);
    if (!out) {
        std::cout << "  Не удалось записать " << path << "\n";
        return false;
    }
    out << "n,measurements,insert_ns,remove_ns\n";
    for (int i = 0; i < results.GetSize(); ++i) {
        const BenchmarkResult& r = results[i];
        out << r.size << "," << r.measurements << "," << r.insertNs << "," << r.removeNs << "\n";
    }
    std::cout << "  Результаты сохранены в " << path << "\n";
    return true;
}

bool DrawPlot(const std::string& csvPath, const std::string& pngPath) {
    if (!std::ifstream("plot.py")) {
        std::cout << "  Не найден plot.py: запускайте программу из папки лабы (там, где plot.py)\n";
        return false;
    }
    std::cout.flush();  // чтобы таблица вывелась раньше сообщения от Python
    std::string command = "python3 plot.py \"" + csvPath + "\" \"" + pngPath + "\"";
    if (std::system(command.c_str()) != 0) {
        std::cout << "  Не получилось построить график. Нужен Python 3 с matplotlib "
                     "(sudo apt install python3-matplotlib)\n";
        return false;
    }
    return true;
}
