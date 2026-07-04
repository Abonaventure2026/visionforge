#pragma once
#include "../core/HalconTool.h"
#include "../core/CommonStruct.h"
#include <string>
#include <vector>

class COcrBottleAlgo
{
public:
#ifdef USE_HALCON
    static int RunBottleOCR(const std::string& imgPath,
                            const std::string& fontName,
                            SOCRResult& outResult);
    static int BatchOCR(const std::vector<std::string>& imgPaths,
                        const std::string& fontName,
                        std::vector<SOCRResult>& batchRes);
#else
    // 打桩模式：返回模拟结果
    static int RunBottleOCR(const std::string& imgPath,
                            const std::string& fontName,
                            SOCRResult& outResult) {
        (void)imgPath; (void)fontName;
        outResult = CMockDataGenerator::GenerateMockOCRResult(8);
        return 0;
    }
    static int BatchOCR(const std::vector<std::string>& imgPaths,
                        const std::string& fontName,
                        std::vector<SOCRResult>& batchRes) {
        (void)imgPaths; (void)fontName;
        batchRes.clear();
        for (size_t i = 0; i < imgPaths.size(); i++) {
            batchRes.push_back(CMockDataGenerator::GenerateMockOCRResult(8));
        }
        return (int)batchRes.size();
    }
#endif
};
