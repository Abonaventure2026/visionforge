#include "ImageProc.h"

Image ImagePreproc::GrayBlur(const Image& src, int ksize) {
    if (!src.IsValid() || src.channels != 3) return Image();
    Image gray = src.ToGray();
    if (ksize == 3) {
        Image blurred(gray.width, gray.height, 1);
        GaussianBlur3x3(gray, blurred);
        return blurred;
    }
    return gray;
}

Image ImagePreproc::OtsuBin(const Image& gray) {
    Image bin(gray.width, gray.height, 1);
    for (int i=0; i<gray.height*gray.width; ++i)
        bin.data[i] = gray.data[i] > 128 ? 255 : 0;
    return bin;
}

Image ImagePreproc::IlluminationCorrect(const Image& gray) {
    return gray.Clone();
}

Image ImagePreproc::CropROI(const Image& src, int x, int y, int w, int h) {
    return src.Crop(x, y, w, h);
}
