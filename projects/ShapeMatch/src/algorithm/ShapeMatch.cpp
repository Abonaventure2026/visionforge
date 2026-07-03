#include "ShapeMatch.h"
#include <algorithm>
#include <cstring>
#include <omp.h>
#include <cmath>

std::vector<MyPoint2i> ShapeMatching::FilterOutlierRANSAC(const std::vector<MyPoint2i>& srcPts, const std::vector<MyPoint2i>& tempPts, float thresh) {
    if (srcPts.size() != tempPts.size()) return srcPts;
    std::vector<MyPoint2i> inlier;
    for (size_t i = 0; i < srcPts.size(); i++) {
        float dx = std::abs(static_cast<float>(srcPts[i].x - tempPts[i].x));
        float dy = std::abs(static_cast<float>(srcPts[i].y - tempPts[i].y));
        if (dx < thresh && dy < thresh) inlier.push_back(srcPts[i]);
    }
    return inlier;
}

int ShapeMatching::CreateModel(const Image& grayTemp, float maxContrast, float minContrast) {
    if (grayTemp.width == 0 || grayTemp.height == 0) return 0;
    singleModel = TemplateModel();
    Image blur(grayTemp.width, grayTemp.height, 1);
    GaussianBlur5x5(grayTemp, blur);
    Image dx(grayTemp.width, grayTemp.height, 1);
    Image dy(grayTemp.width, grayTemp.height, 1);
    Sobel5x5(blur, dx, dy);

    float edgeThresh = maxContrast;
    std::vector<MyPoint2i> edgePts;
    for (int y = 0; y < grayTemp.height; y++) {
        for (int x = 0; x < grayTemp.width; x++) {
            float gx = dx.data[y * grayTemp.width + x];
            float gy = dy.data[y * grayTemp.width + x];
            float mag = std::sqrt(gx*gx + gy*gy);
            if (mag > edgeThresh) {
                edgePts.emplace_back(x, y);
            }
        }
    }

    if (edgePts.size() < 30) {
        PrintLog("警告：模板边缘点过少(" + std::to_string(edgePts.size()) + ")，可能阈值过高");
        return -1;
    }
    if (edgePts.size() > 8000) {
        PrintLog("警告：模板边缘点过多(" + std::to_string(edgePts.size()) + ")，可能阈值过低");
        return -1;
    }

    singleModel.width = grayTemp.width;
    singleModel.height = grayTemp.height;
    for (auto& p : edgePts) {
        int idx = p.y * grayTemp.width + p.x;
        float gxV = dx.data[idx];
        float gyV = dy.data[idx];
        float mag = std::sqrt(gxV * gxV + gyV * gyV);
        singleModel.edgePoints.push_back(p);
        singleModel.gx.push_back(static_cast<double>(gxV));
        singleModel.gy.push_back(static_cast<double>(gyV));
        singleModel.invMag.push_back(mag > 1e-8 ? 1.0 / mag : 0.0);
    }
    singleModel.valid = true;
    PrintLog("模板边缘点数量: " + std::to_string(edgePts.size()));
    return 1;
}

int ShapeMatching::CreateAngleLib(const Image& grayTemp, float maxC, float minC, double startA, double endA, double stepA) {
    transformLib.Clear();
    for (double ang = startA; ang <= endA; ang += stepA) {
        Image rotImg = grayTemp.Rotate(ang);
        TemplateModel md;
        int ret = CreateModel(rotImg, maxC, minC);
        if (ret > 0) {
            md.angle = ang;
            md.scale = 1.0;
            transformLib.models.push_back(md);
        }
    }
    PrintLog("角度库生成，模型数: " + std::to_string(transformLib.models.size()));
    return static_cast<int>(transformLib.models.size());
}

int ShapeMatching::CreateScaleLib(const Image& grayTemp, float maxC, float minC, double startS, double endS, double stepS) {
    transformLib.Clear();
    for (double sc = startS; sc <= endS; sc += stepS) {
        int nw = static_cast<int>(grayTemp.width * sc);
        int nh = static_cast<int>(grayTemp.height * sc);
        if (nw < 10 || nh < 10) continue;
        Image scaleImg = grayTemp.Resize(nw, nh);
        TemplateModel md;
        int ret = CreateModel(scaleImg, maxC, minC);
        if (ret > 0) {
            md.scale = sc;
            md.angle = 0.0;
            transformLib.models.push_back(md);
        }
    }
    PrintLog("尺度库生成，模型数: " + std::to_string(transformLib.models.size()));
    return static_cast<int>(transformLib.models.size());
}

int ShapeMatching::CreateTransformLib(const Image& grayTemp, float maxC, float minC,
                                      double startA, double endA, double stepA,
                                      double startS, double endS, double stepS) {
    transformLib.Clear();
    for (double s = startS; s <= endS + 1e-6; s += stepS) {
        int nw = static_cast<int>(grayTemp.width * s);
        int nh = static_cast<int>(grayTemp.height * s);
        if (nw < 10 || nh < 10) continue;
        Image scaled = grayTemp.Resize(nw, nh);
        for (double a = startA; a <= endA + 1e-6; a += stepA) {
            Image rot = scaled.Rotate(a);
            TemplateModel md;
            int ret = CreateModel(rot, maxC, minC);
            if (ret > 0) {
                md.angle = a;
                md.scale = s;
                transformLib.models.push_back(md);
            }
        }
    }
    PrintLog("联合变换库生成，模型数: " + std::to_string(transformLib.models.size()));
    return static_cast<int>(transformLib.models.size());
}

double ShapeMatching::SearchMatch(const Image& graySearch, int x0, int y0, int x1, int y1, float minScore, float greed, MyPoint2i& resPt) {
    if (!singleModel.valid || graySearch.width == 0 || graySearch.height == 0) return -1.0;
    int w = graySearch.width, h = graySearch.height;
    Image blur(w, h, 1);
    GaussianBlur5x5(graySearch, blur);
    Image dx(w, h, 1), dy(w, h, 1);
    Sobel5x5(blur, dx, dy);

    double bestScore = -1.0;
    MyPoint2i bestPt(0, 0);
    int total = static_cast<int>(singleModel.edgePoints.size());
    if (total == 0) return -1.0;

    int step = 1;
    #pragma omp parallel for reduction(max:bestScore)
    for (int y = y0; y < y1; y += step) {
        for (int x = x0; x < x1; x += step) {
            double sumDot = 0.0;
            int validCount = 0;
            for (int m = 0; m < total; m++) {
                int cx = x + singleModel.edgePoints[m].x;
                int cy = y + singleModel.edgePoints[m].y;
                if (cx < 0 || cx >= w || cy < 0 || cy >= h) continue;
                int idx = cy * w + cx;
                double sx = static_cast<double>(dx.data[idx]);
                double sy = static_cast<double>(dy.data[idx]);
                double smag = std::sqrt(sx*sx + sy*sy);
                if (smag < 1e-6) continue;
                double dot = sx * singleModel.gx[m] + sy * singleModel.gy[m];
                double tmag = 1.0 / singleModel.invMag[m];
                double normDot = dot / (tmag * smag);
                sumDot += normDot;
                validCount++;
            }
            if (validCount == 0) continue;
            double score = sumDot / validCount;
            if (score > bestScore) {
                bestScore = score;
                bestPt = MyPoint2i(x, y);
            }
        }
    }
    resPt = bestPt;
    PrintLog("最佳匹配分数: " + std::to_string(bestScore) + " 位置(" + std::to_string(bestPt.x) + "," + std::to_string(bestPt.y) + ")");
    return bestScore;
}

double ShapeMatching::SearchMultiTransform(const Image& graySearch, int x0, int y0, int x1, int y1, float minScore, float greed, MyPoint2i& resPt, double& outA, double& outS) {
    if (transformLib.models.empty()) {
        double sc = SearchMatch(graySearch, x0, y0, x1, y1, minScore, greed, resPt);
        outA = 0.0; outS = 1.0;
        return sc;
    }
    double bestSc = -1.0;
    MyPoint2i bestPt(0,0);
    double bestA = 0, bestS = 1.0;
    #pragma omp parallel for
    for (int i = 0; i < (int)transformLib.models.size(); ++i) {
        TemplateModel bak = singleModel;
        singleModel = transformLib.models[i];
        MyPoint2i curPt;
        double sc = SearchMatch(graySearch, x0, y0, x1, y1, minScore, greed, curPt);
        #pragma omp critical
        {
            if (sc > bestSc) {
                bestSc = sc;
                bestPt = curPt;
                bestA = transformLib.models[i].angle;
                bestS = transformLib.models[i].scale;
            }
        }
        singleModel = bak;
    }
    resPt = bestPt;
    outA = bestA;
    outS = bestS;
    return bestSc;
}

MyPoint2i ShapeMatching::PyramidMatch(Image& tempGray, Image& searchGray, int pyrLv, int lowT, int highT, float minSc, float greed,
                                       double& outScore, double& outAngle, double& outScale,
                                       bool enableRot, bool enableScale,
                                       double rotS, double rotE, double rotStep,
                                       double scaleS, double scaleE, double scaleStep) {
    (void)pyrLv; // 忽略金字塔层数，固定用原图
    Image tempPyr0 = tempGray;
    Image srcPyr0 = searchGray;
    MyPoint2i bestPos(-1, -1);
    double bestScore = -1.0;
    double bestAngle = 0.0, bestScale = 1.0;

    if (enableRot || enableScale) {
        if (enableRot && enableScale) {
            CreateTransformLib(tempPyr0, static_cast<float>(highT), static_cast<float>(lowT),
                               rotS, rotE, rotStep, scaleS, scaleE, scaleStep);
        } else if (enableRot) {
            CreateAngleLib(tempPyr0, static_cast<float>(highT), static_cast<float>(lowT), rotS, rotE, rotStep);
        } else if (enableScale) {
            CreateScaleLib(tempPyr0, static_cast<float>(highT), static_cast<float>(lowT), scaleS, scaleE, scaleStep);
        }
        MyPoint2i tmpPos;
        double tmpA, tmpS;
        int x1 = std::max(2, srcPyr0.width - tempPyr0.width);
        int y1 = std::max(2, srcPyr0.height - tempPyr0.height);
        double sc = SearchMultiTransform(srcPyr0, 1, 1, x1, y1, minSc, greed, tmpPos, tmpA, tmpS);
        if (sc > bestScore) {
            bestScore = sc;
            bestPos = tmpPos;
            bestAngle = tmpA;
            bestScale = tmpS;
        }
    } else {
        CreateModel(tempPyr0, static_cast<float>(highT), static_cast<float>(lowT));
        MyPoint2i tmpPos;
        int x1 = std::max(2, srcPyr0.width - tempPyr0.width);
        int y1 = std::max(2, srcPyr0.height - tempPyr0.height);
        double sc = SearchMatch(srcPyr0, 1, 1, x1, y1, minSc, greed, tmpPos);
        if (sc > bestScore) {
            bestScore = sc;
            bestPos = tmpPos;
            bestAngle = 0; bestScale = 1.0;
        }
    }

    bestPos = SubpixelRefine(searchGray, tempGray, bestPos, bestAngle, bestScale);
    outScore = bestScore;
    outAngle = bestAngle;
    outScale = bestScale;
    return bestPos;
}

void ShapeMatching::DrawMatchPoint(Image& dstImg, MyPoint2i pos) {
    if (!singleModel.valid || dstImg.width == 0) return;
    for (auto& p : singleModel.edgePoints) {
        MyPoint2i dp(p.x + pos.x, p.y + pos.y);
        if (dp.x >= 0 && dp.x < dstImg.width && dp.y >= 0 && dp.y < dstImg.height) {
            int idx = (dp.y * dstImg.width + dp.x) * 3;
            dstImg.data[idx] = 0;
            dstImg.data[idx+1] = 0;
            dstImg.data[idx+2] = 255;
        }
    }
}

void ShapeMatching::DrawMatchPointWithTransform(Image& dstImg, MyPoint2i pos, double angle, double scale) {
    DrawMatchPoint(dstImg, pos);
}

#ifdef USE_OPENCV
int ShapeMatching::CreateModelCV(const Mat& temp, double maxC, double minC) {
    Image img(temp.cols, temp.rows, temp.channels());
    memcpy(img.data, temp.data, temp.total() * temp.channels());
    return CreateModel(img, static_cast<float>(maxC), static_cast<float>(minC));
}
double ShapeMatching::SearchMatchCV(const Mat& src, int bh, int bw, int eh, int ew, double minSc, double greed, Point& outPt) {
    Image img(src.cols, src.rows, src.channels());
    memcpy(img.data, src.data, src.total() * src.channels());
    MyPoint2i pt;
    double sc = SearchMatch(img, bw, bh, ew, eh, static_cast<float>(minSc), static_cast<float>(greed), pt);
    outPt.x = pt.x; outPt.y = pt.y;
    return sc;
}
Point ShapeMatching::PyramidMatchCV(Mat& tg, Mat& sg, int lv, int lt, int ht, double ms, double gr, double& sc, double& ang, double& scl) {
    Image tImg(tg.cols, tg.rows, tg.channels());
    memcpy(tImg.data, tg.data, tg.total() * tg.channels());
    Image sImg(sg.cols, sg.rows, sg.channels());
    memcpy(sImg.data, sg.data, sg.total() * sg.channels());
    Image tGray = tImg.ToGray();
    Image sGray = sImg.ToGray();
    MyPoint2i res = PyramidMatch(tGray, sGray, lv, lt, ht, static_cast<float>(ms), static_cast<float>(gr), sc, ang, scl, false, false, 0,0,0,1,1,0);
    return Point(res.x, res.y);
}
void ShapeMatching::DrawMatchCV(Mat& dst, Point pos) {
    for (auto& p : singleModel.edgePoints)
        circle(dst, Point(p.x + pos.x, p.y + pos.y), 1, Scalar(0,0,255), 1);
}
#endif
