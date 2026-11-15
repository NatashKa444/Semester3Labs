#pragma once

#include <string>

#include "DynamicArray.hpp"

struct BenchmarkResult {
    long long size = 0;
    double addMedianNs = 0;
    double addAvgNs = 0;
    double addMinNs = 0;
    double removeMedianNs = 0;
    double removeAvgNs = 0;
    double removeMinNs = 0;
};

const int REPEATS = 1000;

BenchmarkResult MeasureTree(long long size);
DynamicArray<BenchmarkResult> RunBenchmark(const DynamicArray<long long>& sizes);
void PrintResults(const DynamicArray<BenchmarkResult>& results);
void SaveResultsCsv(const DynamicArray<BenchmarkResult>& results, const std::string& path);
