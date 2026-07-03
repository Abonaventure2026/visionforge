#pragma once
#include "cross_util.h"
#include "base/image_base.h"
#include <vector>
#include <map>
#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
using namespace cv;
#endif

struct TemplateModel {
    std::vector<MyPoint2i> edgePoints;
    std::vector<double> gx, gy, invMag;
    int width, height;
    bool valid;
    double angle;
    double scale;
    TemplateModel() : valid(false), width(0), height(0), angle(0), scale(1.0) {}
};

struct MultiTemplateLib {
    std::vector<TemplateModel> models;
    void Clear() { models.clear(); }
};

class ShapeMatching {
private:
    TemplateModel singleModel;
    MultiTemplateLib transformLib;
    bool enableMask = false;
    std::vector<MyPoint2i> FilterOutlierRANSAC(const std::vector<MyPoint2i>& srcPts, const std::vector<MyPoint2i>& tempPts, float thresh);
public:
    void SetEnableMask(bool enable) { enableMask = enable; }
    int CreateModel(const Image& grayTemp, float maxC, float minC);
    int CreateAngleLib(const Image& grayTemp, float maxC, float minC, double startA, double endA, double stepA);
    int CreateScaleLib(const Image& grayTemp, float maxC, float minC, double startS, double endS, double stepS);
    int CreateTransformLib(const Image& grayTemp, float maxC, float minC,
                           double startA, double endA, double stepA,
                           double startS, double endS, double stepS);
    double SearchMatch(const Image& graySearch, int x0, int y0, int x1, int y1, float minScore, float greed, MyPoint2i& resPt);
    double SearchMultiTransform(const Image& graySearch, int x0, int y0, int x1, int y1, float minScore, float greed, MyPoint2i& resPt, double& outA, double& outS);
    MyPoint2i PyramidMatch(Image& tempGray, Image& searchGray, int pyrLv, int lowT, int highT, float minSc, float greed,
                           double& outScore, double& outAngle, double& outScale,
                           bool enableRot = true, bool enableScale = true,
                           double rotS = -30, double rotE = 30, double rotStep = 5,
                           double scaleS = 0.8, double scaleE = 1.2, double scaleStep = 0.1);
    void DrawMatchPoint(Image& dstImg, MyPoint2i pos);
    void DrawMatchPointWithTransform(Image& dstImg, MyPoint2i pos, double angle, double scale);

#ifdef USE_OPENCV
    int CreateModelCV(const Mat& temp, double maxC, double minC);
    double SearchMatchCV(const Mat& src, int bh, int bw, int eh, int ew, double minSc, double greed, Point& outPt);
    Point PyramidMatchCV(Mat& tg, Mat& sg, int lv, int lt, int ht, double ms, double gr, double& sc, double& ang, double& scl);
    void DrawMatchCV(Mat& dst, Point pos);
#endif
};
