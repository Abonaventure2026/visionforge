#include "CSVWriter.h"
#include <fstream>
#include <iomanip>

bool ExportCSV(const std::string& path, const InspectResult& res) {
    std::ofstream f(path);
    if (!f) return false;
    f << "Width(mm),Height(mm),Thickness(mm),PointCount\n";
    f << std::fixed << std::setprecision(3)
      << res.width_mm << "," << res.height_mm << "," << res.thickness_mm << ","
      << res.point_cloud.size() << "\n";
    return true;
}
