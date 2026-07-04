#include "../core/LogService.h"
#include "../core/HalconTool.h"
#include "../algorithm/OcrModule/OcrBottleAlgo.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void PrintUsage() {
    cout << "HalconVision Console - 跨平台Halcon视觉控制台" << endl;
    cout << "用法:" << endl;
    cout << "  ./HalconConsole --ocr <image_path> [font_name]" << endl;
    cout << "  ./HalconConsole --batch <image_dir> [font_name]" << endl;
    cout << "  ./HalconConsole --mock              # 打桩模式测试" << endl;
    cout << "  ./HalconConsole --help" << endl;
    cout << endl;
    cout << "示例:" << endl;
    cout << "  ./HalconConsole --ocr bottle2.png Universal_0-9_NoRej" << endl;
    cout << "  ./HalconConsole --batch ./assets/ocr_sample/" << endl;
    cout << "  ./HalconConsole --mock               # 生成模拟数据测试" << endl;
}

int main(int argc, char* argv[]) {
    CLogService::GetInstance().Init("./logs/console.log", LOG_INFO);
    CLogService::GetInstance().Info("HalconVision Console 启动");

    if (argc < 2) {
        PrintUsage();
        return 0;
    }

    string cmd = argv[1];

    if (cmd == "--help" || cmd == "-h") {
        PrintUsage();
        return 0;
    }

#ifdef USE_HALCON
    // 真实Halcon模式
    try {
        if (cmd == "--ocr") {
            if (argc < 3) {
                cerr << "错误：请指定图像路径" << endl;
                return 1;
            }
            string imgPath = argv[2];
            string fontName = (argc >= 4) ? argv[3] : "Universal_0-9_NoRej";

            CLogService::GetInstance().Info("执行OCR识别: " + imgPath);

            SOCRResult result;
            if (COcrBottleAlgo::RunBottleOCR(imgPath, fontName, result) == 0) {
                PrintOCRResult(result);
            } else {
                cerr << "OCR识别失败" << endl;
                return 1;
            }
        }
        else if (cmd == "--batch") {
            if (argc < 3) {
                cerr << "错误：请指定图像目录" << endl;
                return 1;
            }
            string imgDir = argv[2];
            string fontName = (argc >= 4) ? argv[3] : "Universal_0-9_NoRej";

            CLogService::GetInstance().Info("批量OCR识别: " + imgDir);

            // TODO: 遍历目录实现批量处理
            vector<string> imgPaths;
            vector<SOCRResult> results;
            COcrBottleAlgo::BatchOCR(imgPaths, fontName, results);
            cout << "批量处理完成，成功: " << results.size() << " 张" << endl;
        }
        else if (cmd == "--mock") {
            cout << "=== 打桩模式测试 ===" << endl;
            SOCRResult mockResult = CMockDataGenerator::GenerateMockOCRResult(10);
            PrintOCRResult(mockResult);
        }
        else {
            cerr << "未知命令: " << cmd << endl;
            PrintUsage();
            return 1;
        }
    }
    catch (const exception& e) {
        CLogService::GetInstance().Error(string("异常: ") + e.what());
        cerr << "错误: " << e.what() << endl;
        return 1;
    }
#else
    // 打桩模式：所有命令返回模拟数据
    cout << "🔧 打桩测试模式 (USE_HALCON=OFF)" << endl;
    cout << "所有OCR和识别功能返回模拟数据" << endl;
    cout << endl;
    
    if (cmd == "--ocr" || cmd == "--batch") {
        SOCRResult result = CMockDataGenerator::GenerateMockOCRResult(8);
        PrintOCRResult(result);
        cout << endl;
        cout << "💡 提示: 使用 -DUSE_HALCON=ON 重新编译以启用真实Halcon功能" << endl;
    }
    else if (cmd == "--mock") {
        cout << "=== 打桩模式测试 ===" << endl;
        SOCRResult mockResult = CMockDataGenerator::GenerateMockOCRResult(10);
        PrintOCRResult(mockResult);
        
        cout << endl << "=== 缺陷检测模拟 ===" << endl;
        SDefectResult defectResult = CMockDataGenerator::GenerateMockDefectResult(5);
        cout << "缺陷数量: " << defectResult.defectNum << endl;
        
        cout << endl << "=== 模板匹配模拟 ===" << endl;
        SMatchResult matchResult = CMockDataGenerator::GenerateMockMatchResult(3);
        cout << "匹配数量: " << matchResult.matchNum << endl;
    }
    else {
        cerr << "未知命令: " << cmd << endl;
        PrintUsage();
        return 1;
    }
#endif

    CLogService::GetInstance().Info("程序正常退出");
    return 0;
}
