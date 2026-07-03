#pragma once
#include "core/StructParam.h"
#include <vector>

class MultiFreqUnwrap {
public:
    static std::vector<float> Unwrap(const std::vector<std::vector<float>>& wrap_phases,
                                     const std::vector<float>& freqs,
                                     const std::vector<int>& gray_code);
};
