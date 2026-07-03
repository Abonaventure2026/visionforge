#pragma once
#include <string>
#include <vector>

bool ExportCSV(const std::string& path, const std::vector<std::string>& headers,
               const std::vector<std::vector<double>>& data);
