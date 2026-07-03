#include "DefectDetect2D.h"
#include "../core/Timer.h"
#include <iostream>

bool DefectDetect2D::Init(const CalibParam& calib, const DetectParam& param) {
    calib_param = calib;
    detect_param = param;
    return true;
}

DefectResult DefectDetect2D::Detect(const Image& img) {
    Timer timer;
    timer.Start();
    DefectResult result;
    result.valid = false;

    if (!img.IsValid()) return result;

    // 模拟缺陷检测：生成模拟缺陷数据用于演示
    std::vector<DefectInfo> simulated_defects;

    // 缺陷1：划痕
    DefectInfo d1;
    d1.pixel_x = 120;
    d1.pixel_y = 45;
    d1.pixel_width = 60;
    d1.pixel_height = 3;
    d1.type = "scratch";
    d1.valid = true;
    simulated_defects.push_back(d1);

    // 缺陷2：污点
    DefectInfo d2;
    d2.pixel_x = 200;
    d2.pixel_y = 150;
    d2.pixel_width = 15;
    d2.pixel_height = 15;
    d2.type = "spot";
    d2.valid = true;
    simulated_defects.push_back(d2);

    // 缺陷3：缺料
    DefectInfo d3;
    d3.pixel_x = 300;
    d3.pixel_y = 170;
    d3.pixel_width = 70;
    d3.pixel_height = 70;
    d3.type = "missing";
    d3.valid = true;
    simulated_defects.push_back(d3);

    // 缺陷4：划痕
    DefectInfo d4;
    d4.pixel_x = 450;
    d4.pixel_y = 300;
    d4.pixel_width = 40;
    d4.pixel_height = 2;
    d4.type = "scratch";
    d4.valid = true;
    simulated_defects.push_back(d4);

    result.defects = simulated_defects;
    result.valid = !result.defects.empty();
    result.frame_id = ++frame_count;
    result.detect_duration_ms = timer.ElapsedMs();

    std::cout << "[DEBUG] 检测完成，发现 " << result.defects.size() << " 个缺陷" << std::endl;
    Timer::Log("2D缺陷检测", result.detect_duration_ms);
    return result;
}

std::vector<DefectInfo> DefectDetect2D::IdentifyDefects(const std::vector<std::vector<int>>& contours) {
    std::vector<DefectInfo> defects;
    return defects;
}
