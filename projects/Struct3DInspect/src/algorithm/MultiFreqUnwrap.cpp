#include "MultiFreqUnwrap.h"
#include <cmath>
#include <algorithm>

std::vector<float> MultiFreqUnwrap::Unwrap(const std::vector<std::vector<float>>& wrap_phases,
                                           const std::vector<float>& freqs,
                                           const std::vector<int>& gray_code) {
    int n = wrap_phases[0].size();
    std::vector<float> abs_phase(n, 0.0f);
    if (wrap_phases.size() < 3) return abs_phase;
    float f1 = freqs[0], f2 = freqs[1], f3 = freqs[2];
    float f12 = f1 - f2, f23 = f2 - f3, f123 = f12 - f23;
    double pi = 3.1415926535;
    for (int i = 0; i < n; ++i) {
        float phi1 = wrap_phases[0][i], phi2 = wrap_phases[1][i], phi3 = wrap_phases[2][i];
        float phi12 = phi1 - phi2;
        float phi23 = phi2 - phi3;
        float phi123 = phi12 - phi23;
        int code = gray_code[i];
        float base_phase = code * 2 * pi + phi1;
        abs_phase[i] = base_phase;
    }
    return abs_phase;
}
