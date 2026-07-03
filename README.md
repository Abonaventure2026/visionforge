# VisionForge - 工业检测机器视觉代码工程

[![License](https://img.shields.io/badge/License-Commercial%20Proprietary-red.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.2+-green.svg)](https://qt.io/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.5+-optional-blue.svg)](https://opencv.org/)
[![Python](https://img.shields.io/badge/Python-3.7+-yellow.svg)](https://python.org/)

**版本**: 2.4 | **更新日期**: 2026-07-03

---

## 📖 项目简介

本项目面向工业检测领域的机器视觉工程化落地，涵盖 **3D结构光检测**、**形状匹配**、**结构件测量**、**全场景视觉检测** 四大核心场景，提供从代码生成、测试数据准备、编译调试到 Git 仓库管理的全链路解决方案。

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
│ 应用层 (Application Layer)                                     │
│ ┌─────────────┬─────────────┬─────────────┬─────────────────┐ │
│ │ 3D结构光    │ 形状匹配    │ 结构测量    │ 全场景检测     │ │
│ │ 检测系统    │ 系统        │ 系统        │ 系统           │ │
│ └─────────────┴─────────────┴─────────────┴─────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ 业务逻辑层 (Business Logic)                                    │
│ ┌─────────────┬─────────────┬─────────────┬─────────────────┐ │
│ │ 标定模块    │ 检测模块    │ 测量模块    │ 分析模块       │ │
│ └─────────────┴─────────────┴─────────────┴─────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ 算法层 (Algorithm Layer)                                       │
│ ┌─────────────┬─────────────┬─────────────┬─────────────────┐ │
│ │ 图像处理    │ 特征提取    │ 3D重建     │ 缺陷检测       │ │
│ └─────────────┴─────────────┴─────────────┴─────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│ 基础设施层 (Infrastructure)                                    │
│ ┌─────────────┬─────────────┬─────────────┬─────────────────┐ │
│ │ STB Image   │ Eigen       │ OpenMP     │ Qt6 GUI        │ │
│ │ 图像IO      │ 数学库      │ 并行计算   │ 界面框架       │ │
│ └─────────────┴─────────────┴─────────────┴─────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📁 工程列表

| Key | 工程名 | 类别 | 描述 |
|-----|--------|------|------|
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
- **OpenCV**: 可选（用于对比测试）

### 编译工程

```bash
# 进入项目目录
cd visionforge

# 编译所有工程
./build_all.sh

# 清理后重新编译
./build_all.sh --clean

# 仅编译指定工程
./build_all.sh --project Struct3DInspect
```

### 运行工程

```bash
cd projects/Struct3DInspect && ./run.sh
cd projects/ShapeMatch && ./run.sh
cd projects/StructMeasure && ./run.sh
cd projects/VisionInspect && ./run.sh
```

## 📊 性能指标

| 指标 | 目标值 | 测量方法 |
|------|--------|----------|
| 标定时间 | ≤2秒 | 从加载图像到生成标定参数 |
| 单帧检测时间 | ≤2秒 | 从输入图像到输出检测结果 |
| 全流程时间 | ≤3秒 | 包含图像采集、预处理、检测、输出 |
| 内存占用 | ≤1GB | 峰值内存使用量 |
| 检测精度 | ≥99.5% | 标准测试集上的准确率 |


## 📁 目录结构

```
visionforge/
├── projects/                    # 生成的工程目录
│   ├── Struct3DInspect/         # 3D结构光检测工程
│   ├── ShapeMatch/              # 形状匹配工程
│   ├── StructMeasure/           # 结构件测量工程
│   └── VisionInspect/           # 工业视觉全场景工程
├── testdata/                    # 统一测试数据
├── third_party/                 # 第三方依赖（STB头文件）
├── build_all.sh                 # 一键编译所有工程
├── push_all.sh                  # 一键推送所有工程
├── LICENSE                      # 项目许可证
└── README.md                    # 项目总览
```

## 🔧 脚本工具链

| 脚本 | 用途 |
|------|------|
| `build_all.sh` | 一键编译所有工程 |
| `push_all.sh` | 一键推送所有工程到远程仓库 |


## 📝 各工程详细说明

### 1. Struct3DInspect - 3D结构光检测系统
基于 LCOS-LED 结构光投影，实现格雷码+多步相移三维重建。

**核心算法**: 格雷码解码 → 多步相移 → 多频外差 → 三角测量重建

**主要输出**: 深度图、PLY点云、CSV尺寸报表

### 2. ShapeMatch - 形状匹配系统
基于梯度金字塔的形状匹配，支持旋转和尺度不变性。

**核心算法**: 梯度金字塔 → 余弦相似度匹配 → RANSAC精定位

**主要输出**: 匹配坐标、角度、尺度、匹配分数

### 3. StructMeasure - 结构件测量系统
基于RANSAC拟合的结构件几何尺寸测量。

**核心算法**: Otsu阈值 → Canny边缘 → RANSAC分段拟合 → 锥角计算

**主要输出**: 锥角、尺寸、对称度、CSV报表

### 4. VisionInspect - 工业视觉全场景系统
集成标定、2D缺陷检测、3D检测的综合性视觉系统。

**核心算法**: 相机标定 → 2D缺陷检测 → 尺寸测量 → 3D检测

**主要输出**: 缺陷报表、尺寸数据、点云


## 🤝 贡献指南

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 📄 许可证

本工程为商业专有软件，未经授权不得复制、分发或用于商业目的。详见 LICENSE。

## 👤 作者

**Yongji Gao**

- Email: bonaventure@163.com

## 🙏 致谢

- [STB](https://github.com/nothings/stb) - 单头文件图像库
- [Qt](https://qt.io/) - 跨平台应用框架
- [OpenCV](https://opencv.org/) - 开源计算机视觉库（可选）

⭐ 如果这个项目对您有帮助，请给一个 Star！
