#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <atomic>
#include <format>
#include <array>
#include <vector>

#include <benchmark/benchmark.h>

using namespace std;

long long compute_sum(int from, int to) {
    long long sum = 0;
    for (int i = from; i < to; ++i) {
        sum += i;
    }
    return sum;
}

long long compute_sum_parallel(int N, int countThread) {
    // Разбивка матрицы на части для каждого потока
    int chunkSize = N / countThread;

    // Создание пула потоков
    std::vector<std::thread> threads;

    std::mutex mx;
    long long sum = 0;
    for (int i = 0; i < countThread; ++i) {
        int start = i * chunkSize;
        int end = (i == countThread - 1) ? N : start + chunkSize;

        
        threads.push_back(std::thread([&, start, end]() {
            auto el = compute_sum(start, end);
            std::unique_lock<std::mutex> lock(mx);
            sum = sum + el;
            }));
    }
    // Ожидание завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
    return sum;
}



static void BM_Parallel(benchmark::State& state, int N, int numThreads) {

    for (auto _ : state) {
        auto sum = compute_sum_parallel(N, numThreads);
        benchmark::DoNotOptimize(sum);
    }
}



int main(int argc, char** argv) {
    for (int N : {1e7, 1e8, 1e9}) {
        for (int numThread : {1, 2, 4, 8}) {
            benchmark::RegisterBenchmark(std::format("BM_Parallel/size{}/threads{}", N, numThread),
                BM_Parallel,
                N,
                numThread)
                ->Unit(benchmark::kMillisecond);
        }
    }

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}