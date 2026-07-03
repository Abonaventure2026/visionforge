#pragma once
#include "../core/ImageBase.h"
#include <vector>

struct FeaturePoint {
    int x, y;
    float response;
};

struct FeatureResult {
    std::vector<FeaturePoint> points;
    bool valid = false;
};

class FeatureDetect {
public:
    static FeatureResult Detect(const Image& gray);
};
