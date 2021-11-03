#pragma once

#include <mutex>
#include <iostream>
#include <sstream>

// 用锁来保护输出
#define MY_LOG(msg) my_log(msg, __FILE__, __LINE__);
void my_log(const std::string& msg, const std::string& file, std::size_t line);

// 获取当前thread_id的字符串
static std::string get_tid()
{
	std::stringstream tmp;
	tmp << std::this_thread::get_id();
	return tmp.str();
}

