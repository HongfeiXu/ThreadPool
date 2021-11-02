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
		if (accountA == accountB) return false; // avoid deadlock in case of self transfer

		// https://en.cppreference.com/w/cpp/thread/lock
		// https://en.cppreference.com/w/cpp/thread/lock_tag
		// https://en.cppreference.com/w/cpp/thread/lock_guard

		// lock both mutexes without deadlock
		lock(*accountA->getLock(), *accountB->getLock());
		// make sure both already-locked mutexes are unlocked at the end of scope
		lock_guard<mutex> lockA(*accountA->getLock(), adopt_lock);
		lock_guard<mutex> lockB(*accountB->getLock(), adopt_lock);

// equivalent approach
		//unique_lock<mutex> lockA(*accountA->getLock(), defer_lock);
		//unique_lock<mutex> lockB(*accountB->getLock(), defer_lock);
		//lock(*accountA->getLock(), *accountB->getLock());

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

mutex sCoutLock;
void randomTransfer(Bank* bank, Account* accountA, Account* accountB)
{
	while (true)
	{
		double randomMoney = ((double)rand() / RAND_MAX) * 100;
		if (bank->transferMoney(accountA, accountB, randomMoney))
		{
			sCoutLock.lock();	// 保护输出逻辑的原子性
			cout << "Transfer " << randomMoney << " from " << accountA->getName()
				<< " to " << accountB->getName()
				<< " , Bank totalMoney: " << bank->totalMoney() << endl;
			sCoutLock.unlock();
		}
		else
		{
			const std::lock_guard<std::mutex> lock(sCoutLock);	// RAII，与上面的lock unlock效果一致
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
再也不会死锁了，但有个问题是失败的时候不会等待；
这在很多业务中是很常见的一个需求：每一次操作都要正确执行，如果条件不满足就停下来等待，直到条件满足之后再继续。而不是直接返回。
即生产者-消费者模型
*/

/*

Transfer failed, YYF has only $0.149541, but 45.6618 required
Transfer failed, YYF has only $0.149541, but 0.375378 required
Transfer failed, YYF has only $0.149541, but 75.0633 required
Transfer failed, YYF has only $0.149541, but 11.4048 required
Transfer failed, YYF has only $0.149541, but 40.4706 required
Transfer failed, YYF has only $0.149541, but 31.1136 required
Transfer failed, YYF has only $0.149541, but 99.2615 required
Transfer failed, YYF has only $0.149541, but 3.85754 required
Transfer failed, YYF has only $0.149541, but 25.2083 required
Transfer failed, YYF has only $0.149541, but 18.9276 required
Transfer failed, YYF has only $0.149541, but 24.7688 required
Transfer failed, YYF has only $0.149541, but 15.3508 required

*/

