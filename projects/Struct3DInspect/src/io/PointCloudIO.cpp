#include "PointCloudIO.h"
#include <fstream>

bool SavePLY(const std::string& path, const std::vector<Point3D>& cloud) {
    std::ofstream f(path);
    if (!f) return false;
    f << "ply\nformat ascii 1.0\nelement vertex " << cloud.size() << "\n";
    f << "property float x\nproperty float y\nproperty float z\nend_header\n";
    for (auto& p : cloud)
        f << p.x << " " << p.y << " " << p.z << "\n";
    return true;
}
