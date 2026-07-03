#include "PLYWriter.h"
#include <fstream>
#include <iostream>

bool SavePLY(const std::string& path, const std::vector<Point3D>& points) {
    std::ofstream fout(path);
    if (!fout) {
        std::cout << "[PLYWriter] 无法打开文件: " << path << std::endl;
        return false;
    }

    fout << "ply\n";
    fout << "format ascii 1.0\n";
    fout << "element vertex " << points.size() << "\n";
    fout << "property float x\n";
    fout << "property float y\n";
    fout << "property float z\n";
    fout << "end_header\n";

    for (const auto& p : points) {
        fout << p.x << " " << p.y << " " << p.z << "\n";
    }

    std::cout << "[PLYWriter] PLY保存完成: " << path << " (" << points.size() << " 个点)" << std::endl;
    return true;
}
