#include "CSVWriter.h"
#include <fstream>
#include <iostream>

bool ExportCSV(const std::string& path, const std::vector<std::string>& headers,
               const std::vector<std::vector<double>>& data) {
    std::ofstream fout(path);
    if (!fout) {
        std::cout << "[CSVWriter] 无法打开文件: " << path << std::endl;
        return false;
    }

    // 写入表头
    for (size_t i = 0; i < headers.size(); ++i) {
        fout << headers[i];
        if (i < headers.size() - 1) fout << ",";
    }
    fout << "\n";

    // 写入数据
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            fout << row[i];
            if (i < row.size() - 1) fout << ",";
        }
        fout << "\n";
    }

    std::cout << "[CSVWriter] CSV导出完成: " << path << std::endl;
    return true;
}
