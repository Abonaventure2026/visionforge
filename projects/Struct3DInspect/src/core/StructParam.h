#pragma once
#include <vector>
#include <cmath>

struct CalibParam {
    double baseline_mm = 120.0;
    double cam_focal_px = 1850.0;
    double proj_focal_px = 1720.0;
    double pixel_size_mm = 0.0045;
    int gray_bits = 8;
    int shift_steps = 4;
    std::vector<float> freqs = {16.0f, 24.0f, 32.0f};
};

struct Point3D {
    float x, y, z;
    Point3D(float x_=0, float y_=0, float z_=0) : x(x_), y(y_), z(z_) {}
};

struct InspectResult {
    std::vector<float> depth_map;
    std::vector<Point3D> point_cloud;
    double width_mm = 0, height_mm = 0, thickness_mm = 0;
    bool valid = false;
};
