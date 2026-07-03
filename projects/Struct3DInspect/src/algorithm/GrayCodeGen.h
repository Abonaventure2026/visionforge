#pragma once
#include "core/ImageBase.h"
#include "core/StructParam.h"
#include <vector>

class GrayCodeGen {
public:
    static Image GenSingleGray(int bit, int width, int height, bool inv = false);
    static std::vector<Image> GenFullSeq(const CalibParam& param);
    static std::vector<int> Decode(const std::vector<Image>& frames, int bits);
};
