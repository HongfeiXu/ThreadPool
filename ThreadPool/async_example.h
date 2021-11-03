#pragma once

// ref: https://en.cppreference.com/w/cpp/thread/async

#include <future>
#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>
#include <mutex>

using namespace std;

std::mutex m;
struct X {
	void foo(int i, const std::string& str) {
		std::lock_guard<std::mutex> lk(m);
		std::cout << str << ' ' << i << '\n';
	}
	void bar(const std::string& str) {
		std::lock_guard<std::mutex> lk(m);
		std::cout << str << '\n';
	}
	int operator()(int i) {
		std::lock_guard<std::mutex> lk(m);
		std::cout << i << '\n';
		return i + 10;
	}
};

std::mutex coutMutex;

template <typename RandomIt>
int parallel_sum(RandomIt beg, RandomIt end)
{
	//{
	//	std::lock_guard<std::mutex> lk(coutMutex);
	//	cout << "thread id: " << this_thread::get_id() << endl;
	//}
	auto len = end - beg;
	if (len < 1000)
		return accumulate(beg, end, 0);
	RandomIt mid = beg + len / 2;
	auto handle = async(launch::async, parallel_sum<RandomIt>, mid, end);
	int sum = parallel_sum(beg, mid);
	return sum + handle.get();
}

void test()
{
	vector<int> v(10000, 1);
	cout << "The sum is " << parallel_sum(v.begin(), v.end()) << "\n";

	X x;
	// Calls (&x)->foo(42, "Hello") with default policy:
	// may print "Hello 42" concurrently or defer execution
	auto a1 = std::async(&X::foo, &x, 42, "Hello");
	// Calls x.bar("world!") with deferred policy
	// prints "world!" when a2.get() or a2.wait() is called
	auto a2 = std::async(std::launch::deferred, &X::bar, x, "world!");
	// Calls X()(43); with async policy
	// prints "43" concurrently
	auto a3 = std::async(std::launch::async, X(), 43);
	a2.wait();                     // prints "world!"
	std::cout << a3.get() << '\n'; // prints "53"
}

/*

TODO: 为啥这里的输出与 cppreference 中不一致

*/

/*

The sum is 10000
Hello 42
world!
43
53

G:\Projects\CPP\ThreadPool\x64\Debug\ThreadPool.exe (process 26836) exited with code 0.
Press any key to close this window . . .

*/

