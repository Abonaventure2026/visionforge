#include "ImageBase.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../third_party/stb_image.h"
#include "../../third_party/stb_image_write.h"

Image Image::ToGray() const {
    if (!IsValid() || channels != 3) return Image();
    Image out(width, height, 1);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int idx = (i * width + j) * 3;
            uint8_t r = data[idx], g = data[idx+1], b = data[idx+2];
            out.data[i * width + j] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
        }
    }
    return out;
}

Image Image::Crop(int x, int y, int w, int h) const {
    if (!IsValid() || x < 0 || y < 0 || x + w > width || y + h > height) return Image();
    Image out(w, h, channels);
    for (int i = 0; i < h; ++i) {
        memcpy(out.data + i * w * channels,
               data + (y + i) * width * channels + x * channels,
               w * channels);
    }
    return out;
}

Image Image::Resize(int newW, int newH) const {
    if (!IsValid() || newW <= 0 || newH <= 0) return Image();
    Image out(newW, newH, channels);
    for (int y = 0; y < newH; ++y) {
        for (int x = 0; x < newW; ++x) {
            int srcX = x * width / newW;
            int srcY = y * height / newH;
            int srcIdx = (srcY * width + srcX) * channels;
            int dstIdx = (y * newW + x) * channels;
            for (int c = 0; c < channels; ++c) {
                out.data[dstIdx + c] = data[srcIdx + c];
            }
        }
    }
    return out;
}

void Image::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (!data || x < 0 || x >= width || y < 0 || y >= height || channels < 3) return;
    int idx = (y * width + x) * channels;
    data[idx] = r; data[idx+1] = g; data[idx+2] = b;
}

bool Image::LoadFromFile(const std::string& path) {
    int w, h, c;
    uint8_t* img_data = stbi_load(path.c_str(), &w, &h, &c, 3);
    if (!img_data) return false;
    Release();
    width = w; height = h; channels = 3;
    data = new uint8_t[width * height * channels];
    memcpy(data, img_data, width * height * channels);
    stbi_image_free(img_data);
    return true;
}

bool Image::SaveToFile(const std::string& path) const {
    if (!IsValid()) return false;
    int ret = stbi_write_png(path.c_str(), width, height, channels, data, width * channels);
    return ret != 0;
}

void GaussianBlur3x3(const Image& src, Image& dst) {
    if (!src.IsValid() || src.channels != 1) return;
    dst = Image(src.width, src.height, 1);
    float kernel[3][3] = {{1,2,1},{2,4,2},{1,2,1}};
    float sum = 16.0f;
    for (int y = 1; y < src.height - 1; ++y) {
        for (int x = 1; x < src.width - 1; ++x) {
            float val = 0;
            for (int ky = -1; ky <= 1; ++ky)
                for (int kx = -1; kx <= 1; ++kx)
                    val += kernel[ky+1][kx+1] * src.data[(y+ky)*src.width + (x+kx)];
            dst.data[y*dst.width + x] = static_cast<uint8_t>(val / sum);
        }
    }
}

void Sobel3x3(const Image& gray, Image& dx, Image& dy) {
    if (!gray.IsValid() || gray.channels != 1) return;
    dx = Image(gray.width, gray.height, 1);
    dy = Image(gray.width, gray.height, 1);
    float sobelX[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    float sobelY[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    for (int y = 1; y < gray.height - 1; ++y) {
        for (int x = 1; x < gray.width - 1; ++x) {
            float gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky)
                for (int kx = -1; kx <= 1; ++kx) {
                    uint8_t val = gray.data[(y+ky)*gray.width + (x+kx)];
                    gx += sobelX[ky+1][kx+1] * val;
                    gy += sobelY[ky+1][kx+1] * val;
                }
            dx.data[y*gray.width + x] = std::clamp(static_cast<int>(std::abs(gx)), 0, 255);
            dy.data[y*gray.width + x] = std::clamp(static_cast<int>(std::abs(gy)), 0, 255);
        }
    }
}

void ComputeGradient(const Image& dx, const Image& dy,
                     std::vector<float>& mag, std::vector<float>& angle) {
    int total = dx.width * dx.height;
    mag.resize(total);
    angle.resize(total);
    for (int i = 0; i < total; ++i) {
        float gx = dx.data[i], gy = dy.data[i];
        mag[i] = std::sqrt(gx*gx + gy*gy);
        angle[i] = std::atan2(gy, gx) * 180.0f / 3.1415926535f;
    }
}

void OtsuThreshold(const Image& gray, Image& binary) {
    if (!gray.IsValid() || gray.channels != 1) return;
    binary = Image(gray.width, gray.height, 1);
    int hist[256] = {0};
    int total = gray.width * gray.height;
    for (int i = 0; i < total; ++i) hist[gray.data[i]]++;
    double sumAll = 0;
    for (int i = 0; i < 256; ++i) sumAll += (double)i * hist[i];
    double w0 = 0, sum0 = 0, maxVar = 0;
    int bestT = 128;
    for (int t = 0; t < 256; ++t) {
        w0 += hist[t];
        if (w0 < 1e-3) continue;
        double w1 = total - w0;
        if (w1 < 1e-3) break;
        sum0 += (double)t * hist[t];
        double u0 = sum0 / w0;
        double u1 = (sumAll - sum0) / w1;
        double var = w0 * w1 * (u0 - u1) * (u0 - u1);
        if (var > maxVar) { maxVar = var; bestT = t; }
    }
    for (int i = 0; i < total; ++i) {
        binary.data[i] = gray.data[i] > bestT ? 255 : 0;
    }
}

void MorphClose(const Image& src, Image& dst, int kernel_size) {
    if (!src.IsValid() || src.channels != 1) return;
    dst = Image(src.width, src.height, 1);
    memcpy(dst.data, src.data, src.width * src.height);
}
