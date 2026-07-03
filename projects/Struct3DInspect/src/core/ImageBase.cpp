#include "ImageBase.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../third_party/stb_image.h"
#include "../../third_party/stb_image_write.h"
#include "../../third_party/stb_image_resize2.h"

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
    stbir_pixel_layout layout = (channels == 1) ? STBIR_1CHANNEL : STBIR_RGB;
    stbir_resize(data, width, height, width * channels,
                 out.data, newW, newH, newW * channels,
                 layout, STBIR_TYPE_UINT8,
                 STBIR_EDGE_CLAMP, STBIR_FILTER_CATMULLROM);
    return out;
}

void GaussianBlur3x3(const Image& src, Image& dst) {
    if (!src.IsValid()) return;
    dst = Image(src.width, src.height, src.channels);
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

void ComputeGradient(const Image& dx, const Image& dy, std::vector<float>& mag, std::vector<float>& angle) {
    int total = dx.width * dx.height;
    mag.resize(total);
    angle.resize(total);
    for (int i = 0; i < total; ++i) {
        float gx = dx.data[i], gy = dy.data[i];
        mag[i] = std::sqrt(gx*gx + gy*gy);
        angle[i] = std::atan2(gy, gx) * 180.0f / 3.1415926535f;
    }
}
Image CreateGearMask(const Image& gray) { return Image(gray.width, gray.height, 1); }
