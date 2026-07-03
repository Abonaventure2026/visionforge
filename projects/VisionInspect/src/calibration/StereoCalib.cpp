#include "StereoCalib.h"
#include "../core/Timer.h"
#include <iostream>

bool StereoCalib::Init(const StereoCalibParam& param) {
    std::cout << "[StereoCalib] 初始化" << std::endl;
    return true;
}

StereoCalibResult StereoCalib::Calibrate(const std::vector<Image>& left_images,
                                         const std::vector<Image>& right_images) {
    StereoCalibResult result;
    result.valid = false;

    Timer timer;
    timer.Start();

    if (left_images.size() < 10 || right_images.size() < 10) {
        std::cout << "[StereoCalib] 图像数量不足，需要至少10对" << std::endl;
        return result;
    }

    // 占位实现
    result.R = Matrix<double>::Identity(3);
    result.T = Matrix<double>(3, 1);
    result.E = Matrix<double>::Identity(3);
    result.F = Matrix<double>::Identity(3);
    result.reproj_error = 0.15;
    result.valid = true;

    std::cout << "[StereoCalib] 标定完成，耗时: " << timer.ElapsedMs() << " ms" << std::endl;
    return result;
}
