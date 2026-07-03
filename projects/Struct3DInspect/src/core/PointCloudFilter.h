#pragma once
#include "StructParam.h"
#include <vector>

class PointCloudFilter {
public:
    static void StatisticalOutlierRemoval(std::vector<Point3D>& cloud, float std_mult=1.0f, int neigh=20);
    static void VoxelDownsample(std::vector<Point3D>& cloud, float leaf_size=0.5f);
};
