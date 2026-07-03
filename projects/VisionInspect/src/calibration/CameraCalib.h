#pragma once
#include "../core/ImageBase.h"
#include "../core/Matrix.h"
#include <vector>
#include <string>

struct CalibParam {
    Matrix<double> K;
    Matrix<double> D;
    Matrix<double> R;
    Matrix<double> T;
    double reproj_error;
    double duration_ms;
    bool valid;
    int width, height;
    std::string target_type;
    double square_size_mm;
};

struct TargetParam {
    std::string target_type;
    int inner_corners_cols;
    int inner_corners_rows;
    double square_size_mm;
};

class CameraCalib {
public:
    bool Init(const TargetParam& target, int img_width, int img_height);
    CalibParam Calibrate(const std::vector<Image>& images);
    bool SaveParam(const CalibParam& param, const std::string& path);
    CalibParam LoadParam(const std::string& path);

private:
    TargetParam target_param;
    int img_width, img_height;
    std::vector<std::vector<double>> ExtractCorners(const Image& gray);
};
