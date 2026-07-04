#pragma once
#include "CommonStruct.h"
#include <string>
#include <vector>
#include <mutex>

// ============================================================
// 条件编译：Halcon 相关声明
// ============================================================
#ifdef USE_HALCON
    #ifndef __APPLE__
        #include "HalconCpp.h"
        #include "HDevThread.h"
        #if defined(__linux__) && !defined(__arm__) && !defined(NO_EXPORT_APP_MAIN)
            #include <X11/Xlib.h>
        #endif
    #else
        #ifndef HC_LARGE_IMAGES
            #include <HALCONCpp/HalconCpp.h>
            #include <HALCONCpp/HDevThread.h>
        #else
            #include <HALCONCppxl/HalconCpp.h>
            #include <HALCONCppxl/HDevThread.h>
        #endif
        #include <stdio.h>
        #include <HALCON/HpThread.h>
        #include <CoreFoundation/CFRunLoop.h>
    #endif
    
    using namespace HalconCpp;
#else
    // 打桩模式：声明 Mock 类型
    #include <iostream>
    #include <string>
#endif

// ============================================================
// 绘图工具函数
// ============================================================
void disp_continue_message(HTuple hv_WindowHandle, HTuple hv_Color, HTuple hv_Box);
void set_display_font(HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font,
                      HTuple hv_Bold, HTuple hv_Slant);
void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
                  HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box);

// ============================================================
// 通用图像工具类
// ============================================================
class CHalconImageTool
{
public:
    static int ReadImageFile(const std::string& imgPath, HObject& outImage, SImageInfo& outInfo);
    static int CreateImageWindow(const HObject& image, HTuple& outWinHandle,
                                  int offsetX = 0, int offsetY = 80);
    static void ClearAllWindow();
    static int SaveImageFile(const HObject& image, const std::string& savePath);
    static int ReadImageToWindow(const std::string& strPath, HTuple& hv_WindowHandle,
                                  HObject& ho_Image, HTuple& hv_Width, HTuple& hv_Height,
                                  int winX = 0, int winY = 80);
    static int GetImageInfo(const HObject& image, SImageInfo& outInfo);
    static HObject CreateMockImage(int width = 640, int height = 480, int channel = 3);
};

// ============================================================
// 统一资源管理单例
// ============================================================
class CResourceMgr
{
public:
    static CResourceMgr& GetInstance();
    
    void RegisterImage(HObject* pImage);
    void UnregisterImage(HObject* pImage);
    void ClearAllImages();
    
    void RegisterWindow(HTuple* pWindow);
    void UnregisterWindow(HTuple* pWindow);
    void ClearAllWindows();
    
    void RegisterModel(void* pModel);
    void UnregisterModel(void* pModel);
    void ClearAllModels();
    
    void CleanupAll();
    
    size_t GetImageCount() const;
    size_t GetWindowCount() const;
    size_t GetModelCount() const;

private:
    CResourceMgr() = default;
    ~CResourceMgr() { CleanupAll(); }
    CResourceMgr(const CResourceMgr&) = delete;
    CResourceMgr& operator=(const CResourceMgr&) = delete;
    
    mutable std::mutex m_mutex;
    
#ifdef USE_HALCON
    std::vector<HObject*> m_images;
    std::vector<HTuple*> m_windows;
#else
    std::vector<void*> m_images;
    std::vector<void*> m_windows;
#endif
    std::vector<void*> m_models;
    
    size_t m_imageCount = 0;
    size_t m_windowCount = 0;
    size_t m_modelCount = 0;
};

// ============================================================
// 打桩模式专用：模拟数据生成器
// ============================================================
#ifndef USE_HALCON
class CMockDataGenerator
{
public:
    static SOCRResult GenerateMockOCRResult(int charCount = 8);
    static SDefectResult GenerateMockDefectResult(int defectCount = 3);
    static SMatchResult GenerateMockMatchResult(int matchCount = 2);
    static SStruct3DResult GenerateMock3DResult(int width = 640, int height = 480);
};
#endif // USE_HALCON
