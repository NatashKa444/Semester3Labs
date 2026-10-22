#include "Benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <new>
#include <string>

#include "ShrdPtr.hpp"
#include "UnqPtr.hpp"
#include "WeakPtr.hpp"

struct Point {
    long long x;
    long long y;
};

double CreateRaw(int n);
double CreateUnqPtr(int n);
double CreateStdUnique(int n);
double CreateShrdPtr(int n);
double CreateStdShared(int n);
double CopyShrdPtr(int n);
double CopyStdShared(int n);
double LockWeakPtr(int n);
double LockStdWeak(int n);

double Average(double (*test)(int), int n);
void PrintBar(const std::string& name, double value, double maxValue);
void PrintMemoryTable();

const int REPEATS = 10;

size_t g_allocatedBytes = 0;

void* operator new(size_t size) {
    g_allocatedBytes += size;
    void* memory = std::malloc(size == 0 ? 1 : size);
    if (memory == nullptr) {
        throw std::bad_alloc();
    }
    return memory;
}

void operator delete(void* memory) noexcept {
    std::free(memory);
}

void operator delete(void* memory, size_t) noexcept {
    std::free(memory);
}

using Clock = std::chrono::steady_clock;

double MsSince(Clock::time_point start) {
    std::chrono::duration<double, std::milli> passed = Clock::now() - start;
    return passed.count();
}

double CreateRaw(int n) {
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        Point* p = new Point{i, i};
        delete p;
    }
    return MsSince(start);
}

double CreateUnqPtr(int n) {
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        UnqPtr<Point> p = MakeUnique<Point>(Point{i, i});
    }
    return MsSince(start);
}

double CreateStdUnique(int n) {
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        std::unique_ptr<Point> p = std::make_unique<Point>(Point{i, i});
    }
    return MsSince(start);
}

double CreateShrdPtr(int n) {
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        ShrdPtr<Point> p = MakeShared<Point>(Point{i, i});
    }
    return MsSince(start);
}

double CreateStdShared(int n) {
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        std::shared_ptr<Point> p = std::make_shared<Point>(Point{i, i});
    }
    return MsSince(start);
}

double CopyShrdPtr(int n) {
    ShrdPtr<Point> original = MakeShared<Point>(Point{1, 1});
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        ShrdPtr<Point> copy = original;
    }
    return MsSince(start);
}

double CopyStdShared(int n) {
    std::shared_ptr<Point> original = std::make_shared<Point>(Point{1, 1});
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        std::shared_ptr<Point> copy = original;
    }
    return MsSince(start);
}

double LockWeakPtr(int n) {
    ShrdPtr<Point> shared = MakeShared<Point>(Point{1, 1});
    WeakPtr<Point> weak = shared;
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        ShrdPtr<Point> locked = weak.Lock();
    }
    return MsSince(start);
}

double LockStdWeak(int n) {
    std::shared_ptr<Point> shared = std::make_shared<Point>(Point{1, 1});
    std::weak_ptr<Point> weak = shared;
    Clock::time_point start = Clock::now();
    for (int i = 0; i < n; ++i) {
        std::shared_ptr<Point> locked = weak.lock();
    }
    return MsSince(start);
}

double Average(double (*test)(int), int n) {
    double sum = 0;
    for (int i = 0; i < REPEATS; ++i) {
        sum += test(n);
    }
    return sum / REPEATS;
}

void PrintBar(const std::string& name, double value, double maxValue) {
    int stars = (int)(value / maxValue * 20);
    std::cout << name << ": " << value << " мс  ";
    for (int i = 0; i < stars; ++i) {
        std::cout << "*";
    }
    std::cout << "\n";
}

void PrintMemoryTable() {
    std::cout << "\n--- Память на один объект (объект Point = 16 байт) ---\n";
    std::cout << "  вариант             сам указатель      в куче       всего\n";

    size_t heap = 0;

    g_allocatedBytes = 0;
    Point* raw = new Point{1, 1};
    heap = g_allocatedBytes;
    delete raw;
    std::cout << "  " << std::left << std::setw(17) << "raw" << std::right << std::setw(12)
              << sizeof(Point*) << std::setw(12) << heap << std::setw(12) << sizeof(Point*) + heap
              << "\n";

    g_allocatedBytes = 0;
    {
        UnqPtr<Point> p = MakeUnique<Point>(Point{1, 1});
        heap = g_allocatedBytes;
    }
    std::cout << "  " << std::left << std::setw(17) << "UnqPtr" << std::right << std::setw(12)
              << sizeof(UnqPtr<Point>) << std::setw(12) << heap << std::setw(12)
              << sizeof(UnqPtr<Point>) + heap << "\n";

    g_allocatedBytes = 0;
    {
        std::unique_ptr<Point> p = std::make_unique<Point>(Point{1, 1});
        heap = g_allocatedBytes;
    }
    std::cout << "  " << std::left << std::setw(17) << "std::unique_ptr" << std::right << std::setw(12)
              << sizeof(std::unique_ptr<Point>) << std::setw(12) << heap << std::setw(12)
              << sizeof(std::unique_ptr<Point>) + heap << "\n";

    g_allocatedBytes = 0;
    {
        ShrdPtr<Point> p = MakeShared<Point>(Point{1, 1});
        heap = g_allocatedBytes;
    }
    std::cout << "  " << std::left << std::setw(17) << "ShrdPtr" << std::right << std::setw(12)
              << sizeof(ShrdPtr<Point>) << std::setw(12) << heap << std::setw(12)
              << sizeof(ShrdPtr<Point>) + heap << "\n";

    g_allocatedBytes = 0;
    {
        std::shared_ptr<Point> p = std::make_shared<Point>(Point{1, 1});
        heap = g_allocatedBytes;
    }
    std::cout << "  " << std::left << std::setw(17) << "std::shared_ptr" << std::right << std::setw(12)
              << sizeof(std::shared_ptr<Point>) << std::setw(12) << heap << std::setw(12)
              << sizeof(std::shared_ptr<Point>) + heap << "\n";
}

void RunBenchmark(int maxN) {
    int sizes[] = {10, 100, 1000, 10000, 100000, 1000000, 10000000};

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\nКаждое время — среднее из " << REPEATS << " замеров.\n";

    std::cout << "\n--- 1. Создание и удаление N объектов (время, мс) ---\n";
    std::cout << std::setw(10) << "N" << std::setw(12) << "raw" << std::setw(12) << "UnqPtr"
              << std::setw(16) << "std::unique" << std::setw(12) << "ShrdPtr" << std::setw(16)
              << "std::shared" << "\n";
    for (int n : sizes) {
        if (n > maxN) {
            break;
        }
        std::cout << std::setw(10) << n << std::setw(12) << Average(CreateRaw, n) << std::setw(12)
                  << Average(CreateUnqPtr, n) << std::setw(16) << Average(CreateStdUnique, n) << std::setw(12)
                  << Average(CreateShrdPtr, n) << std::setw(16) << Average(CreateStdShared, n) << "\n";
    }

    std::cout << "\n--- 2. N копий одного указателя (время, мс) ---\n";
    std::cout << std::setw(10) << "N" << std::setw(12) << "ShrdPtr" << std::setw(18)
              << "std::shared_ptr" << "\n";
    for (int n : sizes) {
        if (n > maxN) {
            break;
        }
        std::cout << std::setw(10) << n << std::setw(12) << Average(CopyShrdPtr, n) << std::setw(18)
                  << Average(CopyStdShared, n) << "\n";
    }

    std::cout << "\n--- 3. N вызовов Lock у слабого указателя (время, мс) ---\n";
    std::cout << std::setw(10) << "N" << std::setw(12) << "WeakPtr" << std::setw(18)
              << "std::weak_ptr" << "\n";
    for (int n : sizes) {
        if (n > maxN) {
            break;
        }
        std::cout << std::setw(10) << n << std::setw(12) << Average(LockWeakPtr, n) << std::setw(18)
                  << Average(LockStdWeak, n) << "\n";
    }

    int n = 1000000;
    std::cout << "\n--- График: создание и удаление " << n << " объектов ---\n";
    double raw = Average(CreateRaw, n);
    double unq = Average(CreateUnqPtr, n);
    double stdUnq = Average(CreateStdUnique, n);
    double shrd = Average(CreateShrdPtr, n);
    double stdShrd = Average(CreateStdShared, n);
    double maxValue = std::max({raw, unq, stdUnq, shrd, stdShrd});
    PrintBar("raw", raw, maxValue);
    PrintBar("UnqPtr", unq, maxValue);
    PrintBar("std::unique_ptr", stdUnq, maxValue);
    PrintBar("ShrdPtr", shrd, maxValue);
    PrintBar("std::shared_ptr", stdShrd, maxValue);

    std::cout << "\n--- График: " << n << " копий указателя ---\n";
    double copy = Average(CopyShrdPtr, n);
    double stdCopy = Average(CopyStdShared, n);
    maxValue = std::max(copy, stdCopy);
    PrintBar("ShrdPtr", copy, maxValue);
    PrintBar("std::shared_ptr", stdCopy, maxValue);

    std::cout << "\n--- График: " << n << " вызовов Lock ---\n";
    double lock = Average(LockWeakPtr, n);
    double stdLock = Average(LockStdWeak, n);
    maxValue = std::max(lock, stdLock);
    PrintBar("WeakPtr", lock, maxValue);
    PrintBar("std::weak_ptr", stdLock, maxValue);

    PrintMemoryTable();
}
