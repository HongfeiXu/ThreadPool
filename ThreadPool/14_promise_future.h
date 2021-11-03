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

// concurrent_task����ֱ�ӷ��ؼ�����������������һ��promise��������Ž����
void concurrent_task(int min, int max, promise<double>* result)
{
	vector<future<double>> results;	// ����һ���������洢 future ���󣬽���������ȡ����Ľ��

	unsigned concurrent_count = thread::hardware_concurrency();

	for (int i = 0; i < concurrent_count; ++i)
	{
		// �������װ�� packaged_task������ concurrent_worker ����װ���������޷�ֱ�ӻ�ȡ�� return ��ֵ������Ҫͨ�� future ����ȡ
		packaged_task<double(int, int)>task(concurrent_worker);
		// ��ȡ��������� future ���󣬲�������뼯����
		results.push_back(task.get_future());

		int range = (max - min) * (i + 1) / concurrent_count + min;
		// ͨ��һ���µ��߳���ִ������
		thread t(std::move(task), min, range);
		t.detach();		// TODO: ?? Ϊɶ���� join������ join Ҫ���ܶ�
		min = range + 1;
	}
	cout << "threads create finish" << endl;
	double sum = 0;
	for (auto& r : results)
	{
		sum += r.get();	// ͨ�� future ���ϣ���ȡÿ������ļ������������ۼ�
	}
	// ������������֮�󣬽��ܽ�����õ�promise�����ϡ�һ�����������set_value�����������future����ͻ������
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
	// ͨ��sum.get_future().get()����ȡ���
	cout << "Concurrent task finish, " << ms << " ms consumed, Result: " << sum.get_future().get() << endl;
}

/*

threads create finish
concurrent_task finish
Concurrent task finish, 441 ms consumed, Result: 6.66667e+11

G:\Projects\CPP\ThreadPool\x64\Debug\ThreadPool.exe (process 34608) exited with code 0.
Press any key to close this window . . .


*/
