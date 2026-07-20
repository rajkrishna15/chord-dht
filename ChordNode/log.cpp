#include "log.h"

#include <iostream>
#include <ctime>
#include <pthread.h>

namespace {

std::string g_ip = "";
long long int g_port = -1;
long long int g_nodeid = -1;
pthread_mutex_t g_logmutex = PTHREAD_MUTEX_INITIALIZER;

const char *levelname(LogLevel level){
	switch(level){
		case LogLevel::INFO:  return "INFO";
		case LogLevel::WARN:  return "WARN";
		case LogLevel::ERROR: return "ERROR";
	}
	return "INFO";
}

std::string timestamp(){
	time_t now = time(nullptr);
	char buf[32];
	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
	return std::string(buf);
}

} // namespace

void log_init(const std::string &ip, long long int port){
	pthread_mutex_lock(&g_logmutex);
	g_ip = ip;
	g_port = port;
	pthread_mutex_unlock(&g_logmutex);
}

void log_set_nodeid(long long int id){
	pthread_mutex_lock(&g_logmutex);
	g_nodeid = id;
	pthread_mutex_unlock(&g_logmutex);
}

void log_message(LogLevel level, const std::string &msg){
	pthread_mutex_lock(&g_logmutex);
	std::cout << "[" << timestamp() << "] [" << levelname(level) << "] [node "
	          << g_nodeid << " " << g_ip << ":" << g_port << "] " << msg << std::endl;
	pthread_mutex_unlock(&g_logmutex);
}
