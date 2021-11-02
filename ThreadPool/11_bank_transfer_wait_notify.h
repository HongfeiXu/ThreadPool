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
		// https://en.cppreference.com/w/cpp/thread/unique_lock
		// 为了与条件变量配合，因为条件变量会解锁和重新锁定互斥体
		unique_lock<mutex> aLock(mMoneyLock);
		// 通过条件变量进行等待
		// 1. 如果后面的lambda表达式满足判断，则继续
		// 2. 如果不满足，则此处会解锁互斥体，并让当前线程等待（解锁很重要，只有这样才能让其他线程获取互斥体）
		mConditionVar.wait(aLock, [this, amount] {
			return mMoney + amount > 0;
			});
		mMoney += amount;
		// 金额发生变动后，通知所有在条件变量上等待的其他线程，
		// 此时所有调用wait的线程都会再次唤醒，然后尝试取锁（只有一个能取到）并再次判断条件是否满足。
		mConditionVar.notify_all();
	}

	string getName()
	{
		return mName;
	}

	double getMoney()
	{
		return mMoney;
	}

private:
	string mName;
	double mMoney;
	mutex mMoneyLock;
	// 条件变量，用来在多个线程间协作 https://en.cppreference.com/w/cpp/thread/condition_variable
	condition_variable mConditionVar;
};


class Bank {
public:
	void AddAccount(Account* account)
	{
		mAccounts.insert(account);
	}

	// 有了上面的等待、通知机制，这里就不用再去考虑数据保护的问题了
	void transferMoney(Account* accountA, Account* accountB, double amount)
	{
		if (accountA == accountB) return; // avoid deadlock in case of self transfer

		accountA->changeMoney(-amount);
		accountB->changeMoney(amount);
	}

	// 获取总金额的时候，有可能正在跑 transferMoney，可能拿到的总金额不对
	// 执行totalMoney的之前，要等待transferMoney完成，
	// 执行totalMoney的时候，要阻塞transferMoney开始
	// ???
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

mutex sCoutLock;
void randomTransfer(Bank* bank, Account* accountA, Account* accountB)
{
	// 有了上面的等待、通知机制，不用再考虑转账失败的情况了
	while (true)
	{
		double randomMoney = ((double)rand() / RAND_MAX) * 100;
		{
			lock_guard<mutex> guard(sCoutLock);
			cout << "Transfer " << randomMoney << " from " << accountA->getName()
				<< " to " << accountB->getName()
				<< " , Bank totalMoney: " << bank->totalMoney() << endl;
		}
		bank->transferMoney(accountA, accountB, randomMoney);
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

走了生产者-消费者模型，不满足转账条件则wait，满足并完成后notify_all
TODO: 出现了另一个问题（这个问题在10_improved_bank_transfer.h中也有的），totalMoney有时候不等于200，因为
totalMoney 与 transferMoney 可能会交错执行？？

*/

/*

Transfer 43.0067 from YYF to Fy , Bank totalMoney: 200
Transfer 67.0858 from Fy to YYF , Bank totalMoney: 200
Transfer 36.5551 from YYF to Fy , Bank totalMoney: 187.509
Transfer 98.3184 from YYF to Fy , Bank totalMoney: 200
Transfer 41.0657 from Fy to YYF , Bank totalMoney: 200

*/

