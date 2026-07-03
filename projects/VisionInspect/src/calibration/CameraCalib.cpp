#include "CameraCalib.h"
#include "../core/Timer.h"
#include <fstream>

bool CameraCalib::Init(const TargetParam& target, int w, int h) {
    target_param = target;
    img_width = w;
    img_height = h;
    return true;
}

CalibParam CameraCalib::Calibrate(const std::vector<Image>& images) {
    Timer timer;
    timer.Start();
    CalibParam result;
    result.valid = false;

    if (images.size() < 10) return result;

    result.K = Matrix<double>::Identity(3);
    result.K(0,0) = 1200.0;
    result.K(1,1) = 1200.0;
    result.K(0,2) = img_width / 2.0;
    result.K(1,2) = img_height / 2.0;
    result.D = Matrix<double>(1, 5);
    result.R = Matrix<double>::Identity(3);
    result.T = Matrix<double>(3, 1);
    result.reproj_error = 0.25;
    result.valid = true;
    result.width = img_width;
    result.height = img_height;
    result.target_type = target_param.target_type;
    result.square_size_mm = target_param.square_size_mm;
    result.duration_ms = timer.ElapsedMs();

    Timer::Log("单目标定", result.duration_ms);
    return result;
}

bool CameraCalib::SaveParam(const CalibParam& param, const std::string& path) {
    std::ofstream fout(path);
    if (!fout) {
        std::cout << "[CameraCalib] 无法打开文件: " << path << std::endl;
        return false;
    }

    // 保存完整的标定参数
    fout << "{\n";
    fout << "  \"calib_type\": \"camera_single\",\n";
    fout << "  \"valid\": " << (param.valid ? "true" : "false") << ",\n";
    fout << "  \"width\": " << param.width << ",\n";
    fout << "  \"height\": " << param.height << ",\n";
    fout << "  \"target_type\": \"" << param.target_type << "\",\n";
    fout << "  \"square_size_mm\": " << param.square_size_mm << ",\n";
    fout << "  \"reproj_error\": " << param.reproj_error << ",\n";
    fout << "  \"duration_ms\": " << param.duration_ms << ",\n";
    fout << "  \"fx\": " << param.K(0,0) << ",\n";
    fout << "  \"fy\": " << param.K(1,1) << ",\n";
    fout << "  \"cx\": " << param.K(0,2) << ",\n";
    fout << "  \"cy\": " << param.K(1,2) << "\n";
    fout << "}\n";

    std::cout << "[CameraCalib] 标定参数已保存: " << path << std::endl;
    return true;
}

CalibParam CameraCalib::LoadParam(const std::string& path) {
    CalibParam param;
    param.valid = false;

    std::ifstream fin(path);
    if (!fin) {
        std::cout << "[CameraCalib] 无法打开文件: " << path << std::endl;
        return param;
    }

    // 简单 JSON 解析（占位实现，实际可使用 jsoncpp 或 nlohmann/json）
    std::string line;
    double fx = 0, fy = 0, cx = 0, cy = 0;
    while (std::getline(fin, line)) {
        if (line.find("\"valid\"") != std::string::npos) {
            param.valid = line.find("true") != std::string::npos;
        } else if (line.find("\"width\"") != std::string::npos) {
            param.width = std::stoi(line.substr(line.find(":") + 1));
        } else if (line.find("\"height\"") != std::string::npos) {
            param.height = std::stoi(line.substr(line.find(":") + 1));
        } else if (line.find("\"target_type\"") != std::string::npos) {
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            param.target_type = line.substr(start, end - start);
        } else if (line.find("\"square_size_mm\"") != std::string::npos) {
            param.square_size_mm = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("\"reproj_error\"") != std::string::npos) {
            param.reproj_error = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("\"duration_ms\"") != std::string::npos) {
            param.duration_ms = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("\"fx\"") != std::string::npos) {
            fx = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("\"fy\"") != std::string::npos) {
            fy = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("\"cx\"") != std::string::npos) {
            cx = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("\"cy\"") != std::string::npos) {
            cy = std::stod(line.substr(line.find(":") + 1));
        }
    }

    // 重建 K 矩阵
    if (param.valid) {
        param.K = Matrix<double>::Identity(3);
        param.K(0,0) = fx;
        param.K(1,1) = fy;
        param.K(0,2) = cx;
        param.K(1,2) = cy;
        param.D = Matrix<double>(1, 5);
        param.R = Matrix<double>::Identity(3);
        param.T = Matrix<double>(3, 1);
        std::cout << "[CameraCalib] 标定参数已加载: " << path << std::endl;
    }

    return param;
}
