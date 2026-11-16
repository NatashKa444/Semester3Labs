#pragma once

#include <string>

#include "DynamicArray.hpp"

// Результат замера для одного размера дерева N
struct BenchmarkResult {
    long long size = 0;     // N — сколько элементов в дереве
    int measurements = 0;   // сколько раз замерили одну операцию (всего по всем повторам)
    double insertNs = 0;    // среднее время вставки ОДНОГО элемента в дерево из N элементов, нс
    double removeNs = 0;    // среднее время удаления ОДНОГО элемента из дерева из N элементов, нс
};

const int REPEATS = 10;        // сколько раз заново строим случайное дерево размера N
const int SINGLE_OPS = 1000;   // сколько одиночных вставок и удалений замеряем на каждом дереве

BenchmarkResult MeasureTree(long long size);
DynamicArray<BenchmarkResult> RunBenchmark(const DynamicArray<long long>& sizes);
void PrintResults(const DynamicArray<BenchmarkResult>& results);
bool SaveResultsCsv(const DynamicArray<BenchmarkResult>& results, const std::string& path);
bool DrawPlot(const std::string& csvPath, const std::string& pngPath);
