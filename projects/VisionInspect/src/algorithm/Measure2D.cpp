#include "Measure2D.h"
#include "../core/Timer.h"
#include <iostream>

MeasureResult Measure2D::Measure(const Image& binary) {
    MeasureResult result;
    result.valid = false;

    Timer timer;
    timer.Start();

    if (!binary.IsValid() || binary.channels != 1) {
        std::cout << "[Measure2D] 无效的二值图像" << std::endl;
        return result;
    }

    // 占位实现
    result.width = binary.width;
    result.height = binary.height;
    result.area = binary.width * binary.height;
    result.perimeter = 2 * (binary.width + binary.height);
    result.valid = true;

    std::cout << "[Measure2D] 测量完成，耗时: " << timer.ElapsedMs() << " ms" << std::endl;
    return result;
}
