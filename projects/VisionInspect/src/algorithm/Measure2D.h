#pragma once
#include "../core/ImageBase.h"
#include <vector>

struct MeasureResult {
    double width = 0;
    double height = 0;
    double area = 0;
    double perimeter = 0;
    bool valid = false;
};

class Measure2D {
public:
    static MeasureResult Measure(const Image& binary);
};
