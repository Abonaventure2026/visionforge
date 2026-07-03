#pragma once
#include "../core/ImageBase.h"
#include "../core/Matrix.h"
#include <vector>
#include <string>

struct StereoCalibParam {
    int width = 0;
    int height = 0;
    double square_size_mm = 10.0;
    bool valid = false;
};

struct StereoCalibResult {
    Matrix<double> R;          // 旋转矩阵
    Matrix<double> T;          // 平移向量
    Matrix<double> E;          // 本质矩阵
    Matrix<double> F;          // 基础矩阵
    double reproj_error = 0;
    bool valid = false;
};

class StereoCalib {
public:
    bool Init(const StereoCalibParam& param);
    StereoCalibResult Calibrate(const std::vector<Image>& left_images,
                                const std::vector<Image>& right_images);
};
