#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <atomic>
#include <format>
#include <array>

#include <benchmark/benchmark.h>

std::vector<std::vector<double>> generateMatrix(int size) {
    std::vector<std::vector<double>> matrix(size, std::vector<double>(size));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
    return matrix;
}

// Функция для генерации случайного вектора
std::vector<double> generateVector(int size) {
    std::vector<double> vector(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    for (int i = 0; i < size; ++i) {
        vector[i] = dis(gen);
    }
    return vector;
}

// Последовательный алгоритм умножения матрицы на вектор
std::vector<double> multiplyMatrixVectorSequential(const std::vector<std::vector<double>>& matrix, const std::vector<double>& vector) {
    int size = matrix.size();
    std::vector<double> result(size);

    for (int i = 0; i < size; ++i) {
        result[i] = 0;
        for (int j = 0; j < size; ++j) {
            result[i] += matrix[i][j] * vector[j];
        }
    }

    return result;
}

// Параллельный алгоритм умножения матрицы на вектор
std::vector<double> multiplyMatrixVectorParallel(const std::vector<std::vector<double>>& matrix,
    const std::vector<double>& vector,
    int numThreads)
{
    int size = matrix.size();
    std::vector<double> result(size, 0.0);

    // Разбивка матрицы на части для каждого потока
    int chunkSize = size / numThreads;

    // Создание пула потоков
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? size : start + chunkSize;

        threads.push_back(std::thread([&, start, end]() {
            for (int row = start; row < end; ++row) {
                for (int col = 0; col < size; ++col) {
                    result[row] += matrix[row][col] * vector[col];
                }
            }
            }));
    }

    // Ожидание завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    return result;
}

static void BM_Parallel(benchmark::State& state, int N, int numThreads) {
    std::vector<std::vector<double>> matrix = generateMatrix(N);
    std::vector<double> vec = generateVector(N);

    for (auto _ : state) {
        auto res = multiplyMatrixVectorParallel(matrix, vec, numThreads);
        benchmark::DoNotOptimize(res);
    }
}

static void BM_Sequential(benchmark::State& state, int N) {
    std::vector<std::vector<double>> matrix = generateMatrix(N);
    std::vector<double> vec = generateVector(N);

    for (auto _ : state) {
        auto res = multiplyMatrixVectorSequential(matrix, vec);
        benchmark::DoNotOptimize(res);
    }
}

int main(int argc, char** argv) {
    for (int N : {5e3, 1e4, 2e4}) {
        for (int numThread : {2, 4, 8}) {
            benchmark::RegisterBenchmark(std::format("BM_Parallel/size{}/threads{}", N, numThread),
                BM_Parallel,
                N,
                numThread)
                ->Unit(benchmark::kMillisecond);
        }
        benchmark::RegisterBenchmark(std::format("BM_Sequential/size{}/threads1", N),
            BM_Sequential,
            N)
            ->Unit(benchmark::kMillisecond);
    }

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}