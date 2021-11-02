#pragma once


#include <iostream>
#include <thread>

using namespace std;

void test()
{
	thread t([] {
		cout << "Hello World from lambda thread." << endl;
		});
	t.join();
}
