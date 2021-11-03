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

// concurrent_task不再直接返回计算结果，而是增加了一个promise对象来存放结果。
void concurrent_task(int min, int max, promise<double>* result)
{
	vector<future<double>> results;	// 创建一个集合来存储 future 对象，将用它来获取任务的结果

	unsigned concurrent_count = thread::hardware_concurrency();

	for (int i = 0; i < concurrent_count; ++i)
	{
		// 将任务包装成 packaged_task，由于 concurrent_worker 被包装成了任务，无法直接获取它 return 的值，而是要通过 future 来获取
		packaged_task<double(int, int)>task(concurrent_worker);
		// 获取任务关联的 future 对象，并将其存入集合中
		results.push_back(task.get_future());

		int range = (max - min) * (i + 1) / concurrent_count + min;
		// 通过一个新的线程来执行任务
		thread t(std::move(task), min, range);
		t.detach();		// TODO: ?? 为啥不用 join，发现 join 要慢很多
		min = range + 1;
	}
	cout << "threads create finish" << endl;
	double sum = 0;
	for (auto& r : results)
	{
		sum += r.get();	// 通过 future 集合，获取每个任务的计算结果，将其累加
	}
	// 在任务计算完成之后，将总结过设置到promise对象上。一旦这里调用了set_value，其相关联的future对象就会就绪。
	result->set_value(sum);
	cout << "concurrent_task finish" << endl;
}

void test()
{
	promise<double> sum;

	auto start_time = chrono::steady_clock::now();
	concurrent_task(0, MAX, &sum);
	auto end_time = chrono::steady_clock::now();
	auto ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
	// 通过sum.get_future().get()来获取结果
	cout << "Concurrent task finish, " << ms << " ms consumed, Result: " << sum.get_future().get() << endl;
}

/*

threads create finish
concurrent_task finish
Concurrent task finish, 441 ms consumed, Result: 6.66667e+11

G:\Projects\CPP\ThreadPool\x64\Debug\ThreadPool.exe (process 34608) exited with code 0.
Press any key to close this window . . .


*/
