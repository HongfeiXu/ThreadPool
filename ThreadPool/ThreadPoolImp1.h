#pragma once

// ref: https://github.com/kanade2010/ThreadPool
// 局限：仅支持void()类型的task，且输出结果好像不大对。。。


#include <vector>
#include <deque>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <cassert>
#include <chrono>
#include <sstream>

#include "util.h"

class ThreadPool {
public:
	typedef std::function<void()> task_t;

	ThreadPool(int init_size = 3): m_init_threads_size(init_size), m_mutex(), m_cond(), m_is_started(false)
	{
		start();
	}

	~ThreadPool()
	{
		if (m_is_started)
		{
			stop();
		}
	}

	void stop()
	{
		MY_LOG("ThreadPool::stop() stop.");
		{
			std::unique_lock<std::mutex> lock(m_mutex);		// locks the associated mutex by calling m_mutex.lock()
			m_is_started = false;
			m_cond.notify_all();
			MY_LOG("ThreadPool::stop() notify_all.");
		}
		for (threads_t::iterator it = m_threads.begin(); it != m_threads.end(); ++it)
		{
			(*it)->join();		// ???
			delete* it;
		}
		m_threads.clear();
	}

	void add_task(const task_t& task)	// thread safe
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_tasks.push_back(task);
		m_cond.notify_one();
	}

private:
	ThreadPool(const ThreadPool&) = delete;	// 禁止复制拷贝
	const ThreadPool& operator=(const ThreadPool&) = delete;

	bool is_started() { return m_is_started; }

	void start()
	{
		assert(m_threads.empty());
		m_is_started = true;
		m_threads.reserve(m_init_threads_size);
		for (int i = 0; i < m_init_threads_size; ++i)
		{
			m_threads.push_back(new std::thread(std::bind(&ThreadPool::thread_loop, this)));
		}
	}

	void thread_loop()
	{
		MY_LOG("thread_pool::threadLoop() tid : " + get_tid() + " start.");
		while (m_is_started)
		{
			task_t task = take();
			if (task)
			{
				task();
			}
		}
		MY_LOG("thread_pool::threadLoop() tid : " + get_tid() + " exit.");
	}

	task_t take()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		// always use a while-loop, due to spurious wakeup
		while (m_tasks.empty() && m_is_started)
		{
			MY_LOG("thread_pool::threadLoop() tid : " + get_tid() + " wait.");
			m_cond.wait(lock);
		}

		MY_LOG("thread_pool::threadLoop() tid : " + get_tid() + " wakeup.");


		task_t task;
		tasks_t::size_type size = m_tasks.size();
		if (!m_tasks.empty() && m_is_started)
		{
			task = m_tasks.front();
			m_tasks.pop_front();
			assert(size - 1 == m_tasks.size());	// ???
		}
		return task;
	}

	typedef std::vector<std::thread*> threads_t;
	typedef std::deque<task_t> tasks_t;

	int m_init_threads_size;

	threads_t m_threads;
	tasks_t m_tasks;

	std::mutex m_mutex;
	std::condition_variable m_cond;
	bool m_is_started;
};

std::mutex g_mutex;

void testFunc()
{
	for (int i = 1; i < 4; ++i)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::lock_guard<std::mutex> lock(g_mutex);
		std::cout << "testFunc() [" << i << "] at thread [ " << std::this_thread::get_id() << "] output" << std::endl;
	}
}

/*

#include "ThreadPoolImp1.h"

int main()
{
	ThreadPool thread_pool;

	for (int i = 0; i < 5; ++i)
	{
		thread_pool.add_task(testFunc);
	}

	getchar();

	return 0;
}

输出：

[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:90] thread_pool::threadLoop() tid : 4892 start.
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:90] thread_pool::threadLoop() tid : 27820 start.
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:90] thread_pool::threadLoop() tid : 32260 start.
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:113] thread_pool::threadLoop() tid : 4892 wakeup.
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:113] thread_pool::threadLoop() tid : 27820 wakeup.
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:113] thread_pool::threadLoop() tid : 32260 wakeup.
testFunc() [1] at thread [ 4892] output
testFunc() [1] at thread [ 32260] output
testFunc() [1] at thread [ 27820] output
testFunc() [2] at thread [ 4892] output
testFunc() [2] at thread [ 27820] output
testFunc() [2] at thread [ 32260] output
testFunc() [3] at thread [ 4892] output
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:113] thread_pool::threadLoop() tid : 4892 wakeup.
testFunc() [3] at thread [ 27820] output
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:113] thread_pool::threadLoop() tid : 27820 wakeup.
testFunc() [3] at thread [ 32260] output
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:109] thread_pool::threadLoop() tid : 32260 wait.
testFunc() [1] at thread [ 4892] output
testFunc() [1] at thread [ 27820] output
testFunc() [2] at thread [ 4892] output
testFunc() [2] at thread [ 27820] output
testFunc() [3] at thread [ 4892] output
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:109] thread_pool::threadLoop() tid : 4892 wait.
testFunc() [3] at thread [ 27820] output
[G:\Projects\CPP\ThreadPool\ThreadPool\ThreadPoolImp1.h:109] thread_pool::threadLoop() tid : 27820 wait.

后面的输出好像都不大对了，为啥只输出了2个

*/