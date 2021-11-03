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
		//exclusive.lock();	// ���ʹ�������ǰ������
		//sum += sqrt(i);
		//exclusive.unlock();	// ����������

		// RAII
		lock_guard<mutex> lk(exclusive);
		sum += sqrt(i);
	}
}

// ���ǻ��ָ�ÿ���̵߳�������ʵ�Ƕ����ģ��������ݵĴ����Ǻ�ʱ�ģ�
// ����ʵ�ⲿ���߼�ÿ���߳̿��Ե�������û��Ҫ������
// ֻ�������������ݵ�ʱ�����һ���������Ϳ����ˡ�
void concurrent_worker_opt(int min, int max)
{
	//cout << "[thread-" << this_thread::get_id() << "] is waking up" << endl;
	double tmp_sum = 0;		// ��һ���ֲ��������浱ǰ�̵߳Ĵ�����
	for (int i = min; i <= max; ++i)
	{
		tmp_sum += sqrt(i);
	}
	// �ڻ��ܵ�ʱ�����������
	//exclusive.lock();
	//sum += tmp_sum;
	//exclusive.unlock();

	// https://zh.cppreference.com/w/cpp/language/raii
	// �������Ч��������ʹ��
	lock_guard<mutex> lk(exclusive);
	sum += tmp_sum;
}

void concurrent_task(int min, int max)
{
	auto start_time = chrono::steady_clock::now();

	unsigned concurrent_count = thread::hardware_concurrency();		// ��ǰӲ��֧�ֶ��ٸ��̲߳���ִ��
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
������ˣ����ܲ�ܶ࣬��Ϊconcurrent_worker��Ƶ���ļ�������
*/

/*
Concurrent task finish, 24607ms consumed, Result: 6.66667e+11
*/

/*
����concurrent_worker_opt����������
*/

/*
Concurrent task finish, 444ms consumed, Result: 6.66667e+11
*/