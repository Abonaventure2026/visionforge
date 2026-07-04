#include "LogService.h"
#include <iostream>

CLogService& CLogService::GetInstance() {
    static CLogService instance;
    return instance;
}

void CLogService::Init(const std::string& logPath, LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) return;

    m_level = level;
    m_file.open(logPath, std::ios::out | std::ios::app);
    if (!m_file.is_open()) {
        std::cerr << "[LOG] 无法打开日志文件: " << logPath << std::endl;
    }
    m_initialized = true;
}

void CLogService::Debug(const std::string& msg) {
    if (m_level <= LOG_DEBUG) Write(LOG_DEBUG, msg);
}

void CLogService::Info(const std::string& msg) {
    if (m_level <= LOG_INFO) Write(LOG_INFO, msg);
}

void CLogService::Warn(const std::string& msg) {
    if (m_level <= LOG_WARN) Write(LOG_WARN, msg);
}

void CLogService::Error(const std::string& msg) {
    if (m_level <= LOG_ERROR) Write(LOG_ERROR, msg);
}

void CLogService::Write(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string line = GetTimestamp() + " [" + GetLevelStr(level) + "] " + msg + "\n";

    if (m_file.is_open()) {
        m_file << line;
        m_file.flush();
    }

    if (level >= LOG_WARN) {
        std::cerr << line;
    } else {
        std::cout << line;
    }
}

std::string CLogService::GetLevelStr(LogLevel level) const {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string CLogService::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
