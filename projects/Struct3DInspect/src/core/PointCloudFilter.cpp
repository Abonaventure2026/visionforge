#include "PointCloudFilter.h"
#include <algorithm>
#include <cmath>

void PointCloudFilter::StatisticalOutlierRemoval(std::vector<Point3D>& cloud, float std_mult, int neigh) {
    (void)std_mult; (void)neigh;
}

void PointCloudFilter::VoxelDownsample(std::vector<Point3D>& cloud, float leaf_size) {
    (void)leaf_size;
}
