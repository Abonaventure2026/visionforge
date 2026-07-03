#pragma once
#include "../core/ImageBase.h"
#include "../core/Matrix.h"
#include "../calibration/CameraCalib.h"
#include <vector>
#include <string>

struct DefectInfo {
    int pixel_x, pixel_y;
    int pixel_width, pixel_height;
    double world_x, world_y;
    double world_width, world_height;
    std::string type;
    bool valid;
};

struct DefectResult {
    std::vector<DefectInfo> defects;
    double detect_duration_ms;
    int frame_id;
    bool valid;
};

struct DetectParam {
    double min_defect_area = 10.0;
    double min_defect_size = 5.0;
    double max_defect_size = 100.0;
    double max_spot_size = 20.0;
};

class DefectDetect2D {
public:
    bool Init(const CalibParam& calib, const DetectParam& param);
    DefectResult Detect(const Image& img);

private:
    CalibParam calib_param;
    DetectParam detect_param;
    int frame_count = 0;
    std::vector<std::vector<int>> ExtractContours(const Image& binary);
    std::vector<DefectInfo> IdentifyDefects(const std::vector<std::vector<int>>& contours);
};
