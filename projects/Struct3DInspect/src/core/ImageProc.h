#pragma once
#include "ImageBase.h"
#include "StructParam.h"
#include <vector>

class ImagePreproc {
public:
    static Image GrayBlur(const Image& src, int ksize=3);
    static Image OtsuBin(const Image& gray);
    static Image IlluminationCorrect(const Image& gray);
    static Image CropROI(const Image& src, int x, int y, int w, int h);
};
