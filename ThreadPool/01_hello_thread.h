#pragma once

#include <iostream>
#include <thread>

using namespace std;

void hello()
{
	cout << "Hello world from new thread." << endl;
}

void test()
{
	thread t(hello);
	t.join();
}


/*

Hello world from new thread.

G:\Projects\CPP\ThreadPool\x64\Debug\ThreadPool.exe (process 22140) exited with code 0.
Press any key to close this window . . .

*/

