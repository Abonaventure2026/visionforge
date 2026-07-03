#include "StructureMeasure.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <QDebug>
#include <random>
#include <vector>
#include <map>
#include <climits>
#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
#endif

// ========== 基础预处理 ==========
QImage StructureMeasure::toGray(const QImage &img) {
    QImage gray(img.size(), QImage::Format_Grayscale8);
    int w = img.width(), h = img.height();
    for(int y=0; y<h; ++y) {
        const QRgb* src = (const QRgb*)img.constScanLine(y);
        uchar* dst = gray.scanLine(y);
        for(int x=0; x<w; ++x) {
            QRgb c = src[x];
            dst[x] = (uchar)(0.299*qRed(c) + 0.587*qGreen(c) + 0.114*qBlue(c));
        }
    }
    return gray;
}

QImage StructureMeasure::gaussianBlur5(const QImage &gray) {
    int w = gray.width(), h = gray.height();
    QImage out(w, h, QImage::Format_Grayscale8);
    const int kernel[5][5] = {
        {1,2,4,2,1},
        {2,4,8,4,2},
        {4,8,16,8,4},
        {2,4,8,4,2},
        {1,2,4,2,1}
    };
    int sumK = 136;
    for(int y=0; y<h; ++y) {
        uchar* dstRow = out.scanLine(y);
        for(int x=0; x<w; ++x) {
            int total = 0;
            for(int dy=-2; dy<=2; ++dy) {
                int ny = y + dy;
                if(ny < 0) ny = -ny;
                if(ny >= h) ny = 2*(h-1) - ny;
                const uchar* srcRow = gray.constScanLine(ny);
                for(int dx=-2; dx<=2; ++dx) {
                    int nx = x + dx;
                    if(nx < 0) nx = -nx;
                    if(nx >= w) nx = 2*(w-1) - nx;
                    total += srcRow[nx] * kernel[dy+2][dx+2];
                }
            }
            dstRow[x] = total / sumK;
        }
    }
    return out;
}

int StructureMeasure::calcOtsuThresh(const QImage &gray) {
    int hist[256] = {0};
    int w = gray.width(), h = gray.height();
    int total = w * h;
    for(int y=0; y<h; ++y) {
        const uchar* row = gray.constScanLine(y);
        for(int x=0; x<w; ++x) hist[row[x]]++;
    }
    double sumAll = 0;
    for(int i=0; i<256; ++i) sumAll += (double)i * hist[i];
    double w0 = 0, sum0 = 0, maxVar = 0;
    int bestT = 128;
    for(int t=0; t<256; ++t) {
        w0 += hist[t];
        if(w0 < 1e-3) continue;
        double w1 = total - w0;
        if(w1 < 1e-3) break;
        sum0 += (double)t * hist[t];
        double u0 = sum0 / w0;
        double u1 = (sumAll - sum0) / w1;
        double var = w0 * w1 * (u0 - u1) * (u0 - u1);
        if(var > maxVar) {
            maxVar = var;
            bestT = t;
        }
    }
    return bestT;
}

QImage StructureMeasure::thresholdOtsu(const QImage &gray) {
    int T = calcOtsuThresh(gray);
    int w = gray.width(), h = gray.height();
    QImage bin(w, h, QImage::Format_Grayscale8);
    for(int y=0; y<h; ++y) {
        const uchar* src = gray.constScanLine(y);
        uchar* dst = bin.scanLine(y);
        for(int x=0; x<w; ++x) {
            dst[x] = (src[x] < T) ? 255 : 0;
        }
    }
    return bin;
}

// 形态学闭运算：先膨胀后腐蚀
QImage StructureMeasure::morphClose(const QImage &bin, int ksize) {
    int w = bin.width(), h = bin.height();
    QImage tmp = bin.copy();
    // 膨胀
    QImage dilated(w, h, QImage::Format_Grayscale8);
    for(int y=ksize; y<h-ksize; ++y) {
        for(int x=ksize; x<w-ksize; ++x) {
            bool hasWhite = false;
            for(int dy=-ksize; dy<=ksize && !hasWhite; ++dy) {
                const uchar* row = tmp.constScanLine(y+dy);
                for(int dx=-ksize; dx<=ksize; ++dx) {
                    if(row[x+dx] == 255) { hasWhite = true; break; }
                }
            }
            dilated.scanLine(y)[x] = hasWhite ? 255 : 0;
        }
    }
    // 边缘复制
    for(int x=0; x<w; ++x) {
        dilated.scanLine(0)[x] = tmp.scanLine(0)[x];
        dilated.scanLine(h-1)[x] = tmp.scanLine(h-1)[x];
    }
    for(int y=0; y<h; ++y) {
        dilated.scanLine(y)[0] = tmp.scanLine(y)[0];
        dilated.scanLine(y)[w-1] = tmp.scanLine(y)[w-1];
    }
    // 腐蚀
    QImage eroded(w, h, QImage::Format_Grayscale8);
    for(int y=ksize; y<h-ksize; ++y) {
        for(int x=ksize; x<w-ksize; ++x) {
            bool allWhite = true;
            for(int dy=-ksize; dy<=ksize && allWhite; ++dy) {
                const uchar* row = dilated.constScanLine(y+dy);
                for(int dx=-ksize; dx<=ksize; ++dx) {
                    if(row[x+dx] == 0) { allWhite = false; break; }
                }
            }
            eroded.scanLine(y)[x] = allWhite ? 255 : 0;
        }
    }
    for(int x=0; x<w; ++x) {
        eroded.scanLine(0)[x] = dilated.scanLine(0)[x];
        eroded.scanLine(h-1)[x] = dilated.scanLine(h-1)[x];
    }
    for(int y=0; y<h; ++y) {
        eroded.scanLine(y)[0] = dilated.scanLine(y)[0];
        eroded.scanLine(y)[w-1] = dilated.scanLine(y)[w-1];
    }
    return eroded;
}

// ========== 最大连通域过滤 ==========
QImage StructureMeasure::keepMaxRegion(const QImage &bin) {
    int w = bin.width(), h = bin.height();
    QImage label(w, h, QImage::Format_Grayscale8);
    label.fill(0);
    int labelId = 1;
    std::map<int, int> areaMap;
    for(int y=0; y<h; ++y) {
        for(int x=0; x<w; ++x) {
            if(bin.scanLine(y)[x] == 255 && label.scanLine(y)[x] == 0) {
                std::vector<QPoint> stack;
                stack.push_back(QPoint(x,y));
                label.scanLine(y)[x] = labelId;
                int cnt = 0;
                while(!stack.empty()) {
                    QPoint p = stack.back(); stack.pop_back();
                    cnt++;
                    int px = p.x(), py = p.y();
                    // 4邻域
                    int dxs[4] = {-1,1,0,0};
                    int dys[4] = {0,0,-1,1};
                    for(int k=0; k<4; ++k) {
                        int nx = px + dxs[k], ny = py + dys[k];
                        if(nx>=0 && nx<w && ny>=0 && ny<h) {
                            if(bin.scanLine(ny)[nx] == 255 && label.scanLine(ny)[nx] == 0) {
                                label.scanLine(ny)[nx] = labelId;
                                stack.push_back(QPoint(nx,ny));
                            }
                        }
                    }
                }
                areaMap[labelId] = cnt;
                labelId++;
            }
        }
    }
    // 找最大区域
    int maxId = 1, maxArea = 0;
    for(auto &p : areaMap) {
        if(p.second > maxArea) {
            maxArea = p.second;
            maxId = p.first;
        }
    }
    QImage out(w, h, QImage::Format_Grayscale8);
    out.fill(0);
    for(int y=0; y<h; ++y) {
        for(int x=0; x<w; ++x) {
            if(label.scanLine(y)[x] == maxId)
                out.scanLine(y)[x] = 255;
        }
    }
    return out;
}

// ========== 双采样策略 ==========
QVector<QPoint> StructureMeasure::extractCannyEdge(const QImage &gray, int t1, int t2) {
    int w = gray.width(), h = gray.height();
    QVector<QPoint> edges;
    for(int y=1; y<h-1; ++y) {
        const uchar* r0 = gray.constScanLine(y-1);
        const uchar* r1 = gray.constScanLine(y);
        const uchar* r2 = gray.constScanLine(y+1);
        for(int x=1; x<w-1; ++x) {
            int gx = -r0[x-1] - 2*r0[x] - r0[x+1] + r2[x-1] + 2*r2[x] + r2[x+1];
            int gy = -r0[x-1] + r0[x+1] - 2*r1[x-1] + 2*r1[x] - r2[x-1] + r2[x+1];
            float mag = std::sqrt((float)(gx*gx + gy*gy));
            if(mag > t2) edges.append(QPoint(x,y));
        }
    }
    return edges;
}

QVector<QPoint> StructureMeasure::extractValidPoints(const QImage &binary) {
    QVector<QPoint> pts;
    int w = binary.width(), h = binary.height();
    for(int y=0; y<h; ++y) {
        const uchar* row = binary.constScanLine(y);
        int l=-1, r=-1;
        for(int x=0; x<w; ++x) {
            if(row[x] == 255) {
                if(l == -1) l = x;
                r = x;
            }
        }
        if(l == -1 || (r-l) < 5) continue;
        for(int x=l; x<=r; ++x) pts.append(QPoint(x,y));
    }
    return pts;
}

// ========== 包围矩形 ==========
void StructureMeasure::getRectBound(const QVector<QPoint> &pts, int &minX, int &maxX, int &minY, int &maxY) {
    minX = INT_MAX; maxX = 0; minY = INT_MAX; maxY = 0;
    for(const auto &p : pts) {
        if(p.x() < minX) minX = p.x();
        if(p.x() > maxX) maxX = p.x();
        if(p.y() < minY) minY = p.y();
        if(p.y() > maxY) maxY = p.y();
    }
}

// ========== 分割左右 ==========
void StructureMeasure::splitLeftRightByRect(const QVector<QPoint> &allPts, QVector<QPoint> &left, QVector<QPoint> &right, int yMin, int yMax) {
    int minX, maxX, minY, maxY;
    getRectBound(allPts, minX, maxX, minY, maxY);
    int midX = (minX + maxX) / 2;
    for(const auto &p : allPts) {
        if(p.y() < yMin || p.y() > yMax) continue;
        if(p.x() < midX - 1) left.push_back(p);
        else if(p.x() > midX + 1) right.push_back(p);
    }
}

// ========== 直线工具 ==========
double StructureMeasure::pointDistLine(QPoint p, QLine line) {
    QPoint p1 = line.p1(), p2 = line.p2();
    double A = p2.y() - p1.y();
    double B = p1.x() - p2.x();
    double C = 1.0 * p2.x() * p1.y() - 1.0 * p1.x() * p2.y();
    return fabs(A * p.x() + B * p.y() + C) / sqrt(A*A + B*B);
}

QLine StructureMeasure::fitLS(const QVector<QPoint> &pts) {
    if(pts.size() < 2) return QLine(0,0,0,0);
    double sumX=0, sumY=0, sumXX=0, sumXY=0;
    int n = pts.size();
    for(const auto &p : pts) {
        sumX += p.x(); sumY += p.y();
        sumXX += 1.0 * p.x() * p.x();
        sumXY += 1.0 * p.x() * p.y();
    }
    double denom = n * sumXX - sumX * sumX;
    if(fabs(denom) < 1e-6) return QLine(pts.first(), pts.last());
    double k = (n * sumXY - sumX * sumY) / denom;
    double b = (sumY - k * sumX) / n;
    int minX = pts[0].x(), maxX = pts[0].x();
    for(const auto &p : pts) {
        if(p.x() < minX) minX = p.x();
        if(p.x() > maxX) maxX = p.x();
    }
    int y1 = (int)(k * minX + b);
    int y2 = (int)(k * maxX + b);
    return QLine(minX, y1, maxX, y2);
}

QLine StructureMeasure::fitRobustRANSAC(const QVector<QPoint> &pts, int yMin, int yMax, double distThr, int iter) {
    QVector<QPoint> validPts;
    for(const auto &p : pts) {
        if(p.y() >= yMin && p.y() <= yMax) validPts.push_back(p);
    }
    if(validPts.size() < 6) return fitLS(validPts);

    std::mt19937 rng(std::random_device{}());
    int bestCnt = 0;
    QLine bestLine;
    bool hasBestLine = false;

    // 动态调整距离阈值
    double adaptiveDistThr = distThr;

    for(int t=0; t<iter; ++t) {
        std::uniform_int_distribution<int> dist(0, validPts.size()-1);
        int i1 = dist(rng), i2 = dist(rng);
        int attempts = 0;
        while((fabs(validPts[i1].y() - validPts[i2].y()) < 10 || i1 == i2) && attempts < 50) {
            i2 = dist(rng);
            attempts++;
        }
        if(attempts >= 50) continue;

        QLine temp(validPts[i1], validPts[i2]);

        // 检查直线是否有效（非垂直且斜率合理）
        if(fabs(temp.p2().x() - temp.p1().x()) < 1e-3) continue;
        double slope = fabs((double)(temp.p2().y() - temp.p1().y()) / (temp.p2().x() - temp.p1().x()));
        if(slope > 10.0) continue;  // 斜率过大，可能是异常采样

        int cnt = 0;
        for(const auto &p : validPts) {
            if(pointDistLine(p, temp) < adaptiveDistThr) cnt++;
        }

        // 如果当前模型更好，更新最佳模型
        if(cnt > bestCnt || !hasBestLine) {
            bestCnt = cnt;
            bestLine = temp;
            hasBestLine = true;

            // 如果内点比例足够高，提前终止
            if(cnt > validPts.size() * 0.8) break;
        }
    }

    // 如果未找到有效直线，返回最小二乘拟合
    if(!hasBestLine) {
        return fitLS(validPts);
    }

    // 收集内点
    QVector<QPoint> inliers;
    for(const auto &p : validPts) {
        if(pointDistLine(p, bestLine) < adaptiveDistThr) inliers.push_back(p);
    }

    // 如果内点太少，降级为最小二乘
    if(inliers.size() < 4) {
        return fitLS(validPts);
    }

    return fitLS(inliers);
}

// ========== XY双向截断 ==========
QLine StructureMeasure::clipLineFull(QLine raw, int xMin, int xMax, int yMin, int yMax) {
    QPoint p1 = raw.p1(), p2 = raw.p2();
    bool vert = fabs(p2.x() - p1.x()) < 1e-3;
    if(vert) {
        int x = p1.x();
        if(x < xMin) x = xMin;
        if(x > xMax) x = xMax;
        return QLine(QPoint(x, yMin), QPoint(x, yMax));
    }
    double k = (double)(p2.y() - p1.y()) / (p2.x() - p1.x());
    double b = p1.y() - k * p1.x();
    // 计算在yMin和yMax处的x
    int xTop = (int)((yMin - b) / k);
    int xBot = (int)((yMax - b) / k);
    // 限制在xMin~xMax
    xTop = std::max(xMin, std::min(xMax, xTop));
    xBot = std::max(xMin, std::min(xMax, xBot));
    return QLine(QPoint(xTop, yMin), QPoint(xBot, yMax));
}

// ========== 几何计算（支持亚像素） ==========
double StructureMeasure::calcLineLen(QLine line) {
    double dx = line.p2().x() - line.p1().x();
    double dy = line.p2().y() - line.p1().y();
    return sqrt(dx*dx + dy*dy);
}

double StructureMeasure::calcLineAngle(QLine line, QLine center) {
    QPoint c1 = center.p1(), c2 = center.p2();
    QPoint l1 = line.p1(), l2 = line.p2();
    double vx = c2.x() - c1.x(), vy = c2.y() - c1.y();
    double lx = l2.x() - l1.x(), ly = l2.y() - l1.y();
    double dot = vx*lx + vy*ly;
    double modV = sqrt(vx*vx + vy*vy);
    double modL = sqrt(lx*lx + ly*ly);
    if(modV < 1e-6 || modL < 1e-6) return 0;
    double rad = acos(dot / (modV * modL));
    return rad * 180.0 / M_PI;
}

QLine StructureMeasure::getTopHorizontal(int xMin, int xMax, int yTop) {
    return QLine(QPoint(xMin, yTop), QPoint(xMax, yTop));
}

QLine StructureMeasure::getBottomHorizontal(int xMin, int xMax, int yBot) {
    return QLine(QPoint(xMin, yBot), QPoint(xMax, yBot));
}

// ========== 主处理流程 ==========
StructureMeasure::Result StructureMeasure::process(const QImage &img, int morphKernel) {
    Result res;
    // 1. 预处理
    QImage gray = toGray(img);
    QImage blur = gaussianBlur5(gray);
    QImage bin = thresholdOtsu(blur);
    QImage clean = morphClose(bin, morphKernel);
    QImage workpiece = keepMaxRegion(clean);   // 过滤杂点

    // 2. 双采样：优先Canny边缘
    QVector<QPoint> edgePts = extractCannyEdge(gray, 25, 80);
    QVector<QPoint> allPts;
    if(edgePts.size() >= 60) {
        allPts = edgePts;
        qDebug() << "使用Canny边缘点数量:" << allPts.size();
    } else {
        allPts = extractValidPoints(workpiece);
        qDebug() << "降级使用二值填充点数量:" << allPts.size();
    }

    if(allPts.size() < 80) {
        qDebug() << "有效轮廓点不足，识别失败";
        return res;
    }

    // 3. 包围框
    int minX, maxX, minY, maxY;
    getRectBound(allPts, minX, maxX, minY, maxY);

    // 4. 分割左右
    QVector<QPoint> leftPts, rightPts;
    splitLeftRightByRect(allPts, leftPts, rightPts, minY, maxY);
    if(leftPts.empty() || rightPts.empty()) {
        qDebug() << "无法区分左右轮廓";
        return res;
    }

    // 5. 鲁棒拟合
    QLine rawLeft = fitRobustRANSAC(leftPts, minY, maxY, 2.5, 80);
    QLine rawRight = fitRobustRANSAC(rightPts, minY, maxY, 2.5, 80);

    // 6. XY双向截断
    QLine leftLine = clipLineFull(rawLeft, minX, maxX, minY, maxY);
    QLine rightLine = clipLineFull(rawRight, minX, maxX, minY, maxY);
    res.boundaryLines << leftLine << rightLine;

    // 7. 生成中线（左右轮廓几何中心）
    if (leftPts.size() > 0 && rightPts.size() > 0) {
        long long leftSumX = 0, rightSumX = 0;
        for (const auto &p : leftPts) leftSumX += p.x();
        for (const auto &p : rightPts) rightSumX += p.x();
        int leftCenterX = static_cast<int>(leftSumX / leftPts.size());
        int rightCenterX = static_cast<int>(rightSumX / rightPts.size());
        int cx = (leftCenterX + rightCenterX) / 2;
        QPoint topC(cx, minY), botC(cx, maxY);
        res.centerLine = QLine(topC, botC);
        res.hasCenterLine = true;
    } else {
        // 降级方案：使用包围框中心
        int cx = (minX + maxX) / 2;
        QPoint topC(cx, minY), botC(cx, maxY);
        res.centerLine = QLine(topC, botC);
        res.hasCenterLine = true;
    }

    // 8. 上下横线
    res.topLine = getTopHorizontal(minX, maxX, minY);
    res.bottomLine = getBottomHorizontal(minX, maxX, maxY);

    // 9. 测量指标（支持亚像素）
    res.length = calcLineLen(res.centerLine);

    // 计算顶部宽度：在 minY 位置，左右轮廓的X差值
    int topLeftX = INT_MAX, topRightX = INT_MIN;
    int bottomLeftX = INT_MAX, bottomRightX = INT_MIN;
    int yTolerance = 2;  // 允许的Y方向容差

    for (const auto &p : allPts) {
        if (abs(p.y() - minY) <= yTolerance) {
            if (p.x() < topLeftX) topLeftX = p.x();
            if (p.x() > topRightX) topRightX = p.x();
        }
        if (abs(p.y() - maxY) <= yTolerance) {
            if (p.x() < bottomLeftX) bottomLeftX = p.x();
            if (p.x() > bottomRightX) bottomRightX = p.x();
        }
    }

    // 如果顶部/底部点不足，使用左右轮廓线插值
    if (topLeftX == INT_MAX || topRightX == INT_MIN) {
        // 使用轮廓线在 minY 处的X值
        QPoint leftTop = leftLine.p1();
        QPoint rightTop = rightLine.p1();
        topLeftX = leftTop.x();
        topRightX = rightTop.x();
    }
    if (bottomLeftX == INT_MAX || bottomRightX == INT_MIN) {
        QPoint leftBottom = leftLine.p2();
        QPoint rightBottom = rightLine.p2();
        bottomLeftX = leftBottom.x();
        bottomRightX = rightBottom.x();
    }

    res.topWidth = topRightX - topLeftX;
    res.bottomWidth = bottomRightX - bottomLeftX;

    // 10. 计算锥角
    res.leftAngle = calcLineAngle(leftLine, res.centerLine);
    res.rightAngle = calcLineAngle(rightLine, res.centerLine);
    res.symDiff = fabs(res.leftAngle - res.rightAngle);

    // 11. 底部基准点
    QPoint botC = QPoint((minX + maxX) / 2, maxY);
    res.bottomPoint = botC;
    res.bottomCoord = botC;
    res.hasBottomPoint = true;
    res.hasLength = true;

    return res;
}
