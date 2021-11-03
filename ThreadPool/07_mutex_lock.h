#pragma once

#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

static const int MAX = 10e7;
static double sum = 0;

static mutex exclusive;

void concurrent_worker(int min, int max)
{
	cout << "[thread-" << this_thread::get_id() << "] is waking up" << endl;
	for (int i = min; i <= max; ++i)
	{
		//exclusive.lock();	// 访问共享数据前，加锁
		//sum += sqrt(i);
		//exclusive.unlock();	// 访问完后解锁

		// RAII
		lock_guard<mutex> lk(exclusive);
		sum += sqrt(i);
	}
}

// 我们划分给每个线程的数据其实是独立的，对于数据的处理是耗时的，
// 但其实这部分逻辑每个线程可以单独处理，没必要加锁。
// 只有在最后汇总数据的时候进行一次锁保护就可以了。
void concurrent_worker_opt(int min, int max)
{
	//cout << "[thread-" << this_thread::get_id() << "] is waking up" << endl;
	double tmp_sum = 0;		// 用一个局部变量保存当前线程的处理结果
	for (int i = min; i <= max; ++i)
	{
		tmp_sum += sqrt(i);
	}
	// 在汇总的时候进行锁保护
	//exclusive.lock();
	//sum += tmp_sum;
	//exclusive.unlock();

	// https://zh.cppreference.com/w/cpp/language/raii
	// 和上面等效，但建议使用
	lock_guard<mutex> lk(exclusive);
	sum += tmp_sum;
}

void concurrent_task(int min, int max)
{
	auto start_time = chrono::steady_clock::now();

	unsigned concurrent_count = thread::hardware_concurrency();		// 当前硬件支持多少个线程并行执行
	//cout << "concurrent_count = " << concurrent_count << endl;
	vector<thread> threads;

	sum = 0;
	for (int t = 0; t < concurrent_count; ++t)
	{
		int range = (max - min) * (t + 1) / concurrent_count + min;
		//threads.push_back(thread(concurrent_worker, min, range));
		threads.push_back(thread(concurrent_worker_opt, min, range));
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
结果对了，性能差很多，因为concurrent_worker中频繁的加锁解锁
*/

/*
Concurrent task finish, 24607ms consumed, Result: 6.66667e+11
*/

/*
改用concurrent_worker_opt，好起来了
*/

/*
Concurrent task finish, 444ms consumed, Result: 6.66667e+11
*/