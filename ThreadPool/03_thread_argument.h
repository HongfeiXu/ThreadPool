#pragma once

#include <iostream>
#include <thread>
#include <string>

using namespace std;

void hello(string name) {
	cout << "Welcome to " << name << endl;
}

void test() {
	thread t(hello, "H72!");
	t.join();
}