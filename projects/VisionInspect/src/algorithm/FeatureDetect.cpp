#include "FeatureDetect.h"
#include "../core/Timer.h"
#include <iostream>

FeatureResult FeatureDetect::Detect(const Image& gray) {
    FeatureResult result;
    result.valid = false;

    Timer timer;
    timer.Start();

    if (!gray.IsValid() || gray.channels != 1) {
        std::cout << "[FeatureDetect] 无效的灰度图像" << std::endl;
        return result;
    }

    // 占位实现
    result.points.push_back({gray.width/2, gray.height/2, 1.0f});
    result.valid = true;

    std::cout << "[FeatureDetect] 特征检测完成，耗时: " << timer.ElapsedMs() << " ms" << std::endl;
    return result;
}
