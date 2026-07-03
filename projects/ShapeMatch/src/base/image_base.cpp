#include "image_base.h"
#include <algorithm>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../third_party/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../third_party/stb_image_resize2.h"

Image::Image() : width(0), height(0), channel(0), data(nullptr) {}
Image::Image(int w, int h, int c) {
    width = w; height = h; channel = c;
    data = new uint8_t[w * h * c]{};
}
Image::Image(const Image& other) {
    width = other.width;
    height = other.height;
    channel = other.channel;
    data = new uint8_t[width * height * channel];
    memcpy(data, other.data, width * height * channel);
}
Image& Image::operator=(const Image& other) {
    if (this == &other) return *this;
    Release();
    width = other.width;
    height = other.height;
    channel = other.channel;
    data = new uint8_t[width * height * channel];
    memcpy(data, other.data, width * height * channel);
    return *this;
}
Image::~Image() { Release(); }
void Image::Release() {
    if (data) {
        delete[] data;
        data = nullptr;
    }
    width = height = channel = 0;
}
Image Image::Clone() const {
    return Image(*this);
}
Image Image::ToGray() const {
    if (width == 0 || height == 0 || channel == 0) return Image(0,0,0);
    Image out(width, height, 1);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = i * width * channel + j * channel;
            uint8_t r = data[idx], g = data[idx + 1], b = data[idx + 2];
            out.data[i * width + j] = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }
    return out;
}
Image Image::Resize(int newW, int newH) const {
    if (width == 0 || height == 0 || newW <= 0 || newH <= 0) return Image(0,0,0);
    Image out(newW, newH, channel);
    stbir_pixel_layout layout;
    switch (channel) {
        case 1: layout = STBIR_1CHANNEL; break;
        case 2: layout = STBIR_2CHANNEL; break;
        case 3: layout = STBIR_RGB; break;
        case 4: layout = STBIR_RGBA; break;
        default: layout = STBIR_4CHANNEL; break;
    }
    stbir_resize(data, width, height, width * channel,
                 out.data, newW, newH, newW * channel,
                 layout, STBIR_TYPE_UINT8,
                 STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM);
    return out;
}
Image Image::Rotate(double angleDeg) const {
    if (width == 0 || height == 0) return Image(0,0,0);
    float angleRad = static_cast<float>(angleDeg * PI / 180.0);
    float cosA = std::cos(angleRad), sinA = std::sin(angleRad);
    Image out(width, height, channel);
    int cx = width / 2, cy = height / 2;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sx = static_cast<int>((x - cx) * cosA - (y - cy) * sinA + cx);
            int sy = static_cast<int>((x - cx) * sinA + (y - cy) * cosA + cy);
            if (sx < 0 || sx >= width || sy < 0 || sy >= height) continue;
            int srcIdx = (sy * width + sx) * channel;
            int dstIdx = (y * width + x) * channel;
            for (int c = 0; c < channel; c++)
                out.data[dstIdx + c] = data[srcIdx + c];
        }
    }
    return out;
}
// 裁剪函数
Image Image::Crop(int x, int y, int w, int h) const {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (w <= 0 || h <= 0 || x + w > width || y + h > height) return Image(0,0,0);
    Image out(w, h, channel);
    for (int i = 0; i < h; i++) {
        memcpy(out.data + i * w * channel,
               data + (y + i) * width * channel + x * channel,
               w * channel);
    }
    return out;
}
static const float gauss5[5][5] = {{2,4,5,4,2},{4,9,12,9,4},{5,12,15,12,5},{4,9,12,9,4},{2,4,5,4,2}};
static const float gaussSum = 159.f;
void GaussianBlur5x5(const Image& src, Image& dst) {
    if (src.width == 0 || src.height == 0) return;
    int w = src.width, h = src.height;
    Image tmp(w, h, 1);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum = 0;
            for (int k = -2; k <= 2; k++)
                sum += gauss5[2][k+2] * src.data[std::clamp(x + k, 0, w - 1)];
            tmp.data[y * w + x] = static_cast<uint8_t>(sum / gaussSum);
        }
    }
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum = 0;
            for (int k = -2; k <= 2; k++)
                sum += gauss5[k+2][2] * tmp.data[std::clamp(y + k, 0, h - 1) * w + x];
            dst.data[y * w + x] = static_cast<uint8_t>(sum / gaussSum);
        }
    }
}
// 5x5 Sobel 算子
static const float sobel5x5_x[5][5] = {
    {-1, -2, 0, 2, 1},
    {-2, -4, 0, 4, 2},
    {-3, -6, 0, 6, 3},
    {-2, -4, 0, 4, 2},
    {-1, -2, 0, 2, 1}
};
static const float sobel5x5_y[5][5] = {
    {-1, -2, -3, -2, -1},
    {-2, -4, -6, -4, -2},
    {0, 0, 0, 0, 0},
    {2, 4, 6, 4, 2},
    {1, 2, 3, 2, 1}
};
void Sobel5x5(const Image& gray, Image& dx, Image& dy) {
    int w = gray.width, h = gray.height;
    if (w == 0 || h == 0) { dx = Image(0,0,0); dy = Image(0,0,0); return; }
    dx = Image(w, h, 1);
    dy = Image(w, h, 1);
    for (int y = 2; y < h - 2; y++) {
        for (int x = 2; x < w - 2; x++) {
            float gx = 0, gy = 0;
            for (int ky = -2; ky <= 2; ky++) {
                for (int kx = -2; kx <= 2; kx++) {
                    uint8_t val = gray.data[(y+ky)*w + (x+kx)];
                    gx += sobel5x5_x[ky+2][kx+2] * val;
                    gy += sobel5x5_y[ky+2][kx+2] * val;
                }
            }
            dx.data[y*w + x] = std::clamp(static_cast<int>(gx), 0, 255);
            dy.data[y*w + x] = std::clamp(static_cast<int>(gy), 0, 255);
        }
    }
}
void GradientInfo(const Image& dx, const Image& dy, std::vector<float>& magMap, std::vector<int>& dirMap) {
    int w = dx.width, h = dx.height;
    magMap.resize(w * h);
    dirMap.resize(w * h);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            float gx = dx.data[i * w + j];
            float gy = dy.data[i * w + j];
            float mag = std::sqrt(gx * gx + gy * gy);
            magMap[i * w + j] = mag;
            double ang = FastArctan(gy, gx);
            int dir = 0;
            if ((ang > 0 && ang < 22.5) || (ang > 157.5 && ang < 202.5) || ang > 337.5) dir = 0;
            else if ((ang > 22.5 && ang < 67.5) || (ang > 202.5 && ang < 247.5)) dir = 45;
            else if ((ang > 67.5 && ang < 112.5) || (ang > 247.5 && ang < 292.5)) dir = 90;
            else dir = 135;
            dirMap[i * w + j] = dir;
        }
    }
}
void NonMaxSuppression(int w, int h, const std::vector<float>& mag, const std::vector<int>& dirMap, std::vector<float>& nmsOut) {
    nmsOut.assign(w * h, 0.f);
    for (int i = 1; i < h - 1; i++) {
        for (int j = 1; j < w - 1; j++) {
            int idx = i * w + j;
            float cur = mag[idx];
            float p1 = 0, p2 = 0;
            int dir = dirMap[idx];
            switch(dir) {
                case 0: p1 = mag[i*w + j-1]; p2 = mag[i*w + j+1]; break;
                case 45:p1 = mag[(i-1)*w + j+1]; p2 = mag[(i+1)*w + j-1]; break;
                case 90:p1 = mag[(i-1)*w + j]; p2 = mag[(i+1)*w + j]; break;
                case 135:p1 = mag[(i-1)*w + j-1]; p2 = mag[(i+1)*w + j+1]; break;
            }
            if (cur > p1 && cur > p2) nmsOut[idx] = cur;
        }
    }
}
void CannyEdge(int w, int h, std::vector<float>& nms, float highTh, float lowTh, std::vector<MyPoint2i>& edgePts) {
    edgePts.clear();
    for (int i = 1; i < h - 1; i++) {
        for (int j = 1; j < w - 1; j++) {
            float val = nms[i * w + j];
            if (val < lowTh) continue;
            bool strong = val >= highTh;
            bool connect = false;
            if (!strong) {
                for (int di = -1; di <= 1 && !connect; di++)
                    for (int dj = -1; dj <= 1 && !connect; dj++)
                        if (nms[(i+di)*w + (j+dj)] >= highTh) connect = true;
            }
            if (strong || connect) edgePts.emplace_back(j, i);
        }
    }
}
std::vector<Image> BuildPyramid(const Image& gray, int level) {
    std::vector<Image> pyr;
    if (gray.width == 0 || gray.height == 0) return pyr;
    Image cur = Image(gray);
    pyr.push_back(cur);
    for (int l = 1; l <= level; l++) {
        int nw = cur.width / 2;
        int nh = cur.height / 2;
        if (nw < 1 || nh < 1) break;
        Image next = cur.Resize(nw, nh);
        pyr.push_back(next);
        cur = next;
    }
    return pyr;
}
void DrawPoints(Image& img, const std::vector<MyPoint2i>& pts, int offsetX, int offsetY) {
    if (img.width == 0 || img.height == 0) return;
    for (auto& p : pts) {
        int x = p.x + offsetX;
        int y = p.y + offsetY;
        if (x >= 0 && x < img.width && y >= 0 && y < img.height) {
            int idx = (y * img.width + x) * 3;
            img.data[idx] = 0;
            img.data[idx+1] = 0;
            img.data[idx+2] = 255;
        }
    }
}
Image CreateGearMask(const Image& gray) {
    if (gray.width == 0 || gray.height == 0) return Image(0,0,0);
    Image mask(gray.width, gray.height, 1);
    for (int i = 0; i < mask.height; i++)
        for (int j = 0; j < mask.width; j++)
            mask.data[i*mask.width+j] = 255;
    return mask;
}
void MaskFilterGradient(Image& dx, Image& dy, const Image& mask) { /* 空实现 */ }
MyPoint2i SubpixelRefine(const Image& srcGray, const Image& tempGray, const MyPoint2i& pos, double angle, double scale) {
    return pos;
}
