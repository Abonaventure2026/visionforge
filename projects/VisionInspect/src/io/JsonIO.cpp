#include "JsonIO.h"
#include <fstream>
#include <iostream>
#include <sstream>

bool SaveJson(const std::string& path, const std::map<std::string, double>& data) {
    std::ofstream fout(path);
    if (!fout) {
        std::cout << "[JsonIO] 无法打开文件: " << path << std::endl;
        return false;
    }

    fout << "{\n";
    size_t count = 0;
    for (const auto& [key, value] : data) {
        fout << "  \"" << key << "\": " << value;
        if (++count < data.size()) fout << ",";
        fout << "\n";
    }
    fout << "}\n";

    std::cout << "[JsonIO] JSON保存完成: " << path << std::endl;
    return true;
}

std::map<std::string, double> LoadJson(const std::string& path) {
    std::map<std::string, double> result;
    std::ifstream fin(path);
    if (!fin) {
        std::cout << "[JsonIO] 无法打开文件: " << path << std::endl;
        return result;
    }

    // 简化解析（占位实现）
    std::string line;
    while (std::getline(fin, line)) {
        if (line.find(':') != std::string::npos) {
            // 简单解析
        }
    }

    std::cout << "[JsonIO] JSON加载完成: " << path << std::endl;
    return result;
}
