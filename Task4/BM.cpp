#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <atomic>
#include <format>
#include <array>

#include <benchmark/benchmark.h>

using namespace std;

const int n = 100'000;
constexpr std::array<bool, n + 1> sieve_of_eratosthenes() {
	std::array<bool, n+1> is_prime{};
	is_prime.fill(true);
	is_prime[0] = is_prime[1] = false;
	for (int i = 2; i * i <= n; ++i) {
		if (is_prime[i]) {
			for (int j = i * i; j <= n; j += i) {
				is_prime[j] = false;
			}
		}
	}
	return is_prime;
}

// Статическая переменная для хранения результатов решета
const std::array<bool, n + 1> primes = sieve_of_eratosthenes();

// Оптимизированная функция is_prime
constexpr bool is_prime(int num) {
	return primes[num];
}


// Function to find the greatest prime divisor
int find_greatest_prime_divisor(int num) {
	if (num <= 1) {
		return 1;
	}
	for (int i = num - 1; i > 1; --i) {
		if (num % i == 0 && is_prime(i)) {
			return i;
		}
	}
	return num;
}

// Function to process a part of the array in a separate thread
void process_part(vector<int>& arr, int start, int end) {
	for (int i = start; i < end; ++i) {
		arr[i] = find_greatest_prime_divisor(arr[i]);
	}
}


static void BM_task4(benchmark::State& state, int N, int countThreads) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distrib(10, 100);
	vector<int> arr(N);
	generate(arr.begin(), arr.end(), [&distrib, &gen]() { return distrib(gen); });

	for (auto _ : state) {

		vector<thread> thread_pool(countThreads);
		int chunk_size = N / countThreads;
		for (int i = 0; i < countThreads; ++i) {
			int start = i * chunk_size;
			int end = (i == countThreads - 1) ? N : start + chunk_size;
			thread_pool[i] = thread(process_part, ref(arr), start, end);
		}
		for (auto& t : thread_pool) {
			t.join();
		}
		benchmark::DoNotOptimize(arr);
	}
}

static void BM_task4_1thread(benchmark::State& state, int N) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distrib(10, 100);
	vector<int> arr(N);
	generate(arr.begin(), arr.end(), [&distrib, &gen]() { return distrib(gen); });

	for (auto _ : state) {

		process_part(arr, 0, arr.size() - 1);
		benchmark::DoNotOptimize(arr);
	}
}

int main(int argc, char** argv) {
	

	for (int N : {2e7, 5e7, 1e8}) {
		for (int countThreads : {2, 4, 8}) {
			benchmark::RegisterBenchmark(std::format("BM_CountingSort/size{}/threads{}", N, countThreads),
				BM_task4,
				N,
				countThreads)
				->Unit(benchmark::kMillisecond);
		}
		benchmark::RegisterBenchmark(std::format("BM_CountingSort/size{}/threads1", N),
			BM_task4_1thread,
			N)
			->Unit(benchmark::kMillisecond);
	}

	

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();

	return 0;
}