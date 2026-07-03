#include "Detect3D.h"
#include "../core/Timer.h"
#include <iostream>

Detect3DResult Detect3D::Detect(const std::vector<Image>& images) {
    Detect3DResult result;
    result.valid = false;

    Timer timer;
    timer.Start();

    if (images.empty()) {
        std::cout << "[Detect3D] 图像序列为空" << std::endl;
        return result;
    }

    // 占位实现
    int total = images[0].width * images[0].height;
    result.depth_map.resize(total, 0.0f);
    result.point_cloud.resize(total * 3, 0.0f);
    result.valid = true;

    std::cout << "[Detect3D] 3D检测完成，耗时: " << timer.ElapsedMs() << " ms" << std::endl;
    return result;
}
