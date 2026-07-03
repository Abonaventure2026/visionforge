#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPainter>
#include <QApplication>
#include <QTabWidget>
#include <iostream>
#include <fstream>
#include "../algorithm/Measure2D.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    InitUI();
    setWindowTitle("工业视觉全场景检测系统 v2.4");
    resize(1400, 900);
}

QString MainWindow::GetCalibFilePath() {
    return "./calibration.json";
}

void MainWindow::InitUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    tabWidget = new QTabWidget(this);

    QWidget* calibTab = new QWidget();
    QVBoxLayout* calibLayout = new QVBoxLayout(calibTab);
    QHBoxLayout* calibParamLayout = new QHBoxLayout();
    calibParamLayout->addWidget(new QLabel("标定帧数:"));
    spinCalibFrames = new QSpinBox();
    spinCalibFrames->setRange(5, 30);
    spinCalibFrames->setValue(15);
    calibParamLayout->addWidget(spinCalibFrames);
    calibParamLayout->addStretch();
    calibLayout->addLayout(calibParamLayout);
    tabWidget->addTab(calibTab, "标定");

    QWidget* detectTab = new QWidget();
    QVBoxLayout* detectLayout = new QVBoxLayout(detectTab);
    QHBoxLayout* paramLayout = new QHBoxLayout();
    paramLayout->addWidget(new QLabel("基线(mm):"));
    spinBaseline = new QDoubleSpinBox();
    spinBaseline->setRange(10, 500);
    spinBaseline->setValue(120);
    paramLayout->addWidget(spinBaseline);
    paramLayout->addWidget(new QLabel("焦距(px):"));
    spinFocal = new QDoubleSpinBox();
    spinFocal->setRange(500, 5000);
    spinFocal->setValue(1850);
    paramLayout->addWidget(spinFocal);
    paramLayout->addStretch();
    detectLayout->addLayout(paramLayout);
    tabWidget->addTab(detectTab, "检测");

    mainLayout->addWidget(tabWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLoad = new QPushButton("加载图像");
    btnLoadMultiple = new QPushButton("批量加载");
    btnCalibrate = new QPushButton("标定");
    btnSaveCalib = new QPushButton("保存标定");
    btnLoadCalib = new QPushButton("加载标定");
    btnDetect = new QPushButton("执行检测");
    btnSave = new QPushButton("保存结果");
    btnExport = new QPushButton("导出CSV");

    btnLoadMultiple->setStyleSheet("QPushButton { background: #4CAF50; color: white; font-weight: bold; }");
    btnSaveCalib->setStyleSheet("QPushButton { background: #2196F3; color: white; }");
    btnLoadCalib->setStyleSheet("QPushButton { background: #FF9800; color: white; }");

    for (auto b : {btnLoad, btnLoadMultiple, btnCalibrate, btnSaveCalib, btnLoadCalib, btnDetect, btnSave, btnExport})
        btnLayout->addWidget(b);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    labImage = new QLabel("原始图像");
    labResult = new QLabel("检测结果");
    for (auto lab : {labImage, labResult}) {
        lab->setMinimumSize(400, 300);
        lab->setAlignment(Qt::AlignCenter);
        lab->setStyleSheet("border:1px solid #aaa; background:#f8f8f8;");
        splitter->addWidget(lab);
    }
    mainLayout->addWidget(splitter, 1);

    txtReport = new QTextEdit();
    txtReport->setReadOnly(true);
    txtReport->setMaximumHeight(100);
    mainLayout->addWidget(txtReport);

    // 信号连接
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::OnLoadImage);
    connect(btnLoadMultiple, &QPushButton::clicked, this, &MainWindow::OnLoadMultipleImages);
    connect(btnCalibrate, &QPushButton::clicked, this, &MainWindow::OnCalibrate);
    connect(btnSaveCalib, &QPushButton::clicked, this, &MainWindow::OnSaveCalib);
    connect(btnLoadCalib, &QPushButton::clicked, this, &MainWindow::OnLoadCalib);
    connect(btnDetect, &QPushButton::clicked, this, &MainWindow::OnDetect);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::OnSaveResult);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::OnExportCSV);
}

void MainWindow::OnLoadImage() {
    QString path = QFileDialog::getOpenFileName(this, "选择图像", "./assets",
                                                "Images (*.png *.jpg *.bmp)");
    if (path.isEmpty()) return;
    Image img;
    if (!img.LoadFromFile(path.toStdString())) {
        QMessageBox::warning(this, "错误", "无法加载图像");
        return;
    }
    current_image = img;
    labImage->setPixmap(ImageToPix(img).scaled(labImage->size(), Qt::KeepAspectRatio));
    UpdateStatus("已加载图像: " + QFileInfo(path).fileName());
}

void MainWindow::OnLoadMultipleImages() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "选择多张标定图像",
        "./assets",
        "Images (*.png *.jpg *.bmp)"
    );

    if (files.isEmpty()) return;

    std::vector<Image> loaded_images;
    for (const QString& path : files) {
        Image img;
        if (img.LoadFromFile(path.toStdString())) {
            loaded_images.push_back(img);
        }
    }

    if (loaded_images.empty()) {
        QMessageBox::warning(this, "错误", "没有有效的图像被加载");
        return;
    }

    UpdateStatus(QString("已加载 %1 张图像，准备批量标定...").arg(loaded_images.size()));
    BatchCalibrate(loaded_images);
}

void MainWindow::BatchCalibrate(const std::vector<Image>& images) {
    if (images.size() < 5) {
        QMessageBox::warning(this, "提示",
            QString("图像数量不足（%1张），标定至少需要5张").arg(images.size()));
        return;
    }

    UpdateStatus(QString("🔍 正在标定 %1 张图像，请稍候...").arg(images.size()));
    QApplication::processEvents();

    calib_images = images;

    TargetParam target;
    target.target_type = "chessboard";
    target.inner_corners_cols = 9;
    target.inner_corners_rows = 7;
    target.square_size_mm = 10.0;

    calib.Init(target, images[0].width, images[0].height);
    calib_result = calib.Calibrate(calib_images);

    if (calib_result.valid) {
        is_calibrated = true;
        UpdateStatus(QString("✅ 标定完成！使用 %1 张图像，重投影误差: %2 像素，耗时: %3 ms")
                     .arg(images.size())
                     .arg(calib_result.reproj_error, 0, 'f', 3)
                     .arg(calib_result.duration_ms, 0, 'f', 1));
        QMessageBox::information(this, "标定成功",
            QString("标定完成！\n"
                    "使用图像数: %1 张\n"
                    "重投影误差: %2 像素\n"
                    "耗时: %3 ms\n\n"
                    "点击「保存标定」可保存参数供后续使用")
            .arg(images.size())
            .arg(calib_result.reproj_error, 0, 'f', 3)
            .arg(calib_result.duration_ms, 0, 'f', 1));
    } else {
        UpdateStatus("❌ 标定失败，请检查标靶图像");
        QMessageBox::warning(this, "标定失败",
            QString("标定失败，请检查 %1 张图像中是否有清晰可见的棋盘格").arg(images.size()));
    }

    calib_images.clear();
}

void MainWindow::OnCalibrate() {
    if (!current_image.IsValid()) {
        QMessageBox::warning(this, "提示", "请先加载一张图像！");
        return;
    }

    calib_images.push_back(current_image);
    int current = (int)calib_images.size();
    int total = spinCalibFrames->value();

    UpdateStatus(QString("✅ 已采集 %1/%2 张标定图像").arg(current).arg(total));

    if (current < total) {
        QMessageBox::information(this, "标定采集",
            QString("已采集第 %1 张，还需要 %2 张\n\n"
                    "方式一：继续加载下一张图像后点击「标定」\n"
                    "方式二：点击「批量加载」一次性加载多张")
            .arg(current).arg(total - current));
        return;
    }

    UpdateStatus("🔍 正在标定，请稍候...");
    QApplication::processEvents();

    TargetParam target;
    target.target_type = "chessboard";
    target.inner_corners_cols = 9;
    target.inner_corners_rows = 7;
    target.square_size_mm = 10.0;

    calib.Init(target, current_image.width, current_image.height);
    calib_result = calib.Calibrate(calib_images);

    if (calib_result.valid) {
        is_calibrated = true;
        UpdateStatus(QString("✅ 标定完成！重投影误差: %1 像素，耗时: %2 ms")
                     .arg(calib_result.reproj_error, 0, 'f', 3)
                     .arg(calib_result.duration_ms, 0, 'f', 1));
        QMessageBox::information(this, "标定成功",
            QString("标定完成！\n"
                    "重投影误差: %1 像素\n"
                    "耗时: %2 ms\n\n"
                    "点击「保存标定」可保存参数供后续使用")
            .arg(calib_result.reproj_error, 0, 'f', 3)
            .arg(calib_result.duration_ms, 0, 'f', 1));
    } else {
        UpdateStatus("❌ 标定失败，请检查标靶图像");
        QMessageBox::warning(this, "标定失败", "标定失败，请检查标靶图像是否清晰可见");
    }

    calib_images.clear();
}

void MainWindow::OnSaveCalib() {
    if (!calib_result.valid) {
        QMessageBox::warning(this, "提示", "没有有效的标定参数可保存，请先执行标定");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this, "保存标定参数", GetCalibFilePath(), "JSON (*.json)");
    if (path.isEmpty()) return;

    if (calib.SaveParam(calib_result, path.toStdString())) {
        QMessageBox::information(this, "成功", QString("标定参数已保存到:\n%1").arg(path));
        UpdateStatus("✅ 标定参数已保存");
    } else {
        QMessageBox::warning(this, "错误", "保存标定参数失败");
    }
}

void MainWindow::OnLoadCalib() {
    QString path = QFileDialog::getOpenFileName(this, "加载标定参数", "./", "JSON (*.json)");
    if (path.isEmpty()) return;

    CalibParam loaded = calib.LoadParam(path.toStdString());
    if (loaded.valid) {
        calib_result = loaded;
        is_calibrated = true;
        calib_images.clear();
        UpdateStatus(QString("✅ 标定参数已加载: %1").arg(QFileInfo(path).fileName()));
        QMessageBox::information(this, "成功",
            QString("标定参数已加载\n"
                    "重投影误差: %1 像素\n"
                    "图像尺寸: %2 x %3")
            .arg(calib_result.reproj_error, 0, 'f', 3)
            .arg(calib_result.width)
            .arg(calib_result.height));
    } else {
        QMessageBox::warning(this, "错误", "加载标定参数失败，文件格式无效");
    }
}

void MainWindow::DrawDefectBoxes(Image& img, const std::vector<DefectInfo>& defects) {
    if (!img.IsValid() || defects.empty()) return;

    for (const auto& defect : defects) {
        int x = std::max(0, defect.pixel_x);
        int y = std::max(0, defect.pixel_y);
        int w = std::min(defect.pixel_width, img.width - x);
        int h = std::min(defect.pixel_height, img.height - y);

        for (int i = y; i < y + h && i < img.height; ++i) {
            for (int j = x; j < x + w && j < img.width; ++j) {
                if (i == y || i == y + h - 1 || j == x || j == x + w - 1) {
                    int idx = (i * img.width + j) * img.channels;
                    if (idx + 2 < img.width * img.height * img.channels) {
                        img.data[idx] = 255;
                        img.data[idx+1] = 0;
                        img.data[idx+2] = 0;
                    }
                }
            }
        }
    }
}

void MainWindow::OnDetect() {
    if (!current_image.IsValid()) {
        QMessageBox::warning(this, "提示", "请先加载图像");
        return;
    }
    if (!is_calibrated) {
        QMessageBox::warning(this, "提示", "请先完成标定或加载标定参数");
        return;
    }

    std::cout << "[DEBUG] 开始检测..." << std::endl;

    UpdateStatus("检测中...");
    QApplication::processEvents();

    // ===== 1. 图像预处理（灰度化 + 二值化） =====
    Image gray = current_image.ToGray();
    Image blurred(gray.width, gray.height, 1);
    GaussianBlur3x3(gray, blurred);
    Image binary(gray.width, gray.height, 1);
    OtsuThreshold(blurred, binary);

    // ===== 2. 缺陷检测（使用灰度图像） =====
    DetectParam param;
    param.min_defect_area = 10.0;
    detector.Init(calib_result, param);
    detect_result = detector.Detect(current_image);

    // ===== 3. 尺寸测量（使用二值图像） =====
    measure_result = Measure2D::Measure(binary);
    std::cout << "[DEBUG] 尺寸测量: 宽=" << measure_result.width
              << ", 高=" << measure_result.height
              << ", 面积=" << measure_result.area << std::endl;

    // ===== 4. 绘制检测结果图像 =====
    Image result_img = current_image.Clone();

    // 4a. 绘制缺陷框（红色）
    if (detect_result.valid && !detect_result.defects.empty()) {
        DrawDefectBoxes(result_img, detect_result.defects);
    }

    labResult->setPixmap(ImageToPix(result_img).scaled(labResult->size(),
                         Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // ===== 5. 生成完整报告 =====
    QString report = "========== 工业视觉检测报告 ==========\n";
    report += QString("帧号: %1\n").arg(detect_result.frame_id);
    report += "------------------------------------\n";
    report += "【缺陷检测结果】\n";
    if (detect_result.valid && !detect_result.defects.empty()) {
        report += QString("  缺陷数量: %1\n").arg(detect_result.defects.size());
        for (size_t i = 0; i < detect_result.defects.size(); ++i) {
            auto& d = detect_result.defects[i];
            report += QString("  缺陷 %1: %2 @ (%3,%4) [%5x%6]\n")
                      .arg(i+1)
                      .arg(QString::fromStdString(d.type))
                      .arg(d.pixel_x).arg(d.pixel_y)
                      .arg(d.pixel_width).arg(d.pixel_height);
        }
    } else {
        report += "  未检测到缺陷\n";
    }
    report += "------------------------------------\n";
    report += "【尺寸测量结果】\n";
    if (measure_result.valid) {
        report += QString("  宽度: %1 px\n").arg(measure_result.width, 0, 'f', 1);
        report += QString("  高度: %1 px\n").arg(measure_result.height, 0, 'f', 1);
        report += QString("  面积: %1 px²\n").arg(measure_result.area, 0, 'f', 0);
        report += QString("  周长: %1 px\n").arg(measure_result.perimeter, 0, 'f', 1);
    } else {
        report += "  尺寸测量失败\n";
    }
    report += "------------------------------------\n";
    report += QString("检测耗时: %1 ms\n").arg(detect_result.detect_duration_ms, 0, 'f', 2);
    report += "====================================\n";
    UpdateReport(report);

    UpdateStatus(QString("✅ 检测完成 | 缺陷:%1 | 尺寸:%2x%3")
                 .arg(detect_result.defects.size())
                 .arg(measure_result.width, 0, 'f', 0)
                 .arg(measure_result.height, 0, 'f', 0));

    QMessageBox::information(this, "检测结果",
        QString("✅ 检测完成！\n"
                "缺陷数量: %1\n"
                "工件尺寸: %2 x %3 px\n"
                "面积: %4 px²\n\n"
                "详细信息请查看结果面板")
        .arg(detect_result.defects.size())
        .arg(measure_result.width, 0, 'f', 0)
        .arg(measure_result.height, 0, 'f', 0)
        .arg(measure_result.area, 0, 'f', 0));
}

void MainWindow::OnSaveResult() {
    QString path = QFileDialog::getSaveFileName(this, "保存结果", "./result.png", "PNG (*.png)");
    if (path.isEmpty()) return;
    if (current_image.SaveToFile(path.toStdString())) {
        QMessageBox::information(this, "成功", "结果已保存");
    }
}

void MainWindow::OnExportCSV() {
    QString path = QFileDialog::getSaveFileName(this, "导出CSV", "./report.csv", "CSV (*.csv)");
    if (path.isEmpty()) return;

    std::ofstream fout(path.toStdString());
    if (!fout) {
        QMessageBox::warning(this, "错误", "无法创建CSV文件");
        return;
    }

    // 导出缺陷检测结果
    fout << "=== 缺陷检测结果 ===\n";
    fout << "帧号,缺陷类型,位置X,位置Y,宽度,高度\n";
    for (const auto& d : detect_result.defects) {
        fout << detect_result.frame_id << ","
             << d.type << ","
             << d.pixel_x << ","
             << d.pixel_y << ","
             << d.pixel_width << ","
             << d.pixel_height << "\n";
    }

    // 导出尺寸测量结果
    fout << "\n=== 尺寸测量结果 ===\n";
    fout << "宽度(px),高度(px),面积(px²),周长(px)\n";
    fout << measure_result.width << "," << measure_result.height << ","
         << measure_result.area << "," << measure_result.perimeter << "\n";

    fout.close();

    QMessageBox::information(this, "成功", "CSV已导出");
}

QPixmap MainWindow::ImageToPix(const Image& img) {
    if (!img.IsValid()) return QPixmap();
    QImage qimg(img.width, img.height, QImage::Format_RGB888);
    if (img.channels == 1) {
        for (int i = 0; i < img.height; ++i) {
            for (int j = 0; j < img.width; ++j) {
                uint8_t v = img.data[i * img.width + j];
                qimg.setPixel(j, i, qRgb(v, v, v));
            }
        }
    } else if (img.channels >= 3) {
        memcpy(qimg.bits(), img.data, img.width * img.height * 3);
    }
    return QPixmap::fromImage(qimg);
}

void MainWindow::UpdateStatus(const QString& msg) {
    setWindowTitle("工业视觉全场景检测系统 - " + msg);
}

void MainWindow::UpdateReport(const QString& text) {
    txtReport->setPlainText(text);
    txtReport->repaint();
    QApplication::processEvents();
}
