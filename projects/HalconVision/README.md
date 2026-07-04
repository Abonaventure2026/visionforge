
# HalconVision – 跨平台Halcon机器视觉通用工程

[![License](https://img.shields.io/badge/License-Commercial%20Proprietary-red.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.2+-green.svg)](https://qt.io/)
[![Halcon](https://img.shields.io/badge/Halcon-20.11+-orange.svg)](https://www.mvtec.com/)

**HalconVision** 是一款 **学术研究+工业量产一体化** 的跨平台Halcon视觉解决方案，
完整整合历史代码资产、学术前沿算法与工业量产规范，适配半导体、消费电子、新能源等精密制造场景。

---

## 核心特性

### 📦 信息无损整合
- ✅ 初代MFC全套工程代码完整迁移
- ✅ 瓶身OCR整套算子流程无损保留
- ✅ disp_message/set_display_font等绘图函数完整复用
- ✅ 3D结构光仿真生成脚本全部功能迁移
- ✅ 全套自动化Shell/Python流水线脚本无缝适配

### 🔬 学术前沿算法（2025-2026）
- **3D结构光重建升级**：多频外差三频解包算法，精度≤30μm
- **双分支OCR架构**：传统TextModel + 深度学习CNN双模式
- **亚像素尺寸测量**：Canny亚像素+RANSAC鲁棒拟合
- **智能模板匹配**：HU不变矩+光照自适应均衡
- **深度学习缺陷检测**：Halcon DL框架+小样本迁移学习

### 🏭 工业量产标准化（7×24h）
- **资源自动管理**：图像/窗口/模型全生命周期释放
- **多线程解耦**：UI/采集/算法线程分离，界面不卡顿
- **分级日志追溯**：DEBUG/INFO/WARN/ERROR四级日志
- **JSON配置驱动**：无需修改代码即可调试参数
- **三模式编译**：Qt UI / MFC UI / 纯控制台

---

## 技术架构
```text
┌─────────────────────────────────────────────────────────────────────────────┐
│ 应用层 │
│ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────────────┐ │
│ │ Qt UI │ │ MFC UI │ │ Console │ │
│ │ (跨平台主力) │ │ (Windows兼容) │ │ (无界面批量推理) │ │
│ └─────────────────┘ └─────────────────┘ └─────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────────────┤
│ 算法业务层 │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ │
│ │ OCR模块 │ │ 3D重建 │ │ 缺陷检测 │ │模板匹配 │ │尺寸测量 │ │
│ └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘ │
│ ┌─────────────────────────────────────────────────────────────────────┐ │
│ │ 深度学习模块 (Halcon DL) │ │
│ └─────────────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────────────┤
│ 核心工具层 │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ │
│ │HalconTool│ │ResourceMgr│ │LogService│ │JsonConfig│ │
│ └──────────┘ └──────────┘ └──────────┘ └──────────┘ │
├─────────────────────────────────────────────────────────────────────────────┤
│ 硬件采集层 │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ │
│ │GigE相机 │ │虚拟相机 │ │LCOS投影 │ │
│ └──────────┘ └──────────┘ └──────────┘ │
└─────────────────────────────────────────────────────────────────────────────┘
```


---

## 目录结构
```text
HalconVision/
├── src/
│ ├── core/ # 核心工具（HalconTool, LogService, ResourceMgr）
│ ├── algorithm/ # 算法模块
│ │ ├── OcrModule/ # 瓶身OCR（传统+深度学习）
│ │ ├── Struct3D/ # 3D结构光重建
│ │ ├── TemplateMatch/ # 模板匹配
│ │ ├── Measure2D/ # 尺寸测量
│ │ ├── DefectDetect/ # 缺陷检测
│ │ └── deep_learning/ # 深度学习模块
│ ├── capture/ # 相机采集
│ ├── io/ # 数据持久化
│ └── app/ # 应用入口
│ ├── QtUI/ # Qt界面（跨平台）
│ ├── MfcUI/ # MFC界面（Windows兼容）
│ └── ConsoleMain.cpp # 控制台
├── config/ # JSON配置文件
│ └── vision_config.json
├── assets/ # 资源文件
│ ├── ocr_sample/ # OCR测试图片
│ ├── stripe_patterns/ # 结构光条纹图
│ └── template_lib/ # 模板库
├── third_party/ # 第三方依赖
│ ├── halcon_win/ # Windows Halcon库
│ └── halcon_linux/ # Linux Halcon库
├── deployment/ # 部署相关
│ ├── docker/ # Docker配置
│ └── tensorrt/ # TensorRT引擎
├── CMakeLists.txt
├── run.sh
└── README.md
```


---

## 依赖环境

| 组件 | 版本要求 | 说明 |
|------|----------|------|
| 操作系统 | Windows 10/11、Ubuntu 20.04/22.04 | 双平台支持 |
| C++编译器 | GCC 9+ / Clang 10+ / MSVC 2019+ | C++17标准 |
| Qt | 6.2+ | Qt界面开发（跨平台） |
| MFC | VS2022自带 | MFC界面（仅Windows兼容模式） |
| Halcon | 20.11 / 24 / 25 | 工业视觉核心 |
| CMake | 3.20+ | 跨平台构建 |
| CUDA | 11.7+ | GPU加速（推荐） |
| TensorRT | 8.5+ | 深度学习推理加速 |

---

## 编译与运行

### 编译模式说明

| 模式 | 平台 | 说明 |
|------|------|------|
| **Qt UI（默认）** | Windows/Linux/macOS | 跨平台Qt界面，推荐使用 |
| **MFC UI** | Windows ONLY | 兼容老版本工程，可选 |
| **Console** | 全部平台 | 无界面，批量推理专用 |

### Windows (VS2022)

```bash
# 1. 配置Halcon依赖
# 将Halcon Win64库放入 third_party/halcon_win/

# 2. 使用CMake生成VS工程（默认Qt界面）
mkdir build && cd build
cmake .. -DUSE_HALCON=ON -DBUILD_UI=ON -DBUILD_MFC=OFF

# 3. 编译
cmake --build . --config Release

# 4. 运行
./run.sh          # Qt界面
./run.sh --mfc    # MFC界面（需编译时启用）
```

### Linux (Ubuntu)

```bash
# 1. 安装依赖
sudo apt install cmake g++ qt6-base-dev libqt6opengl6-dev

# 2. 配置Halcon依赖
# 将Halcon Linux库放入 third_party/halcon_linux/
export HALCONROOT=$(pwd)/third_party/halcon_linux
export HALCONARCH=x64-linux

# 3. 编译
mkdir build && cd build
cmake .. -DBUILD_UI=ON -DUSE_HALCON=ON
make -j$(nproc)

# 4. 运行
./run.sh
```

### 三模式编译开关

| 开关 | 说明 | 默认 |
|------|------|------|
| `-DBUILD_UI=ON` | 编译Qt可视化界面 | ON |
| `-DBUILD_MFC=OFF` | 编译MFC备用界面(Windows) | OFF |
| `-DUSE_DL=OFF` | 启用深度学习模块 | OFF |
| `-DENABLE_TENSORRT=OFF` | 启用TensorRT加速 | OFF |
| `-DENABLE_CUDA=ON` | 启用CUDA加速 | ON |

## 使用流程
1. 图形界面模式 (Qt UI)
```bash
./run.sh
#加载图像 → 选择算法 → 执行检测 → 查看结果 → 导出报表
```
2. 控制台模式 (批量推理)
```bash
./run.sh --console --ocr ./assets/ocr_sample/bottle.png
./run.sh --console --batch ./assets/ocr_sample/
./run.sh --console --3d ./assets/stripe_patterns/
```
## 性能指标

| 指标 | 目标值 | 测量方法 |
|------|--------|----------|
| OCR识别 | ≤1.5s/帧 | 连续测试1000帧 |
| 3D重建 | ≤2s/帧 | 含全流程处理 |
| 缺陷检测 | ≤1.8s/帧 | 500万像素图像 |
| 资源释放 | 自动 | 7×24h无内存泄漏 |

## 扩展与定制

- **新增算法**：继承算法基类，实现检测接口
- **新增传感器**：实现相机采集接口
- **模型训练**：使用Halcon DL训练自定义模型
- **导出ONNX**：对接Python深度学习对比实验

## 许可证

本工程为商业专有软件，未经授权不得复制、分发或用于商业目的。详见 LICENSE。

## 作者

Yongji Gao
Email: bonaventure@163.com

## 相关资源

- [Qt 官方文档](https://doc.qt.io/)
- [MVTec Halcon 官方文档](https://www.mvtec.com/doc/)
- [工业3D AOI公开数据集](https://www.kaggle.com/datasets/ravirajsinh45/industrial-aoi-dataset)

---
*文档版本: 2.4 | 更新日期: 2026-07-04*
