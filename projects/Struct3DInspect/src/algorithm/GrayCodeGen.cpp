#include "GrayCodeGen.h"

Image GrayCodeGen::GenSingleGray(int bit, int width, int height, bool inv) {
    Image img(width, height, 1);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int code = (x >> bit) ^ (x >> (bit + 1));
            uint8_t val = code ? 255 : 0;
            if (inv) val = 255 - val;
            img.data[y * width + x] = val;
        }
    }
    return img;
}

std::vector<Image> GrayCodeGen::GenFullSeq(const CalibParam& param) {
    std::vector<Image> seq;
    int bits = param.gray_bits;
    for (int b = 0; b < bits; ++b) {
        seq.push_back(GenSingleGray(b, 1280, 1024, false));
        seq.push_back(GenSingleGray(b, 1280, 1024, true));
    }
    Image black(1280, 1024, 1);
    memset(black.data, 0, 1280*1024);
    Image white(1280, 1024, 1);
    memset(white.data, 255, 1280*1024);
    seq.push_back(black);
    seq.push_back(white);
    return seq;
}

std::vector<int> GrayCodeGen::Decode(const std::vector<Image>& frames, int bits) {
    if (frames.size() < 2*bits) return {};
    int w = frames[0].width, h = frames[0].height;
    std::vector<int> code_map(w * h, 0);
    for (int b = 0; b < bits; ++b) {
        const Image& pos = frames[2*b];
        const Image& neg = frames[2*b+1];
        for (int i = 0; i < w*h; ++i) {
            if (pos.data[i] > neg.data[i] + 30)
                code_map[i] |= (1 << b);
        }
    }
    return code_map;
}
