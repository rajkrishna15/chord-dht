#ifndef __log_H_INCLUDED__
#define __log_H_INCLUDED__

#include <string>
#include <sstream>

enum class LogLevel { INFO, WARN, ERROR };

void log_init(const std::string &ip, long long int port);
void log_set_nodeid(long long int id);
void log_message(LogLevel level, const std::string &msg);

#define LOG_INFO(x)  do { std::ostringstream _oss; _oss << x; log_message(LogLevel::INFO,  _oss.str()); } while(0)
#define LOG_WARN(x)  do { std::ostringstream _oss; _oss << x; log_message(LogLevel::WARN,  _oss.str()); } while(0)
#define LOG_ERROR(x) do { std::ostringstream _oss; _oss << x; log_message(LogLevel::ERROR, _oss.str()); } while(0)

#endif
