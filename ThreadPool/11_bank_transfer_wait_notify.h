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
		// Ϊ��������������ϣ���Ϊ�����������������������������
		unique_lock<mutex> aLock(mMoneyLock);
		// ͨ�������������еȴ�
		// 1. ��������lambda���ʽ�����жϣ������
		// 2. ��������㣬��˴�����������壬���õ�ǰ�̵߳ȴ�����������Ҫ��ֻ�����������������̻߳�ȡ�����壩
		mConditionVar.wait(aLock, [this, amount] {
			return mMoney + amount > 0;
			});
		mMoney += amount;
		// �����䶯��֪ͨ���������������ϵȴ��������̣߳�
		// ��ʱ���е���wait���̶߳����ٴλ��ѣ�Ȼ����ȡ����ֻ��һ����ȡ�������ٴ��ж������Ƿ����㡣
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
	// ���������������ڶ���̼߳�Э�� https://en.cppreference.com/w/cpp/thread/condition_variable
	condition_variable mConditionVar;
};


class Bank {
public:
	void AddAccount(Account* account)
	{
		mAccounts.insert(account);
	}

	// ��������ĵȴ���֪ͨ���ƣ�����Ͳ�����ȥ�������ݱ�����������
	void transferMoney(Account* accountA, Account* accountB, double amount)
	{
		if (accountA == accountB) return; // avoid deadlock in case of self transfer

		accountA->changeMoney(-amount);
		accountB->changeMoney(amount);
	}

	// ��ȡ�ܽ���ʱ���п��������� transferMoney�������õ����ܽ���
	// ִ��totalMoney��֮ǰ��Ҫ�ȴ�transferMoney��ɣ�
	// ִ��totalMoney��ʱ��Ҫ����transferMoney��ʼ
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
	// ��������ĵȴ���֪ͨ���ƣ������ٿ���ת��ʧ�ܵ������
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

// �����˻�֮������ת��
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

����������-������ģ�ͣ�������ת��������wait�����㲢��ɺ�notify_all
TODO: ��������һ�����⣨���������10_improved_bank_transfer.h��Ҳ�еģ���totalMoney��ʱ�򲻵���200����Ϊ
totalMoney �� transferMoney ���ܻύ��ִ�У���

*/

/*

Transfer 43.0067 from YYF to Fy , Bank totalMoney: 200
Transfer 67.0858 from Fy to YYF , Bank totalMoney: 200
Transfer 36.5551 from YYF to Fy , Bank totalMoney: 187.509
Transfer 98.3184 from YYF to Fy , Bank totalMoney: 200
Transfer 41.0657 from Fy to YYF , Bank totalMoney: 200

*/

