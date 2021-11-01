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

	unsigned concurrent_count = thread::hardware_concurrency();		// ��ǰӲ��֧�ֶ��ٸ��̲߳���ִ��
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
���ܲ�û�����Ե������������ص��ǣ�����Ľ���Ǵ���ġ�
ԭ��
	1. ���ٻ����ϵͳ������ܴ��ڲ�һ�£�ĳ���������󱣴��ڴ������ĸ��ٻ������ˣ�����û��ͬ���������У���ʱ���ֵ�����������������ǲ��ɼ��ġ�
	2. sum+=sqrt(i)������䲻��ԭ�ӵģ�����ʵ�Ǻܶ���ָ�����ϲ�����ɡ�
*/

/*

Concurrent task finish, 1631ms consumed, Result: 9.62738e+10

*/

