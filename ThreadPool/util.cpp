#include "util.h"

std::mutex log_mutex;
void my_log(const std::string& msg, const std::string& file, std::size_t line)
{
	std::lock_guard<std::mutex> lock(log_mutex);
	std::cout << "[" << file << ":" << line << "] " << msg << std::endl;
}

