#pragma once

#include <iostream>
#include <future>
#include <vector>
#include <chrono>

using namespace std;

static const int MAX = 10e7;

double concurrent_worker(int min, int max)
{
	double sum = 0;
	for (int i = min; i <= max; ++i)
	{
		sum += sqrt(i);
	}

	return sum;
}

double concurrent_task(int min, int max)
{
	vector<future<double>> results;	// 创建一个集合来存储 future 对象，将用它来获取任务的结果

	unsigned concurrent_count = thread::hardware_concurrency();

	for (int i = 0; i < concurrent_count; ++i)
	{
		// 将任务包装成 packaged_task，由于 concurrent_worker 被包装成了任务，无法直接获取它 return 的值，而是要通过 future 来获取
		packaged_task<double(int, int)>task(concurrent_worker);
		// 获取任务关联的 future 对象，并将其存入集合中
		results.push_back(task.get_future());

		int range = (max-min) * (i + 1) / concurrent_count + min;
		// 通过一个新的线程来执行任务
		thread t(std::move(task), min, range);
		t.detach();		// ?? 为啥不用 join
		min = range + 1;
	}
	cout << "threads create finish" << endl;
	double sum = 0;
	for (auto& r : results)
	{
		sum += r.get();	// 通过 future 集合，获取每个任务的计算结果，将其累加
	}
	return sum;
}

void test()
{
	auto start_time = chrono::steady_clock::now();
	double r = concurrent_task(0, MAX);
	auto end_time = chrono::steady_clock::now();
	auto ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
	cout << "Concurrent task finish, " << ms << " ms consumed, Result: " << r << endl;
}

/*

threads create finish
Concurrent task finish, 453 ms consumed, Result: 6.66667e+11

G:\Projects\CPP\ThreadPool\x64\Debug\ThreadPool.exe (process 18980) exited with code 0.
Press any key to close this window . . .

*/
