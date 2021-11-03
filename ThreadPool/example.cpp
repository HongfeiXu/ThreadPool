#include <iostream>
#include <random>
#include "ThreadPool.h"

// ref: https://github.com/mtrebi/thread-pool#usage-example

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-1000, 1000);
auto rnd = std::bind(dist, mt);


void simulate_hard_computation() {
	std::this_thread::sleep_for(std::chrono::milliseconds(100 + rnd()));
}

// Simple function that adds multiplies two numbers and prints the result
void multiply(const int a, const int b) {
	simulate_hard_computation();
	const int res = a * b;
	std::cout << a << " * " << b << " = " << res << std::endl;
}

// Same as before but now we have an output parameter
void multiply_output(int& out, const int a, const int b) {
	simulate_hard_computation();
	out = a * b;
	std::cout << a << " * " << b << " = " << out << std::endl;
}

// Same as before but now we have an output parameter
int multiply_return(const int a, const int b) {
	simulate_hard_computation();
	const int res = a * b;
	std::cout << a << " * " << b << " = " << res << std::endl;
	return res;
}


int main(int argc, char* argv[])
{
	// Create pool with 3 threads
	ThreadPool pool(3);

	// Initialize pool
	pool.init();

	// Submit (partial) multiplication table
	for (int i = 1; i < 3; ++i) {
		for (int j = 1; j < 10; ++j) {
			pool.submit(multiply, i, j);
		}
	}

	// Submit function with output parameter passed by ref
	int output_ref;
	auto future1 = pool.submit(multiply_output, std::ref(output_ref), 5, 6);

	// Wait for multiplication output to finish
	future1.get();
	std::cout << "Last operation result is equals to " << output_ref << std::endl;

	// Submit function with return parameter 
	auto future2 = pool.submit(multiply_return, 5, 3);

	// Wait for multiplication output to finish
	int res = future2.get();
	std::cout << "Last operation result is equals to " << res << std::endl;

	pool.shutdown();

	return 0;
}

/*

1 * 1 = 1
1 * 4 = 4
1 * 2 = 2
1 * 6 = 6
1 * 7 = 7
1 * 5 = 5
1 * 9 = 9
2 * 1 = 2
1 * 8 = 8
2 * 3 = 6
2 * 4 = 8
2 * 5 = 10
2 * 6 = 12
2 * 2 = 4
2 * 8 = 16
2 * 7 = 14
5 * 6 = 30
Last operation result is equals to 30
1 * 3 = 3
2 * 9 = 18
5 * 3 = 15
Last operation result is equals to 15

G:\Projects\CPP\ThreadPool\x64\Debug\ThreadPool.exe (process 34632) exited with code 0.
Press any key to close this window . . .

*/
