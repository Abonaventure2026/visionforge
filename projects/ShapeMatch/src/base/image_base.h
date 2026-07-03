#pragma once
#include "cross_util.h"
#include <vector>
struct MyPoint2i {
    int x, y;
    MyPoint2i(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
};
struct Image {
    int width, height, channel;
    uint8_t* data;
    Image();
    Image(int w, int h, int c);
    Image(const Image& other);
    Image& operator=(const Image& other);
    ~Image();
    Image Clone() const;
    void Release();
    Image ToGray() const;
    Image Resize(int newW, int newH) const;
    Image Rotate(double angleDeg) const;
    // 裁剪函数
    Image Crop(int x, int y, int w, int h) const;
};
void GaussianBlur5x5(const Image& src, Image& dst);
void Sobel5x5(const Image& gray, Image& dx, Image& dy);
void GradientInfo(const Image& dx, const Image& dy, std::vector<float>& magMap, std::vector<int>& dirMap);
void NonMaxSuppression(int w, int h, const std::vector<float>& mag, const std::vector<int>& dirMap, std::vector<float>& nmsOut);
void CannyEdge(int w, int h, std::vector<float>& nms, float highTh, float lowTh, std::vector<MyPoint2i>& edgePts);
std::vector<Image> BuildPyramid(const Image& gray, int level);
void DrawPoints(Image& img, const std::vector<MyPoint2i>& pts, int offsetX, int offsetY);
Image CreateGearMask(const Image& gray);
void MaskFilterGradient(Image& dx, Image& dy, const Image& mask);
MyPoint2i SubpixelRefine(const Image& srcGray, const Image& tempGray, const MyPoint2i& pos, double angle, double scale);
