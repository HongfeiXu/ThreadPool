#pragma once

#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <set>

using namespace std;

class Account {
public:
	Account(string name, double money) : mName(name), mMoney(money) {};

	void changeMoney(double amount)
	{
		mMoney += amount;
	}

	string getName()
	{
		return mName;
	}

	double getMoney()
	{
		return mMoney;
	}

	mutex* getLock()
	{
		return &mMoneyLock;
	}

private:
	string mName;
	double mMoney;
	mutex mMoneyLock;
};


class Bank {
public:
	void AddAccount(Account* account)
	{
		mAccounts.insert(account);
	}
	
	bool transferMoney(Account* accountA, Account* accountB, double amount)
	{
		// https://en.cppreference.com/w/cpp/thread/lock_guard
		// lock_guard 会在自身对象生命周期的范围内锁定互斥体
		lock_guard<mutex> guardA(*accountA->getLock());
		lock_guard<mutex> guardB(*accountB->getLock());

		if (amount > accountA->getMoney())
		{
			return false;
		}
		
		accountA->changeMoney(-amount);
		accountB->changeMoney(amount);
		return true;
	}

	double totalMoney() const 
	{
		double sum = 0;
		for (auto a : mAccounts)
		{
			sum += a->getMoney();
		}
		return sum;
	}

private:
	set<Account*> mAccounts;
};

void randomTransfer(Bank* bank, Account* accountA, Account* accountB)
{
	while (true)
	{
		double randomMoney = ((double)rand() / RAND_MAX) * 100;
		if (bank->transferMoney(accountA, accountB, randomMoney))
		{
			cout << "Transfer " << randomMoney << " from " << accountA->getName()
				<< " to " << accountB->getName()
				<< " , Bank totalMoney: " << bank->totalMoney() << endl;
		}
		else
		{
			cout << "Transfer failed, "
				<< accountA->getName() << " has only $" << accountA->getMoney() << ", but "
				<< randomMoney << " required" << endl;
		}
	}
}

// 两个账户之间来回转账
void test()
{
	Account a("Fy", 100);
	Account b("YYF", 100);

	Bank aBank;
	aBank.AddAccount(&a);
	aBank.AddAccount(&b);

	thread t1(randomTransfer, &aBank, &a, &b);
	thread t2(randomTransfer, &aBank, &b, &a);

	t1.join();
	t2.join();
}

/*
程序运行一会后，不动了，说明发生了死锁，这两个线程可能会同时获取其中一个账号的锁，然后又想获取另外一个账号的锁，此时就发生了死锁；
输出是乱的，两个线程的输出混在了一起，因为两个线程会同时输出，没有做好隔离

*/

/*

Transfer 40.3119 from Fy to YYF , Bank totalMoney: 200
Transfer 11.6428 from YYF to Fy , Bank totalMoney: 200
Transfer failed, Fy has only $61.3086, but 78.8476 required
Transfer 64.095 from YYF to Fy , Bank totalMoney: 200
Transfer 60.979 from Fy to YYF , Bank totalMoney: 200
Transfer 73.3695 from YYF to Fy , Bank totalMoney: 200
Transfer 63.9027 from Fy to YYF , Bank totalMoney: 200
Transfer 74.7246 from YYF to Fy , Bank totalMoney: 200
Transfer 29.5999 from Fy to YYF , Bank totalMoney: 200
Transfer 91.8638 from Fy to YYF , Bank totalMoney: 200
Transfer 45.97 from YYF to Fy , Bank totalMoney: 200
Transfer 28.7088 from Transfer 67.7847 from YYF to Fy , Bank totalMoney: 200
Fy to YYF , Bank totalMoney: 200
Transfer 61.2384 from YYF to Fy , Bank totalMoney: 200

*/

