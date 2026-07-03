#include "Triangulate.h"
#include <cmath>

void Triangulate::CalcDepthAndCloud(const std::vector<float>& abs_phase,
                                    const CalibParam& calib,
                                    int width, int height,
                                    std::vector<float>& depth_map,
                                    std::vector<Point3D>& cloud) {
    depth_map.assign(width * height, 0.0f);
    cloud.clear();
    double pi = 3.1415926535;
    double period_px = 1280.0 / calib.freqs[0];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            float phase = abs_phase[idx];
            float proj_x = phase / (2 * pi) * period_px;
            float disp = proj_x - x;
            if (fabs(disp) < 0.001) continue;
            double depth = calib.baseline_mm * calib.cam_focal_px / disp;
            depth_map[idx] = static_cast<float>(depth);
            if (depth > 10 && depth < 2000) {
                float X = x * calib.pixel_size_mm * depth / calib.cam_focal_px;
                float Y = y * calib.pixel_size_mm * depth / calib.cam_focal_px;
                cloud.emplace_back(X, Y, static_cast<float>(depth));
            }
        }
    }
}
