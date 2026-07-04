#include "OcrBottleAlgo.h"
#include "../core/LogService.h"
#include <chrono>

#ifdef USE_HALCON

using namespace std::chrono;

int COcrBottleAlgo::RunBottleOCR(const std::string& imgPath,
                                   const std::string& fontName,
                                   SOCRResult& outResult)
{
    auto startTime = high_resolution_clock::now();
    HObject ho_Bottle;
    HTuple hv_Width, hv_Height, hv_WindowID;
    HTuple hv_TextModel, hv_TextResultID, hv_Area;
    HTuple hv_Classes, hv_Row, hv_Column;
    HObject ho_Characters;

    try {
        ReadImage(&ho_Bottle, imgPath.c_str());
        GetImageSize(ho_Bottle, &hv_Width, &hv_Height);

        CHalconImageTool::ClearAllWindow();
        SetWindowAttr("background_color", "black");
        OpenWindow(0, 0, 2 * hv_Width, 2 * hv_Height, 0, "visible", "", &hv_WindowID);
        HDevWindowStack::Push(hv_WindowID);
        set_display_font(hv_WindowID, 16, "mono", "true", "false");

        if (HDevWindowStack::IsOpen())
            DispObj(ho_Bottle, HDevWindowStack::GetActive());
        disp_continue_message(hv_WindowID, "black", "true");

        CreateTextModelReader("auto", fontName.c_str(), &hv_TextModel);
        SetTextModelParam(hv_TextModel, "min_stroke_width", 6);
        SetTextModelParam(hv_TextModel, "text_line_structure", "2 2 2");

        FindText(ho_Bottle, hv_TextModel, &hv_TextResultID);

        GetTextObject(&ho_Characters, hv_TextResultID, "all_lines");
        if (HDevWindowStack::IsOpen())
            DispObj(ho_Bottle, HDevWindowStack::GetActive());
        if (HDevWindowStack::IsOpen())
            DispObj(ho_Characters, HDevWindowStack::GetActive());

        GetTextResult(hv_TextResultID, "class", &hv_Classes);
        AreaCenter(ho_Characters, &hv_Area, &hv_Row, &hv_Column);
        disp_message(hv_WindowID, hv_Classes, "image", 80, hv_Column - 3, "green", "false");

        outResult.text.clear();
        outResult.row.clear();
        outResult.col.clear();
        outResult.confidence.clear();

        for (int i = 0; i < hv_Classes.Length(); i++) {
            outResult.text.push_back(hv_Classes[i].S().Text());
            outResult.row.push_back(hv_Row[i].D());
            outResult.col.push_back(hv_Column[i].D());
            outResult.confidence.push_back(1.0);
        }
        outResult.charRegion = ho_Characters;
        outResult.windowHandle = hv_WindowID;
        outResult.charCount = hv_Classes.Length();
        outResult.isValid = (hv_Classes.Length() > 0);

        ClearTextResult(hv_TextResultID);
        ClearTextModel(hv_TextModel);

        auto endTime = high_resolution_clock::now();
        outResult.totalTimeMs = duration_cast<microseconds>(endTime - startTime).count() / 1000.0;

        return 0;
    }
    catch (HException& e) {
        outResult.isValid = false;
        return -1;
    }
}

int COcrBottleAlgo::BatchOCR(const std::vector<std::string>& imgPaths,
                              const std::string& fontName,
                              std::vector<SOCRResult>& batchRes)
{
    batchRes.clear();
    batchRes.reserve(imgPaths.size());

    for (const auto& path : imgPaths) {
        SOCRResult result;
        if (RunBottleOCR(path, fontName, result) == 0) {
            batchRes.push_back(result);
        }
    }

    return (int)batchRes.size();
}

#endif // USE_HALCON
