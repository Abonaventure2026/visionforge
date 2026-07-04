#include "HalconTool.h"
#include "LogService.h"
#include <iostream>
#include <chrono>
#include <algorithm>

#ifdef USE_HALCON
    // ============================================================
    // 真实 Halcon 模式：完整实现（所有绘图函数保留原始Halcon API调用）
    // ============================================================
    
    // ========== disp_continue_message ==========
    void disp_continue_message(HTuple hv_WindowHandle, HTuple hv_Color, HTuple hv_Box)
    {
        HTuple hv_GenParamName, hv_GenParamValue, hv_ContinueMessage;
        hv_GenParamName = HTuple();
        hv_GenParamValue = HTuple();
        if (0 != ((hv_Box.TupleLength()) > 0))
        {
            if (0 != (HTuple(hv_Box[0]) == HTuple("false")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("box");
                hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
            }
            else if (0 != (HTuple(hv_Box[0]) != HTuple("true")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("box_color");
                hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[0]));
            }
        }
        if (0 != ((hv_Box.TupleLength()) > 1))
        {
            if (0 != (HTuple(hv_Box[1]) == HTuple("false")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("shadow");
                hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
            }
            else if (0 != (HTuple(hv_Box[1]) != HTuple("true")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("shadow_color");
                hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[1]));
            }
        }
        if (0 != (hv_Color == HTuple("")))
        {
            hv_Color = HTuple();
        }
        hv_ContinueMessage = "Press Run (F5) to continue";
        DispText(hv_WindowHandle, hv_ContinueMessage, "window", "bottom", "right", hv_Color,
                 hv_GenParamName, hv_GenParamValue);
    }
    
    // ========== set_display_font ==========
    void set_display_font(HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font, HTuple hv_Bold,
                          HTuple hv_Slant)
    {
        HTuple hv_OS, hv_Fonts, hv_Style, hv_Exception;
        HTuple hv_AvailableFonts, hv_Fdx, hv_Indices;
        GetSystem("operating_system", &hv_OS);
        if (0 != (HTuple(hv_Size == HTuple()).TupleOr(hv_Size == -1)))
        {
            hv_Size = 16;
        }
        if (0 != ((hv_OS.TupleSubstr(0, 2)) == HTuple("Win")))
        {
            hv_Size = (1.13677 * hv_Size).TupleInt();
        }
        if (0 != (hv_Font == HTuple("Courier")))
        {
            hv_Fonts.Clear();
            hv_Fonts[0] = "Courier";
            hv_Fonts[1] = "Courier 10 Pitch";
            hv_Fonts[2] = "Courier New";
            hv_Fonts[3] = "CourierNew";
        }
        else if (0 != (hv_Font == HTuple("mono")))
        {
            hv_Fonts.Clear();
            hv_Fonts[0] = "Consolas";
            hv_Fonts[1] = "Menlo";
            hv_Fonts[2] = "Courier";
            hv_Fonts[3] = "Courier 10 Pitch";
            hv_Fonts[4] = "FreeMono";
        }
        else if (0 != (hv_Font == HTuple("sans")))
        {
            hv_Fonts.Clear();
            hv_Fonts[0] = "Luxi Sans";
            hv_Fonts[1] = "DejaVu Sans";
            hv_Fonts[2] = "FreeSans";
            hv_Fonts[3] = "Arial";
        }
        else if (0 != (hv_Font == HTuple("serif")))
        {
            hv_Fonts.Clear();
            hv_Fonts[0] = "Times New Roman";
            hv_Fonts[1] = "Luxi Serif";
            hv_Fonts[2] = "DejaVu Serif";
            hv_Fonts[3] = "FreeSerif";
            hv_Fonts[4] = "Utopia";
        }
        else
        {
            hv_Fonts = hv_Font;
        }
        hv_Style = "";
        if (0 != (hv_Bold == HTuple("true")))
        {
            hv_Style += HTuple("Bold");
        }
        else if (0 != (hv_Bold != HTuple("false")))
        {
            hv_Exception = "Wrong value of control parameter Bold";
            throw HException(hv_Exception);
        }
        if (0 != (hv_Slant == HTuple("true")))
        {
            hv_Style += HTuple("Italic");
        }
        else if (0 != (hv_Slant != HTuple("false")))
        {
            hv_Exception = "Wrong value of control parameter Slant";
            throw HException(hv_Exception);
        }
        if (0 != (hv_Style == HTuple("")))
        {
            hv_Style = "Normal";
        }
        QueryFont(hv_WindowHandle, &hv_AvailableFonts);
        hv_Font = "";
        {
            HTuple end_val48 = (hv_Fonts.TupleLength()) - 1;
            HTuple step_val48 = 1;
            for (hv_Fdx = 0; hv_Fdx.Continue(end_val48, step_val48); hv_Fdx += step_val48)
            {
                hv_Indices = hv_AvailableFonts.TupleFind(HTuple(hv_Fonts[hv_Fdx]));
                if (0 != ((hv_Indices.TupleLength()) > 0))
                {
                    if (0 != (HTuple(hv_Indices[0]) >= 0))
                    {
                        hv_Font = HTuple(hv_Fonts[hv_Fdx]);
                        break;
                    }
                }
            }
        }
        if (0 != (hv_Font == HTuple("")))
        {
            throw HException("Wrong value of control parameter Font");
        }
        hv_Font = (((hv_Font + "-") + hv_Style) + "-") + hv_Size;
        SetFont(hv_WindowHandle, hv_Font);
    }
    
    // ========== disp_message ==========
    void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
                      HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box)
    {
        HTuple hv_GenParamName, hv_GenParamValue;
        if (0 != (HTuple(hv_Row == HTuple()).TupleOr(hv_Column == HTuple())))
        {
            return;
        }
        if (0 != (hv_Row == -1))
        {
            hv_Row = 12;
        }
        if (0 != (hv_Column == -1))
        {
            hv_Column = 12;
        }
        hv_GenParamName = HTuple();
        hv_GenParamValue = HTuple();
        if (0 != ((hv_Box.TupleLength()) > 0))
        {
            if (0 != (HTuple(hv_Box[0]) == HTuple("false")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("box");
                hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
            }
            else if (0 != (HTuple(hv_Box[0]) != HTuple("true")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("box_color");
                hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[0]));
            }
        }
        if (0 != ((hv_Box.TupleLength()) > 1))
        {
            if (0 != (HTuple(hv_Box[1]) == HTuple("false")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("shadow");
                hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
            }
            else if (0 != (HTuple(hv_Box[1]) != HTuple("true")))
            {
                hv_GenParamName = hv_GenParamName.TupleConcat("shadow_color");
                hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[1]));
            }
        }
        if (0 != (hv_CoordSystem != HTuple("window")))
        {
            hv_CoordSystem = "image";
        }
        if (0 != (hv_Color == HTuple("")))
        {
            hv_Color = HTuple();
        }
        DispText(hv_WindowHandle, hv_String, hv_CoordSystem, hv_Row, hv_Column, hv_Color,
                 hv_GenParamName, hv_GenParamValue);
    }
    
    // ========== CHalconImageTool 真实实现 ==========
    int CHalconImageTool::ReadImageToWindow(const std::string& strPath, HTuple& hv_WindowHandle,
                                             HObject& ho_Image, HTuple& hv_Width, HTuple& hv_Height,
                                             int winX, int winY)
    {
        try {
            HTuple hv_Pointer, hv_Type;
            ReadImage(&ho_Image, strPath.c_str());
            GetImagePointer1(ho_Image, &hv_Pointer, &hv_Type, &hv_Width, &hv_Height);
            SetWindowAttr("background_color", "black");
            OpenWindow(winX, winY, hv_Width, hv_Height, 0, "visible", "", &hv_WindowHandle);
            HDevWindowStack::Push(hv_WindowHandle);
            if (HDevWindowStack::IsOpen())
                DispObj(ho_Image, HDevWindowStack::GetActive());
            return 0;
        }
        catch (HException& e) {
            CLogService::GetInstance().Error("ReadImageToWindow异常: " + std::string(e.ErrorMessage()));
            return -1;
        }
    }
    
    int CHalconImageTool::ReadImageFile(const std::string& imgPath, HObject& outImage, SImageInfo& outInfo)
    {
        try {
            HTuple hv_Width, hv_Height, hv_Pointer, hv_Type;
            ReadImage(&outImage, imgPath.c_str());
            GetImagePointer1(outImage, &hv_Pointer, &hv_Type, &hv_Width, &hv_Height);
            outInfo.width = hv_Width;
            outInfo.height = hv_Height;
            outInfo.channel = 1;
            outInfo.path = imgPath;
            outInfo.isValid = true;
            return 0;
        }
        catch (HException& e) {
            CLogService::GetInstance().Error("ReadImageFile异常: " + std::string(e.ErrorMessage()));
            outInfo.isValid = false;
            return -1;
        }
    }
    
    int CHalconImageTool::CreateImageWindow(const HObject& image, HTuple& outWinHandle,
                                             int offsetX, int offsetY)
    {
        try {
            HTuple hv_Width, hv_Height;
            GetImageSize(image, &hv_Width, &hv_Height);
            SetWindowAttr("background_color", "black");
            OpenWindow(offsetX, offsetY, hv_Width, hv_Height, 0, "visible", "", &outWinHandle);
            HDevWindowStack::Push(outWinHandle);
            if (HDevWindowStack::IsOpen())
                DispObj(image, HDevWindowStack::GetActive());
            return 0;
        }
        catch (HException& e) {
            CLogService::GetInstance().Error("CreateImageWindow异常: " + std::string(e.ErrorMessage()));
            return -1;
        }
    }
    
    void CHalconImageTool::ClearAllWindow()
    {
        while (HDevWindowStack::IsOpen())
        {
            CloseWindow(HDevWindowStack::Pop());
        }
    }
    
    int CHalconImageTool::SaveImageFile(const HObject& image, const std::string& savePath)
    {
        try {
            WriteImage(image, "png", 0, savePath.c_str());
            return 0;
        }
        catch (HException& e) {
            CLogService::GetInstance().Error("SaveImageFile异常: " + std::string(e.ErrorMessage()));
            return -1;
        }
    }
    
    int CHalconImageTool::GetImageInfo(const HObject& image, SImageInfo& outInfo)
    {
        try {
            HTuple hv_Width, hv_Height;
            GetImageSize(image, &hv_Width, &hv_Height);
            outInfo.width = hv_Width;
            outInfo.height = hv_Height;
            outInfo.channel = 1;
            outInfo.isValid = true;
            return 0;
        }
        catch (HException& e) {
            CLogService::GetInstance().Error("GetImageInfo异常: " + std::string(e.ErrorMessage()));
            outInfo.isValid = false;
            return -1;
        }
    }
    
    HObject CHalconImageTool::CreateMockImage(int width, int height, int channel)
    {
        (void)width; (void)height; (void)channel;
        HObject empty;
        return empty;
    }
    
    // ========== CResourceMgr 真实实现 ==========
    CResourceMgr& CResourceMgr::GetInstance() {
        static CResourceMgr instance;
        return instance;
    }
    
    void CResourceMgr::RegisterImage(HObject* pImage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_images.push_back(pImage);
        m_imageCount++;
    }
    
    void CResourceMgr::UnregisterImage(HObject* pImage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find(m_images.begin(), m_images.end(), pImage);
        if (it != m_images.end()) {
            if (*it) ClearObj(**it);
            m_images.erase(it);
            m_imageCount--;
        }
    }
    
    void CResourceMgr::ClearAllImages() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto pImage : m_images) {
            if (pImage) ClearObj(*pImage);
        }
        m_images.clear();
        m_imageCount = 0;
    }
    
    void CResourceMgr::RegisterWindow(HTuple* pWindow) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_windows.push_back(pWindow);
        m_windowCount++;
    }
    
    void CResourceMgr::UnregisterWindow(HTuple* pWindow) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find(m_windows.begin(), m_windows.end(), pWindow);
        if (it != m_windows.end()) {
            m_windows.erase(it);
            m_windowCount--;
        }
    }
    
    void CResourceMgr::ClearAllWindows() {
        std::lock_guard<std::mutex> lock(m_mutex);
        CHalconImageTool::ClearAllWindow();
        m_windows.clear();
        m_windowCount = 0;
    }
    
    void CResourceMgr::RegisterModel(void* pModel) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_models.push_back(pModel);
        m_modelCount++;
    }
    
    void CResourceMgr::UnregisterModel(void* pModel) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find(m_models.begin(), m_models.end(), pModel);
        if (it != m_models.end()) {
            m_models.erase(it);
            m_modelCount--;
        }
    }
    
    void CResourceMgr::ClearAllModels() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_models.clear();
        m_modelCount = 0;
    }
    
    void CResourceMgr::CleanupAll() {
        ClearAllImages();
        ClearAllWindows();
        ClearAllModels();
    }
    
    size_t CResourceMgr::GetImageCount() const { return m_imageCount; }
    size_t CResourceMgr::GetWindowCount() const { return m_windowCount; }
    size_t CResourceMgr::GetModelCount() const { return m_modelCount; }

#else  // !USE_HALCON (打桩模式 - 完整实现)
    
    // ============================================================
    // 打桩模式：模拟实现（所有绘图函数输出Mock日志）
    // ============================================================
    
    // HObject 静态成员初始化
    int HObject::s_nextId = 0;
    
    // ========== 绘图函数（打桩实现） ==========
    void disp_continue_message(HTuple hv_WindowHandle, HTuple hv_Color, HTuple hv_Box)
    {
        (void)hv_WindowHandle; (void)hv_Color; (void)hv_Box;
        std::cout << "[MOCK] disp_continue_message called" << std::endl;
    }
    
    void set_display_font(HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font,
                          HTuple hv_Bold, HTuple hv_Slant)
    {
        (void)hv_WindowHandle; (void)hv_Size; (void)hv_Font;
        (void)hv_Bold; (void)hv_Slant;
        std::cout << "[MOCK] set_display_font called" << std::endl;
    }
    
    void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
                      HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box)
    {
        (void)hv_WindowHandle; (void)hv_String; (void)hv_CoordSystem;
        (void)hv_Row; (void)hv_Column; (void)hv_Color; (void)hv_Box;
        std::cout << "[MOCK] disp_message called" << std::endl;
    }
    
    // ========== CHalconImageTool 打桩实现 ==========
    int CHalconImageTool::ReadImageToWindow(const std::string& strPath, HTuple& hv_WindowHandle,
                                             HObject& ho_Image, HTuple& hv_Width, HTuple& hv_Height,
                                             int winX, int winY)
    {
        (void)strPath; (void)winX; (void)winY;
        hv_WindowHandle = 1;
        hv_Width = 640;
        hv_Height = 480;
        ho_Image = HObject();
        
        CLogService::GetInstance().Info("[MOCK] ReadImageToWindow: " + strPath);
        std::cout << "[MOCK] 读取图像: " << strPath << " (模拟模式)" << std::endl;
        return 0;
    }
    
    int CHalconImageTool::ReadImageFile(const std::string& imgPath, HObject& outImage, SImageInfo& outInfo)
    {
        outImage = HObject();
        outInfo.width = 640;
        outInfo.height = 480;
        outInfo.channel = 3;
        outInfo.path = imgPath;
        outInfo.isValid = true;
        CLogService::GetInstance().Info("[MOCK] ReadImageFile: " + imgPath);
        return 0;
    }
    
    int CHalconImageTool::CreateImageWindow(const HObject& image, HTuple& outWinHandle,
                                             int offsetX, int offsetY)
    {
        (void)image; (void)offsetX; (void)offsetY;
        outWinHandle = 1;
        CLogService::GetInstance().Info("[MOCK] CreateImageWindow");
        return 0;
    }
    
    void CHalconImageTool::ClearAllWindow()
    {
        CLogService::GetInstance().Info("[MOCK] ClearAllWindow");
    }
    
    int CHalconImageTool::SaveImageFile(const HObject& image, const std::string& savePath)
    {
        (void)image;
        CLogService::GetInstance().Info("[MOCK] SaveImageFile: " + savePath);
        std::cout << "[MOCK] 保存图像: " << savePath << " (模拟模式)" << std::endl;
        return 0;
    }
    
    int CHalconImageTool::GetImageInfo(const HObject& image, SImageInfo& outInfo)
    {
        (void)image;
        outInfo.width = 640;
        outInfo.height = 480;
        outInfo.channel = 3;
        outInfo.isValid = true;
        return 0;
    }
    
    HObject CHalconImageTool::CreateMockImage(int width, int height, int channel)
    {
        (void)width; (void)height; (void)channel;
        CLogService::GetInstance().Info("[MOCK] CreateMockImage: " + 
            std::to_string(width) + "x" + std::to_string(height));
        return HObject();
    }
    
    // ========== CResourceMgr 打桩实现 ==========
    CResourceMgr& CResourceMgr::GetInstance() {
        static CResourceMgr instance;
        return instance;
    }
    
    void CResourceMgr::RegisterImage(HObject* pImage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_images.push_back(reinterpret_cast<void*>(pImage));
        m_imageCount++;
        CLogService::GetInstance().Debug("[MOCK] RegisterImage: count=" + std::to_string(m_imageCount));
    }
    
    void CResourceMgr::UnregisterImage(HObject* pImage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        void* ptr = reinterpret_cast<void*>(pImage);
        auto it = std::find(m_images.begin(), m_images.end(), ptr);
        if (it != m_images.end()) {
            m_images.erase(it);
            m_imageCount--;
        }
        CLogService::GetInstance().Debug("[MOCK] UnregisterImage: count=" + std::to_string(m_imageCount));
    }
    
    void CResourceMgr::ClearAllImages() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_images.clear();
        m_imageCount = 0;
        CLogService::GetInstance().Info("[MOCK] ClearAllImages");
    }
    
    void CResourceMgr::RegisterWindow(HTuple* pWindow) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_windows.push_back(reinterpret_cast<void*>(pWindow));
        m_windowCount++;
    }
    
    void CResourceMgr::UnregisterWindow(HTuple* pWindow) {
        std::lock_guard<std::mutex> lock(m_mutex);
        void* ptr = reinterpret_cast<void*>(pWindow);
        auto it = std::find(m_windows.begin(), m_windows.end(), ptr);
        if (it != m_windows.end()) {
            m_windows.erase(it);
            m_windowCount--;
        }
    }
    
    void CResourceMgr::ClearAllWindows() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_windows.clear();
        m_windowCount = 0;
        CLogService::GetInstance().Info("[MOCK] ClearAllWindows");
    }
    
    void CResourceMgr::RegisterModel(void* pModel) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_models.push_back(pModel);
        m_modelCount++;
    }
    
    void CResourceMgr::UnregisterModel(void* pModel) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find(m_models.begin(), m_models.end(), pModel);
        if (it != m_models.end()) {
            m_models.erase(it);
            m_modelCount--;
        }
    }
    
    void CResourceMgr::ClearAllModels() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_models.clear();
        m_modelCount = 0;
    }
    
    void CResourceMgr::CleanupAll() {
        ClearAllImages();
        ClearAllWindows();
        ClearAllModels();
    }
    
    size_t CResourceMgr::GetImageCount() const { return m_imageCount; }
    size_t CResourceMgr::GetWindowCount() const { return m_windowCount; }
    size_t CResourceMgr::GetModelCount() const { return m_modelCount; }
    
    // ============================================================
    // 打桩模式专用：模拟数据生成器
    // ============================================================
    
    SOCRResult CMockDataGenerator::GenerateMockOCRResult(int charCount)
    {
        SOCRResult result;
        result.charCount = charCount;
        result.isValid = true;
        result.totalTimeMs = 125.6;
        
        const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for (int i = 0; i < charCount && i < 36; i++) {
            result.text.push_back(std::string(1, chars[i]));
            result.row.push_back(100.0 + i * 20.0);
            result.col.push_back(50.0 + i * 15.0);
            result.confidence.push_back(0.85 + (rand() % 15) / 100.0);
        }
        return result;
    }
    
    SDefectResult CMockDataGenerator::GenerateMockDefectResult(int defectCount)
    {
        SDefectResult result;
        result.defectNum = defectCount;
        result.isValid = true;
        result.totalTimeMs = 89.3;
        
        const char* types[] = {"scratch", "spot", "dent", "crack", "missing"};
        for (int i = 0; i < defectCount; i++) {
            result.area.push_back(100.0 + (rand() % 500));
            result.row.push_back(200.0 + (rand() % 200));
            result.col.push_back(300.0 + (rand() % 200));
            result.type.push_back(types[i % 5]);
        }
        return result;
    }
    
    SMatchResult CMockDataGenerator::GenerateMockMatchResult(int matchCount)
    {
        SMatchResult result;
        result.matchNum = matchCount;
        result.isValid = true;
        result.totalTimeMs = 45.2;
        
        for (int i = 0; i < matchCount; i++) {
            result.row.push_back(150.0 + i * 80.0);
            result.col.push_back(200.0 + i * 60.0);
            result.angle.push_back((rand() % 360) - 180);
            result.score.push_back(0.75 + (rand() % 25) / 100.0);
            result.scale.push_back(0.9 + (rand() % 20) / 100.0);
        }
        return result;
    }
    
    SStruct3DResult CMockDataGenerator::GenerateMock3DResult(int width, int height)
    {
        SStruct3DResult result;
        result.width = width;
        result.height = height;
        result.isValid = true;
        result.totalTimeMs = 345.8;
        result.maxHeight = 45.6;
        result.minHeight = 2.3;
        result.planeParams = {0.0, 0.0, 10.0};
        return result;
    }

#endif // USE_HALCON
