#include "PhaseShiftSolver.h"
#include <cmath>

std::vector<Image> PhaseShiftSolver::Gen4StepShift(int width, int height, float freq) {
    std::vector<Image> seq(4);
    double pi = 3.1415926535;
    for (int s = 0; s < 4; ++s) {
        Image img(width, height, 1);
        double phi = s * pi / 2.0;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double val = cos(2 * pi * freq * x / width + phi);
                img.data[y * width + x] = static_cast<uint8_t>(127.5 * val + 127.5);
            }
        }
        seq[s] = img;
    }
    return seq;
}

std::vector<float> PhaseShiftSolver::SolveWrap(const std::vector<Image>& frames) {
    if (frames.size() < 4) return {};
    int w = frames[0].width, h = frames[0].height;
    std::vector<float> phase(w * h);
    for (int i = 0; i < w*h; ++i) {
        float I0 = frames[0].data[i], I1 = frames[1].data[i];
        float I2 = frames[2].data[i], I3 = frames[3].data[i];
        phase[i] = atan2(I3 - I1, I0 - I2);
    }
    return phase;
}
