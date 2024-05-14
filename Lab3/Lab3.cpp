// Lab3.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>
#include <atomic>

using namespace std;

void foo(void)
{
	std::cout << "Thread start..." << std::endl;
	for (int i = 0; i < 10; ++i)
	{
		std::cout << "Thread id = " << std::this_thread::get_id()
			<< std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "Thread finish!" << std::endl;
	return;
} 

void task0() {
	std::thread myTh(foo);
	std::cout << "Main thread id = " << std::this_thread::get_id()
		<< std::endl;
	myTh.join();
}

void task0_2() {
	std::thread myTh(foo);
	std::cout << "Main thread id = " << std::this_thread::get_id()
		<< std::endl;
	myTh.detach();
}


void printEvenNumbers() {
	for (int i = 2; i <= 100; i += 2) {
		std::cout << "Thread 1: " << i << std::endl;
	}
}

void printOddNumbers() {
	for (int i = 1; i <= 100; i += 2) {
		std::cout << "Thread 2: " << i << std::endl;
	}
}

/*Возникает гонка за вывод*/
int task1() {
	std::thread t1(printEvenNumbers);
	std::thread t2(printOddNumbers);

	t1.join();
	t2.join();

	return 0;
}

void thread_function(int thread_id) {
	cout << "Thread " << thread_id << " is started\n";

	// Имитация сложных вычислений
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distrib(1000, 2000);
	for (int i = 0; i < 100; ++i) {
		this_thread::sleep_for(chrono::milliseconds(distrib(gen)));
		cout << "Thread " << thread_id << ": " << i + 1 << endl;
	}
}

int task2() {
	int p;
	cout << "Input count of threads (<10): ";
	cin >> p;

	if (p <= 0 || p > 10) {
		cout << "Wrong count of threads.\n";
		return 1;
	}

	// Создание и запуск потоков
	vector<thread> threads;
	for (int i = 0; i < p; ++i) {
		threads.push_back(thread(thread_function, i + 1));
	}

	// Ожидание завершения потоков
	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}

// Shared variable (atomic for guaranteed thread interaction)
atomic<int> shared_variable = 0;

// Mutex to ensure synchronized access to the shared variable
mutex mtx3;

void thread_function_task3(int thread_id) {
	while (true) {
		// Lock the mutex for exclusive access to the shared variable
		mtx3.lock();

		// Check the termination condition
		if (shared_variable > 100) {
			break;
		}

		// Increment the shared variable by the thread's index
		shared_variable += thread_id;
		this_thread::sleep_for(chrono::milliseconds(50));

		// Release the mutex before printing to the console
		mtx3.unlock();
		cout << "Thread " << thread_id << ": value of variable = " << shared_variable << endl;
	}
}

int task3() {
	int p;
	cout << "Enter the number of threads (no more than 10): ";
	cin >> p;

	if (p <= 0 || p > 10) {
		cout << "Invalid number of threads.\n";
		return 1;
	}

	// Create and start threads
	vector<thread> threads;
	for (int i = 0; i < p; ++i) {
		threads.push_back(thread(thread_function_task3, i + 1));
	}

	// Wait for threads to finish
	for (auto& thread : threads) {
		thread.join();
	}

	cout << "All threads finished. Value of the shared variable: " << shared_variable << endl;

	return 0;
}


bool is_prime(int num) {
	if (num <= 1) {
		return false;
	}
	for (int i = 2; i * i <= num; ++i) {
		if (num % i == 0) {
			return false;
		}
	}
	return true;
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

int task4() {
	int N;
	cout << "Enter the array size N: ";
	cin >> N;

	// Create a random array
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> distrib(100'000, 1'000'000);
	vector<int> arr(N);
	generate(arr.begin(), arr.end(), [&distrib, &gen]() { return distrib(gen); });

	// Measure the time of sequential processing
	auto start_time = chrono::high_resolution_clock::now();
	process_part(arr, 0, arr.size());
	auto end_time = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
	cout << "Time for sequential processing: " << duration.count() << " ms\n";

	// Test with different numbers of threads
	vector<int> num_threads = { 1, 2, 4, 8 };
	for (int threads : num_threads) {
		// Create a copy of the array
		vector<int> arr_copy = arr;

		// Measure the time of parallel processing
		start_time = chrono::high_resolution_clock::now();
		vector<thread> thread_pool;
		int chunk_size = N / threads;
		for (int i = 0; i < threads; ++i) {
			int start = i * chunk_size;
			int end = (i == threads - 1) ? N : start + chunk_size;
			thread_pool.push_back(thread(process_part, ref(arr_copy), start, end));
		}
		for (auto& t : thread_pool) {
			t.join();
		}
		end_time = chrono::high_resolution_clock::now();
		duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
		cout << "Time for parallel processing with " << threads << " threads: " << duration.count() << " ms\n";

		// Check the results
		if (std::equal(arr_copy.begin(), arr_copy.end(), arr.begin())) {
			cout << "Results of parallel and sequential processing match.\n";
		}
		else {
			cout << "Results of parallel and sequential processing do not match.\n";
		}
	}

	return 0;
}

int main() {

	//task0();
	//task0_2(); // поток отсоединился
	//task1();
	//task2();
	//task3();
	task4();

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
