#pragma once
#include <string>
#include <vector>
#include <map>

bool SaveJson(const std::string& path, const std::map<std::string, double>& data);
std::map<std::string, double> LoadJson(const std::string& path);
