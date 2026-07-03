#pragma once
#include <QImage>
#include <QPoint>
#include <QVector>
#include <QLine>
#include <climits>
#include <cmath>

class StructureMeasure {
public:
    struct Result {
        bool hasBottomPoint = false;
        QPoint bottomPoint;
        bool hasCenterLine = false;
        QLine centerLine;
        QVector<QLine> boundaryLines;
        QLine topLine;          // 上端横线
        QLine bottomLine;       // 下端横线
        bool hasLength = false;
        double length = 0.0;    // 支持亚像素
        double topWidth = 0.0;  // 支持亚像素
        double bottomWidth = 0.0; // 支持亚像素
        double leftAngle = 0;   // 左锥角 (度)
        double rightAngle = 0;  // 右锥角 (度)
        double symDiff = 0;     // 对称度
        QPoint bottomCoord;
    };

    Result process(const QImage &img, int morphKernel = 2);

private:
    // 基础预处理
    QImage toGray(const QImage &img);
    QImage gaussianBlur5(const QImage &gray);
    int calcOtsuThresh(const QImage &gray);
    QImage thresholdOtsu(const QImage &gray);
    QImage morphClose(const QImage &bin, int ksize);
    // 最大连通域保留
    QImage keepMaxRegion(const QImage &bin);
    // 双采样：Canny边缘优先
    QVector<QPoint> extractCannyEdge(const QImage &gray, int t1 = 30, int t2 = 90);
    QVector<QPoint> extractValidPoints(const QImage &binary);
    // 包围框计算
    void getRectBound(const QVector<QPoint> &pts, int &minX, int &maxX, int &minY, int &maxY);
    // 分割左右轮廓（限定Y区间）
    void splitLeftRightByRect(const QVector<QPoint> &allPts, QVector<QPoint> &left, QVector<QPoint> &right, int yMin, int yMax);
    // 直线工具
    double pointDistLine(QPoint p, QLine line);
    QLine fitLS(const QVector<QPoint> &pts);
    QLine fitRobustRANSAC(const QVector<QPoint> &pts, int yMin, int yMax, double distThr = 2.5, int iter = 80);
    // XY双向截断
    QLine clipLineFull(QLine raw, int xMin, int xMax, int yMin, int yMax);
    // 几何计算
    double calcLineLen(QLine line);  // 返回 double 支持亚像素
    double calcLineAngle(QLine line, QLine center);
    QLine getTopHorizontal(int xMin, int xMax, int yTop);
    QLine getBottomHorizontal(int xMin, int xMax, int yBot);
};
