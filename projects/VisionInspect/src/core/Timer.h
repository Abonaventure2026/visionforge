#pragma once
#include <chrono>
#include <string>
#include <iostream>

class Timer {
public:
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

    void Start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    double ElapsedMs() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    double ElapsedS() const {
        return ElapsedMs() / 1000.0;
    }

    static void Log(const std::string& module, double ms) {
        std::cout << "[TIMER] " << module << "耗时: " << ms << "ms" << std::endl;
        if (ms > 2000) {
            std::cout << "[WARN] " << module << "耗时超出2s阈值!" << std::endl;
        }
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
};
