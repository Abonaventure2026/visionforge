#pragma once
#include "core/ImageBase.h"
#include "core/StructParam.h"
#include <vector>

class PhaseShiftSolver {
public:
    static std::vector<Image> Gen4StepShift(int width, int height, float freq = 16.0f);
    static std::vector<float> SolveWrap(const std::vector<Image>& frames);
};
