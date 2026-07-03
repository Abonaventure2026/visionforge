#pragma once
#include "../core/Matrix.h"
#include <vector>

struct HandEyeCalibParam {
    bool valid = false;
};

struct HandEyeCalibResult {
    Matrix<double> R;          // 旋转矩阵
    Matrix<double> T;          // 平移向量
    double reproj_error = 0;
    bool valid = false;
};

class HandEyeCalib {
public:
    bool Init(const HandEyeCalibParam& param);
    HandEyeCalibResult Calibrate(const std::vector<Matrix<double>>& robot_poses,
                                  const std::vector<Matrix<double>>& cam_poses);
};
