#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <iostream>

// ============================================================
// 条件编译：根据 USE_HALCON 决定是否包含 Halcon 头文件
// ============================================================
#ifdef USE_HALCON
    #include <HalconCpp.h>
    using namespace HalconCpp;
#else
    // ============================================================
    // 打桩模式：完整 HTuple Mock（支持常用Halcon操作）
    // ============================================================
    #include <cstdint>
    #include <exception>
    #include <cmath>
    
    // HTuple 完整 Mock
    class HTuple {
    public:
        // 构造函数
        HTuple() : m_type(TYPE_NULL), m_intVal(0), m_doubleVal(0.0) {}
        HTuple(int val) : m_type(TYPE_INT), m_intVal(val), m_doubleVal(0.0) {}
        HTuple(double val) : m_type(TYPE_DOUBLE), m_intVal(0), m_doubleVal(val) {}
        HTuple(const char* str) : m_type(TYPE_STRING), m_strVal(str), m_intVal(0), m_doubleVal(0.0) {}
        HTuple(const std::string& str) : m_type(TYPE_STRING), m_strVal(str), m_intVal(0), m_doubleVal(0.0) {}
        
        // 拷贝构造
        HTuple(const HTuple& other) {
            m_type = other.m_type;
            m_intVal = other.m_intVal;
            m_doubleVal = other.m_doubleVal;
            m_strVal = other.m_strVal;
            m_array = other.m_array;
            m_arrayType = other.m_arrayType;
        }
        
        // 赋值
        HTuple& operator=(const HTuple& other) {
            if (this != &other) {
                m_type = other.m_type;
                m_intVal = other.m_intVal;
                m_doubleVal = other.m_doubleVal;
                m_strVal = other.m_strVal;
                m_array = other.m_array;
                m_arrayType = other.m_arrayType;
            }
            return *this;
        }
        HTuple& operator=(int val) { m_type = TYPE_INT; m_intVal = val; return *this; }
        HTuple& operator=(const char* str) { m_type = TYPE_STRING; m_strVal = str; return *this; }
        
        // 比较操作符
        bool operator==(const HTuple& other) const {
            if (m_type != other.m_type) return false;
            if (m_type == TYPE_INT) return m_intVal == other.m_intVal;
            if (m_type == TYPE_DOUBLE) return fabs(m_doubleVal - other.m_doubleVal) < 1e-9;
            if (m_type == TYPE_STRING) return m_strVal == other.m_strVal;
            return false;
        }
        bool operator!=(const HTuple& other) const { return !(*this == other); }
        bool operator==(int val) const { return m_type == TYPE_INT && m_intVal == val; }
        bool operator!=(int val) const { return !(*this == val); }
        
        // 数组索引
        HTuple operator[](int index) const {
            if (index >= 0 && index < (int)m_array.size()) {
                return m_array[index];
            }
            return HTuple();
        }
        
        // ========== Halcon HTuple 核心方法 ==========
        int Length() const {
            if (m_type == TYPE_STRING) return (int)m_strVal.length();
            if (m_array.size() > 0) return (int)m_array.size();
            return 1;
        }
        int Num() const { return Length(); }
        int TupleLength() const { return Length(); }
        
        std::string S() const {
            if (m_type == TYPE_STRING) return m_strVal;
            if (m_type == TYPE_INT) return std::to_string(m_intVal);
            if (m_type == TYPE_DOUBLE) return std::to_string(m_doubleVal);
            return "";
        }
        std::string ToString() const { return S(); }
        
        double D() const {
            if (m_type == TYPE_INT) return (double)m_intVal;
            if (m_type == TYPE_DOUBLE) return m_doubleVal;
            return 0.0;
        }
        int I() const {
            if (m_type == TYPE_INT) return m_intVal;
            if (m_type == TYPE_DOUBLE) return (int)m_doubleVal;
            return 0;
        }
        
        HTuple TupleOr(const HTuple& other) const {
            if (m_type == TYPE_INT && other.m_type == TYPE_INT) {
                return HTuple(m_intVal || other.m_intVal ? 1 : 0);
            }
            return HTuple(0);
        }
        HTuple TupleOr(int val) const {
            if (m_type == TYPE_INT) {
                return HTuple(m_intVal || val ? 1 : 0);
            }
            return HTuple(0);
        }
        
        HTuple TupleSubstr(int start, int end) const {
            if (m_type == TYPE_STRING) {
                std::string s = m_strVal;
                if (start < 0) start = 0;
                if (end >= (int)s.length()) end = (int)s.length() - 1;
                if (start <= end && start < (int)s.length()) {
                    return HTuple(s.substr(start, end - start + 1));
                }
            }
            return HTuple("");
        }
        
        HTuple TupleInt() const {
            if (m_type == TYPE_DOUBLE) return HTuple((int)m_doubleVal);
            if (m_type == TYPE_INT) return HTuple(m_intVal);
            return HTuple(0);
        }
        
        void Clear() {
            m_type = TYPE_NULL;
            m_intVal = 0;
            m_doubleVal = 0.0;
            m_strVal = "";
            m_array.clear();
            m_arrayType = TYPE_NULL;
        }
        
        HTuple TupleConcat(const HTuple& other) const {
            HTuple result = *this;
            if (other.m_type == TYPE_STRING && m_type == TYPE_STRING) {
                result.m_strVal = m_strVal + other.m_strVal;
            } else {
                result.m_array.push_back(other);
                if (result.m_arrayType == TYPE_NULL) {
                    result.m_arrayType = other.m_type;
                }
            }
            return result;
        }
        HTuple TupleConcat(const char* str) const {
            return TupleConcat(HTuple(str));
        }
        
        HTuple TupleFind(const HTuple& needle) const {
            HTuple result;
            for (size_t i = 0; i < m_array.size(); i++) {
                if (m_array[i] == needle) {
                    return HTuple((int)i);
                }
            }
            if (m_type == needle.m_type) {
                if (m_type == TYPE_STRING && m_strVal == needle.m_strVal) return HTuple(0);
                if (m_type == TYPE_INT && m_intVal == needle.m_intVal) return HTuple(0);
            }
            return HTuple(-1);
        }
        
        bool Continue(const HTuple& end, const HTuple& step) const {
            int current = m_intVal;
            int endVal = end.m_intVal;
            int stepVal = step.m_intVal;
            if (stepVal > 0) return current <= endVal;
            if (stepVal < 0) return current >= endVal;
            return true;
        }
        
        // 运算符重载
        HTuple operator+(const HTuple& other) const {
            if (m_type == TYPE_INT && other.m_type == TYPE_INT) {
                return HTuple(m_intVal + other.m_intVal);
            }
            if (m_type == TYPE_INT && other.m_type == TYPE_DOUBLE) {
                return HTuple(m_intVal + other.m_doubleVal);
            }
            if (m_type == TYPE_DOUBLE && other.m_type == TYPE_INT) {
                return HTuple(m_doubleVal + other.m_intVal);
            }
            if (m_type == TYPE_DOUBLE && other.m_type == TYPE_DOUBLE) {
                return HTuple(m_doubleVal + other.m_doubleVal);
            }
            if (m_type == TYPE_STRING || other.m_type == TYPE_STRING) {
                std::string result = S();
                result += other.S();
                return HTuple(result);
            }
            return HTuple(0);
        }
        
        HTuple operator-(const HTuple& other) const {
            if (m_type == TYPE_INT && other.m_type == TYPE_INT) {
                return HTuple(m_intVal - other.m_intVal);
            }
            if (m_type == TYPE_INT && other.m_type == TYPE_DOUBLE) {
                return HTuple(m_intVal - other.m_doubleVal);
            }
            if (m_type == TYPE_DOUBLE && other.m_type == TYPE_INT) {
                return HTuple(m_doubleVal - other.m_intVal);
            }
            if (m_type == TYPE_DOUBLE && other.m_type == TYPE_DOUBLE) {
                return HTuple(m_doubleVal - other.m_doubleVal);
            }
            return HTuple(0);
        }
        
        HTuple& operator+=(const HTuple& other) {
            *this = *this + other;
            return *this;
        }

    private:
        enum Type { TYPE_NULL, TYPE_INT, TYPE_DOUBLE, TYPE_STRING };
        Type m_type;
        int m_intVal;
        double m_doubleVal;
        std::string m_strVal;
        std::vector<HTuple> m_array;
        Type m_arrayType;
    };
    
    // HObject Mock
    class HObject {
    public:
        HObject() : m_id(++s_nextId) {}
        HObject(const HObject& other) : m_id(++s_nextId) { (void)other; }
        HObject& operator=(const HObject& other) { (void)other; return *this; }
        ~HObject() {}
        bool IsValid() const { return true; }
        int GetId() const { return m_id; }
    private:
        static int s_nextId;
        int m_id;
    };
    
    // HException Mock
    class HException : public std::exception {
    public:
        HException() : m_msg("") {}
        HException(const std::string& msg) : m_msg(msg) {}
        HException(const char* msg) : m_msg(msg) {}
        const char* ErrorMessage() const { return m_msg.c_str(); }
        const char* what() const noexcept override { return m_msg.c_str(); }
    private:
        std::string m_msg;
    };
    
    // HalconCpp 命名空间
    namespace HalconCpp {
        typedef ::HObject HObject;
        typedef ::HTuple HTuple;
        typedef ::HException HException;
    }
    
    // HDevWindowStack Mock
    class HDevWindowStack {
    public:
        static void Push(HTuple) {}
        static bool IsOpen() { return false; }
        static HTuple GetActive() { return HTuple(); }
        static HTuple Pop() { return HTuple(); }
    };
    
    // 宏定义：将 Halcon 算子替换为空操作（打桩）
    #define ReadImage(a, b)              ((void)0)
    #define GetImageSize(a, b, c)        ((void)0)
    #define GetImagePointer1(a, b, c, d, e) ((void)0)
    #define OpenWindow(a, b, c, d, e, f, g, h) ((void)0)
    #define CloseWindow(a)               ((void)0)
    #define DispObj(a, b)                ((void)0)
    #define SetWindowAttr(a, b)          ((void)0)
    #define CreateTextModelReader(a, b, c) ((void)0)
    #define SetTextModelParam(a, b, c)   ((void)0)
    #define FindText(a, b, c)            ((void)0)
    #define GetTextObject(a, b, c)       ((void)0)
    #define GetTextResult(a, b, c)       ((void)0)
    #define AreaCenter(a, b, c, d)       ((void)0)
    #define ClearTextResult(a)           ((void)0)
    #define ClearTextModel(a)            ((void)0)
    #define ClearObj(a)                  ((void)0)
    #define WriteImage(a, b, c, d)       ((void)0)
    #define GetSystem(a, b)              ((void)0)
    #define QueryFont(a, b)              ((void)0)
    #define SetFont(a, b)                ((void)0)
    #define DispText(a, b, c, d, e, f, g, h) ((void)0)
    
#endif // USE_HALCON

// ============================================================
// 统一数据结构（两种模式共用）
// ============================================================

// 图像信息结构体
struct SImageInfo {
#ifdef USE_HALCON
    HTuple width;
    HTuple height;
    HTuple channel;
#else
    int width;
    int height;
    int channel;
#endif
    std::string path;
    std::string timestamp;
    bool isValid;
    
    SImageInfo() : width(0), height(0), channel(0), isValid(false) {}
};

// OCR 识别结果
struct SOCRResult {
    std::vector<std::string> text;
    std::vector<double> row;
    std::vector<double> col;
    std::vector<double> confidence;
    std::vector<int> charIndex;
    
#ifdef USE_HALCON
    HObject charRegion;
    HTuple windowHandle;
#else
    void* charRegion;
    void* windowHandle;
#endif
    
    double totalTimeMs;
    bool isValid;
    int charCount;
    
    SOCRResult() 
        : charRegion(nullptr), windowHandle(nullptr)
        , totalTimeMs(0), isValid(false), charCount(0) {}
};

// 缺陷检测结果
struct SDefectResult {
    int defectNum;
    std::vector<double> area;
    std::vector<double> row;
    std::vector<double> col;
    std::vector<std::string> type;
    
#ifdef USE_HALCON
    HObject defectRegion;
#else
    void* defectRegion;
#endif
    
    double totalTimeMs;
    bool isValid;
    
    SDefectResult() : defectRegion(nullptr), totalTimeMs(0), isValid(false) {}
};

// 模板匹配结果
struct SMatchResult {
    int matchNum;
    std::vector<double> row;
    std::vector<double> col;
    std::vector<double> angle;
    std::vector<double> score;
    std::vector<double> scale;
    double totalTimeMs;
    bool isValid;
    
    SMatchResult() : matchNum(0), totalTimeMs(0), isValid(false) {}
};

// 3D结构光重建结果
struct SStruct3DResult {
#ifdef USE_HALCON
    HObject depthMap;
    HObject pointCloud;
    HTuple width, height;
#else
    void* depthMap;
    void* pointCloud;
    int width, height;
#endif
    std::vector<double> planeParams;
    double maxHeight, minHeight;
    double totalTimeMs;
    bool isValid;
    
    SStruct3DResult() 
        : depthMap(nullptr), pointCloud(nullptr)
        , width(0), height(0), maxHeight(0), minHeight(0)
        , totalTimeMs(0), isValid(false) {}
};

// 尺寸测量结果
struct SMeasureResult {
    std::vector<double> distances;
    std::vector<double> angles;
    std::vector<double> diameters;
    std::vector<double> pointsX, pointsY;
    double totalTimeMs;
    bool isValid;
    
    SMeasureResult() : totalTimeMs(0), isValid(false) {}
};

// 算法配置
struct SAlgorithmConfig {
    std::string ocr_font_name = "Universal_0-9_NoRej";
    int ocr_min_stroke_width = 6;
    std::string ocr_text_line_structure = "2 2 2";
    double ocr_confidence_threshold = 0.5;
    
    int stripe_gray_bits = 8;
    int stripe_shift_steps = 4;
    int stripe_frequency = 3;
    
    double defect_min_area = 10.0;
    double defect_max_area = 10000.0;
    double defect_confidence = 0.5;
    
    double match_min_score = 0.3;
    int match_num_matches = 5;
    
    int enable_log_debug = 0;
    int enable_cuda = 1;
    int enable_tensorrt = 0;
};

// 打印结果辅助函数
inline void PrintOCRResult(const SOCRResult& result) {
    std::cout << "=== OCR 识别结果 ===" << std::endl;
    std::cout << "字符数: " << result.charCount << std::endl;
    std::cout << "识别文字: ";
    for (const auto& t : result.text) {
        std::cout << t;
    }
    std::cout << std::endl;
    std::cout << "耗时: " << result.totalTimeMs << " ms" << std::endl;
    std::cout << "有效: " << (result.isValid ? "是" : "否") << std::endl;
}
