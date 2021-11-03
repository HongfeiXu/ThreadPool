#pragma once

#include <mutex>
#include <iostream>
#include <sstream>

// �������������
#define MY_LOG(msg) my_log(msg, __FILE__, __LINE__);
void my_log(const std::string& msg, const std::string& file, std::size_t line);

// ��ȡ��ǰthread_id���ַ���
static std::string get_tid()
{
	std::stringstream tmp;
	tmp << std::this_thread::get_id();
	return tmp.str();
}

