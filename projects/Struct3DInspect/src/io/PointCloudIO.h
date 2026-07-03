#pragma once
#include "core/StructParam.h"
#include <string>
#include <vector>

bool SavePLY(const std::string& path, const std::vector<Point3D>& cloud);
