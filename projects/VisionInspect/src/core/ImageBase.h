#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <fstream>

struct Image {
    int width, height, channels;
    uint8_t* data;

    Image() : width(0), height(0), channels(0), data(nullptr) {}
    Image(int w, int h, int c) : width(w), height(h), channels(c) {
        data = new uint8_t[w * h * c]();
    }
    Image(const Image& other) {
        width = other.width; height = other.height; channels = other.channels;
        data = new uint8_t[width * height * channels];
        memcpy(data, other.data, width * height * channels);
    }
    Image& operator=(const Image& other) {
        if (this != &other) {
            Release();
            width = other.width; height = other.height; channels = other.channels;
            data = new uint8_t[width * height * channels];
            memcpy(data, other.data, width * height * channels);
        }
        return *this;
    }
    ~Image() { Release(); }

    void Release() {
        if (data) { delete[] data; data = nullptr; }
        width = height = channels = 0;
    }
    bool IsValid() const { return data != nullptr && width > 0 && height > 0 && channels > 0; }
    Image Clone() const { return Image(*this); }

    Image ToGray() const;
    Image Crop(int x, int y, int w, int h) const;
    Image Resize(int newW, int newH) const;
    void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

    bool LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path) const;
};

void GaussianBlur3x3(const Image& src, Image& dst);
void Sobel3x3(const Image& gray, Image& dx, Image& dy);
void ComputeGradient(const Image& dx, const Image& dy,
                     std::vector<float>& mag, std::vector<float>& angle);
void OtsuThreshold(const Image& gray, Image& binary);
void MorphClose(const Image& src, Image& dst, int kernel_size);
