#pragma once

#include <future>
#include <iostream>
#include <thread>

using namespace std;

static const int MAX = 10e7;
static double sum = 0;

void worker(int min, int max)
{
	cout << "task in thread: " << this_thread::get_id() << endl;
	for (int i = min; i <= max; ++i)
	{
		sum += sqrt(i);
	}
}

void test()
{
	sum = 0;
	cout << "Async task triggered, thread: " << this_thread::get_id() << endl;
	auto f1 = async(worker, 0, MAX);	// ���첽�ķ�ʽ����������
	f1.wait();	// �ȴ��첽����ִ�����
	cout << "Async task finish, result: " << sum << endl << endl;
}

/*

Async task triggered, thread: 106988
task in thread: 76220
Async task finish, result: 6.66667e+11


E:\BooksLearn\CPP\ThreadPool\x64\Debug\ThreadPool.exe (���� 106732)���˳�������Ϊ 0��
��������رմ˴���. . .

*/

void test2()
{
	double result = 0;
	cout << "Async task with lambda triggered, thread: " << this_thread::get_id() << endl;
	// ͨ�� launch::async ��ȷָ��Ҫͨ���������߳���ִ������
	auto f2 = async(launch::async, [&result]() {
		cout << "Lambda task in thread: " << this_thread::get_id() << endl;
		for (int i = 0; i <= MAX; ++i)
		{
			result += sqrt(i);
		}
	});
	f2.wait();
	cout << "Async task with lambda finish, result: " << result << endl << endl;
}

/*

Async task with lambda triggered, thread: 98404
Lambda task in thread: 105192
Async task with lambda finish, result: 6.66667e+11


E:\BooksLearn\CPP\ThreadPool\x64\Debug\ThreadPool.exe (���� 92968)���˳�������Ϊ 0��
��������رմ˴���. . .

*/

// ----------------------------------------------------------------------- //
// ���������������˵���ܶ�ʱ��϶�ϣ���Զ���ķ�����ָ���첽����������һ��ʾ����
// ----------------------------------------------------------------------- //

// ��Worker���������񣬰���������Ĳ�����������
class Worker {
public:
	Worker(int min, int max) : mMin(min), mMax(max) {}

	// ����������߼�
	double worker()
	{
		mResult = 0;
		for (int i = mMin; i <= mMax; ++i)
		{
			mResult += sqrt(i);
		}
		return mResult;
	}

	double getResult()
	{
		return mResult;
	}

private:
	int mMin;
	int mMax;
	double mResult;
};


void test3()
{
	Worker w(0, MAX);
	cout << "Task in class triggered, thread: " << this_thread::get_id() << endl;
	// ע�����ﴫ�ݵ��Ƕ����ָ��&w�������д&������w�������ʱ���ơ�
	auto f3 = async(launch::async, &Worker::worker, &w);
	f3.wait();
	cout << "Task in class finish, result: " << w.getResult() << endl << endl;
}


/*

Task in class triggered, thread: 94788
Task in class finish, result: 6.66667e+11


E:\BooksLearn\CPP\ThreadPool\x64\Debug\ThreadPool.exe (���� 95340)���˳�������Ϊ 0��
��������رմ˴���. . .

*/
