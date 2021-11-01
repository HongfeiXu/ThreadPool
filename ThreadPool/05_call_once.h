#pragma once

#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

void init()
{
	cout << "[thread-" << this_thread::get_id() << "] is waking up" << endl;
	cout << "Initialing..." << endl;
}

void worker(once_flag* flag)
{
	call_once(*flag, init);
}

void test()
{
	once_flag flag;
	thread t1(worker, &flag);
	thread t2(worker, &flag);
	thread t3(worker, &flag);

	t1.join();
	t2.join();
	t3.join();
}


/*

[thread-31236] is waking up
Initialing...

G:\Projects\CPP\ThreadPool\Debug\ThreadPool.exe (process 32328) exited with code 0.
Press any key to close this window . . .

*/
