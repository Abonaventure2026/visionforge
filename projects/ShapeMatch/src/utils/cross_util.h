#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <random>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#include <sys/stat.h>
#define PATH_SEP '/'
#endif

void PrintLog(const std::string& msg);
bool FileExists(const std::string& filePath);

template<typename T>
void Free2DArray(T**& arr, int rows) {
    if (!arr) return;
    for(int i=0;i<rows;i++){ if(arr[i]) delete[] arr[i]; arr[i]=nullptr; }
    delete[] arr; arr=nullptr;
}
template<typename T>
T** Alloc2DArray(int rows, int cols) {
    T** arr = new T*[rows];
    for(int i=0;i<rows;i++) arr[i] = new T[cols]{};
    return arr;
}

constexpr double PI = 3.14159265358979323846;
inline double FastArctan(float y, float x) {
    return std::atan2(static_cast<double>(y), static_cast<double>(x)) * 180.0 / PI;
}
