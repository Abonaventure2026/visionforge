#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARN = 2,
    LOG_ERROR = 3
};

class CLogService {
public:
    static CLogService& GetInstance();

    void Init(const std::string& logPath = "./logs/vision.log", LogLevel level = LOG_INFO);
    void SetLevel(LogLevel level) { m_level = level; }

    void Debug(const std::string& msg);
    void Info(const std::string& msg);
    void Warn(const std::string& msg);
    void Error(const std::string& msg);

private:
    CLogService() = default;
    ~CLogService() { if (m_file.is_open()) m_file.close(); }

    void Write(LogLevel level, const std::string& msg);
    std::string GetLevelStr(LogLevel level) const;
    std::string GetTimestamp() const;

    std::ofstream m_file;
    LogLevel m_level = LOG_INFO;
    std::mutex m_mutex;
    bool m_initialized = false;
};
