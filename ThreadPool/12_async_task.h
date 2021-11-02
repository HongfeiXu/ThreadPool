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
	auto f1 = async(worker, 0, MAX);	// 以异步的方式启动了任务
	f1.wait();	// 等待异步任务执行完成
	cout << "Async task finish, result: " << sum << endl << endl;
}

/*

Async task triggered, thread: 106988
task in thread: 76220
Async task finish, result: 6.66667e+11


E:\BooksLearn\CPP\ThreadPool\x64\Debug\ThreadPool.exe (进程 106732)已退出，代码为 0。
按任意键关闭此窗口. . .

*/

void test2()
{
	double result = 0;
	cout << "Async task with lambda triggered, thread: " << this_thread::get_id() << endl;
	// 通过 launch::async 明确指定要通过独立的线程来执行任务
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


E:\BooksLearn\CPP\ThreadPool\x64\Debug\ThreadPool.exe (进程 92968)已退出，代码为 0。
按任意键关闭此窗口. . .

*/

// ----------------------------------------------------------------------- //
// 对于面向对象编程来说，很多时候肯定希望以对象的方法来指定异步任务。下面是一个示例：
// ----------------------------------------------------------------------- //

// 类Worker来描述任务，包含了任务的参数、输出结果
class Worker {
public:
	Worker(int min, int max) : mMin(min), mMax(max) {}

	// 任务的主体逻辑
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
	// 注：这里传递的是对象的指针&w，如果不写&将传入w对象的临时复制。
	auto f3 = async(launch::async, &Worker::worker, &w);
	f3.wait();
	cout << "Task in class finish, result: " << w.getResult() << endl << endl;
}


/*

Task in class triggered, thread: 94788
Task in class finish, result: 6.66667e+11


E:\BooksLearn\CPP\ThreadPool\x64\Debug\ThreadPool.exe (进程 95340)已退出，代码为 0。
按任意键关闭此窗口. . .

*/
