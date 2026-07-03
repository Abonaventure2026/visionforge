#pragma once
#include <string>
#include <vector>

struct Point3D {
    float x, y, z;
};

bool SavePLY(const std::string& path, const std::vector<Point3D>& points);
