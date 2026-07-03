#pragma once
#include "StructParam.h"
#include <vector>

class Triangulate {
public:
    static void CalcDepthAndCloud(const std::vector<float>& abs_phase,
                                  const CalibParam& calib,
                                  int width, int height,
                                  std::vector<float>& depth_map,
                                  std::vector<Point3D>& cloud);
};
