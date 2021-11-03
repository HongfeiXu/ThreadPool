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
	vector<future<double>> results;	// ����һ���������洢 future ���󣬽���������ȡ����Ľ��

	unsigned concurrent_count = thread::hardware_concurrency();

	for (int i = 0; i < concurrent_count; ++i)
	{
		// �������װ�� packaged_task������ concurrent_worker ����װ���������޷�ֱ�ӻ�ȡ�� return ��ֵ������Ҫͨ�� future ����ȡ
		packaged_task<double(int, int)>task(concurrent_worker);
		// ��ȡ��������� future ���󣬲�������뼯����
		results.push_back(task.get_future());

		int range = (max-min) * (i + 1) / concurrent_count + min;
		// ͨ��һ���µ��߳���ִ������
		thread t(std::move(task), min, range);
		t.detach();		// ?? Ϊɶ���� join
		min = range + 1;
	}
	cout << "threads create finish" << endl;
	double sum = 0;
	for (auto& r : results)
	{
		sum += r.get();	// ͨ�� future ���ϣ���ȡÿ������ļ������������ۼ�
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
