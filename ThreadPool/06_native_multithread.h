#pragma once

#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

static const int MAX = 10e7;
static double sum = 0;

void worker(int min, int max)
{
	cout << "[thread-" << this_thread::get_id() << "] is waking up" << endl;
	for (int i = min; i <= max; ++i)
	{
		sum += sqrt(i);
	}
}

void serial_task(int min, int max)
{
	auto start_time = chrono::steady_clock::now();
	sum = 0;
	worker(min, max);
	auto end_time = chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	cout << "Serial task finish, " << ms << "ms consumed, Result: " << sum << endl;
}

void test_serial()
{
	serial_task(0, MAX);
}

/*

Serial task finish, 1914ms consumed, Result: 6.66667e+11

*/

void concurrent_task(int min, int max)
{
	auto start_time = chrono::steady_clock::now();

	unsigned concurrent_count = thread::hardware_concurrency();		// 当前硬件支持多少个线程并行执行
	vector<thread> threads;

	sum = 0;
	for (int t = 0; t < concurrent_count; ++t)
	{
		int range = max * (t + 1) / concurrent_count;
		threads.push_back(thread(worker, min, range));
		min = range + 1;
	}
	for (auto& t : threads)
	{
		t.join();
	}
	auto end_time = chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	cout << "Concurrent task finish, " << ms << "ms consumed, Result: " << sum << endl;
}

void test_concurrent()
{
	concurrent_task(0, MAX);
}

/*
性能并没有明显的提升。更严重的是，这里的结果是错误的。
原因：
	1. 高速缓存和系统主存可能存在不一致，某个结果计算后保存在处理器的高速缓存中了，但是没有同步到主存中，此时这个值对于其他处理器就是不可见的。
	2. sum+=sqrt(i)这条语句不是原子的，它其实是很多条指令的组合才能完成。
*/

/*

Concurrent task finish, 1631ms consumed, Result: 9.62738e+10

*/

