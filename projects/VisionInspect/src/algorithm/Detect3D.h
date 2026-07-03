#pragma once
#include "../core/ImageBase.h"
#include "../core/Matrix.h"
#include <vector>

struct Detect3DResult {
    std::vector<float> depth_map;
    std::vector<float> point_cloud;
    bool valid = false;
};

class Detect3D {
public:
    static Detect3DResult Detect(const std::vector<Image>& images);
};
