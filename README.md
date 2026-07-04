# VisionForge - 工业检测机器视觉代码工程

[![License](https://img.shields.io/badge/License-Commercial%20Proprietary-red.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.2+-green.svg)](https://qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.20+-brightgreen.svg)](https://cmake.org/)
[![Python](https://img.shields.io/badge/Python-3.7+-yellow.svg)](https://python.org/)

**版本**: 2.4 | **更新日期**: 2026-07-03

---

## 📖 项目简介

本项目面向工业检测领域的机器视觉工程化落地，涵盖 **3D结构光检测**、**形状匹配**、**结构件测量**、**全场景视觉检测** 及 **Halcon原型验证** 五大核心场景，提供从代码生成、测试数据准备、编译调试到 Git 仓库管理的全链路解决方案。

### 核心设计原则

| 原则 | 说明 | 实现方式 |
|------|------|----------|
| **轻量化** | 核心算法不依赖 OpenCV | C++17标准库 + STB图像库 + 自研算法 |
| **模块化解耦** | 各模块独立封装 | 标定、检测、应用三大模块通过 JSON 接口通信 |
| **实时性保障** | 满足工业现场实时要求 | 算法精简 + OpenMP并行加速 + SIMD优化 |
| **跨平台支持** | Windows/Linux统一构建 | Qt6 + CMake + vcpkg依赖管理 |
| **Git原生集成** | 自动化版本管理 | 自动初始化Git仓库，支持 GitHub/Gitee 双远程 |

---

## 🏗️ 系统架构
```
┌─────────────────────────────────────────────────────────────────┐
│ 应用层 (Application Layer) │
│ ┌───────────┬───────────┬───────────┬───────────┬───────────┐ │
│ │ 3D结构光 │ 形状匹配 │ 结构测量 │ 全场景 │ Halcon │ │
│ │ 检测系统 │ 系统 │ 系统 │ 检测系统 │ 原型系统 │ │
│ └───────────┴───────────┴───────────┴───────────┴───────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ 业务逻辑层 (Business Logic) │
│ ┌───────────┬───────────┬───────────┬───────────┬───────────┐ │
│ │ 标定模块 │ 检测模块 │ 测量模块 │ 分析模块 │ Halcon │ │
│ │ │ │ │ │ 封装模块 │ │
│ └───────────┴───────────┴───────────┴───────────┴───────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ 算法层 (Algorithm Layer) │
│ ┌───────────┬───────────┬───────────┬───────────┬───────────┐ │
│ │ 图像处理 │ 特征提取 │ 3D重建 │ 缺陷检测 │ Halcon │ │
│ │ │ │ │ │ 算子 │ │
│ └───────────┴───────────┴───────────┴───────────┴───────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ 基础设施层 (Infrastructure) │
│ ┌───────────┬───────────┬───────────┬───────────┬───────────┐ │
│ │ STB Image │ OpenMP │ Qt6 GUI │ Halcon │ mock_ │ │
│ │ 图像IO │ 并行计算 │ 界面框架 │ SDK(可选) │ halcon │ │
│ └───────────┴───────────┴───────────┴───────────┴───────────┘ │
└─────────────────────────────────────────────────────────────────┘
``` 

---

## 📁 工程列表

| 代号 | 工程名 | 类别 | 描述 |
|:-----|--------|:-----|:-----|
| `halcon` | **HalconVision** | 原型验证 | Halcon算法原型验证与封装工程 |
| `3d` | **Struct3DInspect** | 3D检测 | LCOS结构光3D工业检测系统 |
| `shape` | **ShapeMatch** | 2D检测 | 跨平台梯度形状匹配工具 |
| `measure` | **StructMeasure** | 尺寸测量 | 工业结构件智能测量系统 |
| `vision` | **VisionInspect** | 全场景 | 工业视觉全场景检测系统 |

---

## 🚀 快速开始

### 环境要求

- **编译器**: GCC 9+ / Clang 10+ / MSVC 2019+ （支持C++17）
- **Qt**: 6.2 或更高（Core, Gui, Widgets, OpenGL, OpenGLWidgets）
- **CMake**: 3.20+
- **Python**: 3.7+ （用于自动化脚本）
- **Halcon**: 可选（仅 HalconVision 工程需要）
- **OpenCV**: 可选（用于对比测试）

### 生成工程代码

```bash
# 进入项目目录
cd visionforge

# 生成所有工程代码
./generate_all.sh

# 生成指定工程代码
./generate_all.sh --project Struct3DInspect
```

### 编译工程
```bash
# 编译所有工程
./build_all.sh

# 清理后重新编译
./build_all.sh --clean

# 仅编译指定工程
./build_all.sh --project Struct3DInspect
```

---

### 运行工程
```bash
# 每个命令独立执行，均从项目根目录开始
cd projects/HalconVision && ./run.sh
cd projects/Struct3DInspect && ./run.sh
cd projects/ShapeMatch && ./run.sh
cd projects/StructMeasure && ./run.sh
cd projects/VisionInspect && ./run.sh
```

---

## 📊 性能指标
| 指标 | 目标值 | 测量方法 |
|:-----|:------:|:---------|
| 标定时间 | ≤2秒 | 从加载图像到生成标定参数 |
| 单帧检测时间 | ≤2秒 | 从输入图像到输出检测结果 |
| 全流程时间 | ≤3秒 | 包含图像采集、预处理、检测、输出 |
| 内存占用 | ≤1GB | 峰值内存使用量 |
| 检测精度 | ≥99.5% | 标准测试集上的准确率 |

---

## 📁 目录结构
> **说明**：`build/` 目录为 CMake 构建输出目录（包含编译中间文件和可执行文件），属于自动生成目录，**不建议手动修改**；`src/` 为源代码目录，是工程的核心开发目录。
```
visionforge/                          # 项目根目录
├── build_all.sh                      # 一键编译所有工程
├── generate_all.sh                   # 一键生成所有工程代码
├── push_all.sh                       # 一键推送所有工程到远程仓库
├── LICENSE                           # 项目许可证
├── README.md                         # 项目总览（本文件）
├── mv_structure                      # 目录结构快照文件
├── mv_structure.txt                  # 目录结构文本备份
├── logs/                             # 构建日志目录
├── projects/                         # 生成的工程目录
│   ├── HalconVision/                 # Halcon原型验证工程
│   │   ├── assets/                   # 资源文件（棋盘格/OCR/条纹图案）
│   │   ├── build/                    # 构建输出目录
│   │   ├── CMakeLists.txt            # CMake构建配置
│   │   ├── config/                   # 配置文件
│   │   ├── deployment/               # 部署配置（Docker/TensorRT）
│   │   ├── LICENSE                   # 工程许可证
│   │   ├── logs/                     # 工程日志
│   │   ├── README.md                 # 工程说明
│   │   ├── run.sh                    # 运行脚本
│   │   ├── src/                      # 源代码
│   │   │   ├── algorithm/            # 算法模块（OCR等）
│   │   │   ├── app/                  # 应用入口（Console/MFC/Qt）
│   │   │   ├── capture/              # 图像采集
│   │   │   ├── core/                 # 核心工具（HalconTool/LogService）
│   │   │   └── io/                   # 输入输出
│   │   ├── test/                     # 测试代码
│   │   └── third_party/              # 第三方依赖（halcon_linux/halcon_win/mock_halcon）
│   ├── Struct3DInspect/              # 3D结构光检测工程
│   │   ├── assets/                   # 资源文件（图案/测试数据）
│   │   ├── build/                    # 构建输出目录
│   │   ├── CMakeLists.txt            # CMake构建配置
│   │   ├── LICENSE                   # 工程许可证
│   │   ├── README.md                 # 工程说明
│   │   ├── run.sh                    # 运行脚本
│   │   ├── src/                      # 源代码
│   │   │   ├── algorithm/            # 算法（GrayCode/MultiFreqUnwrap/PhaseShift）
│   │   │   ├── calib/                # 标定模块
│   │   │   ├── core/                 # 核心（ImageBase/ImageProc/PointCloudFilter/Triangulate）
│   │   │   ├── io/                   # 输入输出（CSVWriter/PointCloudIO）
│   │   │   └── ui/                   # 用户界面（main/MainWindow）
│   │   ├── test/                     # 测试代码
│   │   └── third_party/              # 第三方依赖（STB头文件）
│   ├── ShapeMatch/                   # 形状匹配工程
│   │   ├── assets/                   # 资源文件（生成图/模板图）
│   │   ├── build/                    # 构建输出目录
│   │   ├── CMakeLists.txt            # CMake构建配置
│   │   ├── run.sh                    # 运行脚本
│   │   ├── src/                      # 源代码
│   │   │   ├── algorithm/            # 算法（ShapeMatch）
│   │   │   ├── base/                 # 基础（image_base）
│   │   │   ├── ui/                   # 用户界面（main/MainWindow）
│   │   │   └── utils/                # 工具（cross_util）
│   │   └── third_party/              # 第三方依赖（STB头文件）
│   ├── StructMeasure/                # 结构件测量工程
│   │   ├── assets/                   # 资源文件
│   │   ├── build/                    # 构建输出目录
│   │   ├── CMakeLists.txt            # CMake构建配置
│   │   ├── LICENSE                   # 工程许可证
│   │   ├── README.md                 # 工程说明
│   │   ├── run.sh                    # 运行脚本
│   │   ├── src/                      # 源代码
│   │   │   ├── main.cpp              # 主入口
│   │   │   ├── MainWindow.cpp        # 主窗口实现
│   │   │   ├── MainWindow.h          # 主窗口头文件
│   │   │   ├── StructureMeasure.cpp  # 测量核心实现
│   │   │   └── StructureMeasure.h    # 测量核心头文件
│   │   └── third_party/              # 第三方依赖（STB头文件）
│   └── VisionInspect/                # 工业视觉全场景工程
│       ├── assets/                   # 资源文件
│       ├── build/                    # 构建输出目录
│       ├── CMakeLists.txt            # CMake构建配置
│       ├── LICENSE                   # 工程许可证
│       ├── README.md                 # 工程说明
│       ├── run.sh                    # 运行脚本
│       ├── src/                      # 源代码
│       │   ├── algorithm/            # 算法（DefectDetect/Detect3D/FeatureDetect/Measure2D）
│       │   ├── calibration/          # 标定（CameraCalib/HandEyeCalib/StereoCalib）
│       │   ├── core/                 # 核心（ImageBase/Matrix/Timer）
│       │   ├── io/                   # 输入输出（CSVWriter/JsonIO/PLYWriter）
│       │   └── ui/                   # 用户界面（main/MainWindow）
│       ├── test/                     # 测试代码
│       └── third_party/              # 第三方依赖（STB头文件）
├── scripts/                          # Python自动化脚本
│   ├── config.json                   # 全局配置
│   ├── config_loader.py              # 配置加载器
│   ├── gen_halcon_test.py            # Halcon测试数据生成
│   ├── gen_measure_test.py           # 测量测试数据生成
│   ├── gen_shape_test.py             # 形状匹配测试数据生成
│   ├── gen_slight3d_test.py          # 3D检测测试数据生成
│   └── gen_vision_test.py            # 视觉检测测试数据生成
├── testdata/                         # （未使用，保留占位）
└── third_party/                      # 全局第三方依赖（STB头文件）
    ├── stb_image.h
    ├── stb_image_resize2.h
    └── stb_image_write.h
```

---

## 🔧 脚本工具链
| 脚本 | 用途 |
|:-----|:-----|
| `build_all.sh` | 一键编译所有工程 |
| `generate_all.sh` | 一键生成所有工程代码 |
| `push_all.sh` | 一键推送所有工程到远程仓库 |
| `scripts/gen_halcon_test.py` | 生成 Halcon 测试数据 |
| `scripts/gen_measure_test.py` | 生成结构测量测试数据 |
| `scripts/gen_shape_test.py` | 生成形状匹配测试数据 |
| `scripts/gen_slight3d_test.py` | 生成3D检测测试数据 |
| `scripts/gen_vision_test.py` | 生成视觉检测测试数据 |

---

## 📝 各工程详细说明
1. HalconVision - Halcon 原型验证系统
基于 Halcon SDK 的算法原型验证与封装工程。

核心算法: Halcon算子封装 → OCR识别 → 棋盘格标定 → 模板匹配

主要输出: OCR识别结果、标定参数、匹配结果
**目录结构**: `src/algorithm/`（OCR/棋盘格/条纹算法）、`src/app/`（Console/MFC/Qt入口）、`src/core/`（HalconTool/LogService）、`src/io/`（图像IO）

2. Struct3DInspect - 3D结构光检测系统
基于 LCOS-LED 结构光投影，实现格雷码+多步相移三维重建。

核心算法: 格雷码解码 → 多步相移 → 多频外差 → 三角测量重建

主要输出: 深度图、PLY点云、CSV尺寸报表

目录结构: src/algorithm/（核心算法）、src/calib/（标定）、src/core/（图像基础/点云滤波/三角测量）、src/io/（CSV/点云IO）、src/ui/（界面）

3. ShapeMatch - 形状匹配系统
基于梯度金字塔的形状匹配，支持旋转和尺度不变性。

核心算法: 梯度金字塔 → 余弦相似度匹配 → RANSAC精定位

主要输出: 匹配坐标、角度、尺度、匹配分数

目录结构: src/algorithm/（ShapeMatch核心）、src/base/（图像基础）、src/utils/（跨平台工具）

4. StructMeasure - 结构件测量系统
基于RANSAC拟合的结构件几何尺寸测量。

核心算法: Otsu阈值 → Canny边缘 → RANSAC分段拟合 → 锥角计算

主要输出: 锥角、尺寸、对称度、CSV报表

5. VisionInspect - 工业视觉全场景系统
集成标定、2D缺陷检测、3D检测的综合性视觉系统。

核心算法: 相机标定 → 2D缺陷检测 → 尺寸测量 → 3D检测

主要输出: 缺陷报表、尺寸数据、点云

目录结构: src/algorithm/（检测算法）、src/calibration/（标定）、src/core/（矩阵/计时器）、src/io/（JSON/PLY/CSV）

---

## 🤝 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

---

## 📄 许可证

本工程为商业专有软件，未经授权不得复制、分发或用于商业目的。详见 LICENSE。


---

## 👤 作者

**Yongji Gao**

- Email: bonaventure@163.com

---

## 🙏 致谢

- [STB](https://github.com/nothings/stb) - 单头文件图像库
- [Qt](https://qt.io/) - 跨平台应用框架
- [Halcon](https://www.mvtec.com/) - 工业视觉SDK（可选）
- [OpenCV](https://opencv.org/) - 开源计算机视觉库（可选）

⭐ 如果这个项目对您有帮助，请给一个 Star！
