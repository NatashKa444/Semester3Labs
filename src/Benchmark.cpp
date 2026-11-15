#include "Benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>

#include "AvlTree.hpp"
#include "Exceptions.hpp"

namespace {

double ElapsedNs(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count();
}

struct Stats {
    DynamicArray<double> times = DynamicArray<double>(REPEATS);

    void Add(double value, int index) { times[index] = value; }

    double Average() const {
        double sum = 0;
        for (int i = 0; i < times.GetSize(); ++i) {
            sum += times[i];
        }
        return sum / times.GetSize();
    }

    double Median() {
        std::sort(&times[0], &times[0] + times.GetSize());
        return times[times.GetSize() / 2];
    }

    double Min() const {
        double result = times[0];
        for (int i = 1; i < times.GetSize(); ++i) {
            result = std::min(result, times[i]);
        }
        return result;
    }
};

}

BenchmarkResult MeasureTree(long long size) {
    std::mt19937 rng(2024);
    DynamicArray<int> values(static_cast<int>(size));
    for (int i = 0; i < values.GetSize(); ++i) {
        values[i] = 2 * i;
    }
    for (int i = values.GetSize() - 1; i > 0; --i) {
        int j = static_cast<int>(rng() % (i + 1));
        std::swap(values[i], values[j]);
    }

    AvlTree<int> tree;
    for (int i = 0; i < values.GetSize(); ++i) {
        tree.Add(values[i]);
    }

    Stats add;
    Stats remove;
    for (int repeat = 0; repeat < REPEATS; ++repeat) {
        int newValue = 2 * static_cast<int>(rng() % (size + 1)) - 1;
        auto start = std::chrono::steady_clock::now();
        tree.Add(newValue);
        add.Add(ElapsedNs(start), repeat);

        int oldValue = values[static_cast<int>(rng() % size)];
        start = std::chrono::steady_clock::now();
        tree.Remove(oldValue);
        remove.Add(ElapsedNs(start), repeat);

        tree.Add(oldValue);
        tree.Remove(newValue);
    }
    if (static_cast<long long>(tree.GetCount()) != size) {
        throw std::logic_error("После замеров размер дерева изменился");
    }

    BenchmarkResult result;
    result.size = size;
    result.addAvgNs = add.Average();
    result.addMedianNs = add.Median();
    result.addMinNs = add.Min();
    result.removeAvgNs = remove.Average();
    result.removeMedianNs = remove.Median();
    result.removeMinNs = remove.Min();
    return result;
}

DynamicArray<BenchmarkResult> RunBenchmark(const DynamicArray<long long>& sizes) {
    DynamicArray<BenchmarkResult> results(sizes.GetSize());
    for (int i = 0; i < sizes.GetSize(); ++i) {
        std::cout << "Замер N = " << sizes[i] << " ..." << std::endl;
        results[i] = MeasureTree(sizes[i]);
    }
    return results;
}

void PrintResults(const DynamicArray<BenchmarkResult>& results) {
    std::cout << "\nВремя ОДНОЙ операции в дереве из N элементов, нс (" << REPEATS << " замеров на каждое N)\n";
    std::cout << "              |      вставка одного элемента      |      удаление одного элемента\n";
    std::cout << "         N    |   медиана   среднее      мин      |   медиана   среднее      мин\n";
    std::cout << std::fixed << std::setprecision(1);
    for (int i = 0; i < results.GetSize(); ++i) {
        const BenchmarkResult& r = results[i];
        std::cout << std::setw(10) << r.size << "    |" << std::setw(10) << r.addMedianNs << std::setw(10)
                  << r.addAvgNs << std::setw(9) << r.addMinNs << "      |" << std::setw(10) << r.removeMedianNs
                  << std::setw(10) << r.removeAvgNs << std::setw(9) << r.removeMinNs << "\n";
    }
    std::cout.unsetf(std::ios::fixed);
}

void SaveResultsCsv(const DynamicArray<BenchmarkResult>& results, const std::string& path) {
    std::ofstream out(path);
    if (!out) {
        throw TreeException("Не удалось открыть файл " + path);
    }
    out << "N,add_median_ns,add_avg_ns,add_min_ns,remove_median_ns,remove_avg_ns,remove_min_ns\n";
    for (int i = 0; i < results.GetSize(); ++i) {
        const BenchmarkResult& r = results[i];
        out << r.size << "," << r.addMedianNs << "," << r.addAvgNs << "," << r.addMinNs << ","
            << r.removeMedianNs << "," << r.removeAvgNs << "," << r.removeMinNs << "\n";
    }
}
