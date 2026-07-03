#include "MainWindow.h"
#include "../../third_party/stb_image.h"
#include <QImage>
#include <QFontDatabase>
#include <QProgressBar>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("ShapeMatchQt 专业形状匹配工具");
    resize(1600, 950);
    InitUI();
}

void MainWindow::InitUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLoadTemp = new QPushButton("📂 加载模板");
    btnLoadSrc  = new QPushButton("📂 加载待匹配");
    btnRunMatch = new QPushButton("▶ 执行匹配");
    btnSaveResult= new QPushButton("💾 保存结果");
    QString btnStyle = "QPushButton { background: #f0f0f0; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
                       "QPushButton:hover { background: #e0e0e0; }"
                       "QPushButton:pressed { background: #d0d0d0; }";
    btnLoadTemp->setStyleSheet(btnStyle);
    btnLoadSrc->setStyleSheet(btnStyle);
    btnSaveResult->setStyleSheet(btnStyle);
    btnRunMatch->setStyleSheet(btnStyle + "QPushButton { background: #4CAF50; color: white; }"
                                          "QPushButton:hover { background: #45a049; }"
                                          "QPushButton:disabled { background: #cccccc; color: #888888; }");
    btnLayout->addWidget(btnLoadTemp);
    btnLayout->addWidget(btnLoadSrc);
    btnLayout->addWidget(btnRunMatch);
    btnLayout->addWidget(btnSaveResult);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    QHBoxLayout *paramLayout = new QHBoxLayout();
    paramLayout->addWidget(new QLabel("预设:"));
    presetCombo = new QComboBox();
    presetCombo->addItems({"快速", "标准", "高精度"});
    presetCombo->setStyleSheet("QComboBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }");
    paramLayout->addWidget(presetCombo);

    paramLayout->addWidget(new QLabel("高阈值:"));
    sliderHighTh = new QSlider(Qt::Horizontal);
    sliderHighTh->setRange(0,255);
    sliderHighTh->setValue(120);
    sliderHighTh->setFixedWidth(100);
    spinHighTh = new QDoubleSpinBox();
    spinHighTh->setRange(0,255);
    spinHighTh->setValue(120);
    spinHighTh->setFixedWidth(50);
    connect(sliderHighTh, &QSlider::valueChanged, spinHighTh, &QDoubleSpinBox::setValue);
    connect(spinHighTh, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ sliderHighTh->setValue((int)v); });
    paramLayout->addWidget(sliderHighTh);
    paramLayout->addWidget(spinHighTh);

    paramLayout->addWidget(new QLabel("低阈值:"));
    sliderLowTh = new QSlider(Qt::Horizontal);
    sliderLowTh->setRange(0,100);
    sliderLowTh->setValue(10);
    sliderLowTh->setFixedWidth(100);
    spinLowTh = new QDoubleSpinBox();
    spinLowTh->setRange(0,100);
    spinLowTh->setValue(10);
    spinLowTh->setFixedWidth(50);
    connect(sliderLowTh, &QSlider::valueChanged, spinLowTh, &QDoubleSpinBox::setValue);
    connect(spinLowTh, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ sliderLowTh->setValue((int)v); });
    paramLayout->addWidget(sliderLowTh);
    paramLayout->addWidget(spinLowTh);

    paramLayout->addWidget(new QLabel("金字塔:"));
    sliderPyramid = new QSlider(Qt::Horizontal);
    sliderPyramid->setRange(1,6);
    sliderPyramid->setValue(1);
    sliderPyramid->setFixedWidth(80);
    spinPyramid = new QDoubleSpinBox();
    spinPyramid->setRange(1,6);
    spinPyramid->setValue(1);
    spinPyramid->setFixedWidth(40);
    connect(sliderPyramid, &QSlider::valueChanged, spinPyramid, &QDoubleSpinBox::setValue);
    connect(spinPyramid, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v){ sliderPyramid->setValue((int)v); });
    paramLayout->addWidget(sliderPyramid);
    paramLayout->addWidget(spinPyramid);

    paramLayout->addWidget(new QLabel("最低分数:"));
    spinMinScore = new QDoubleSpinBox();
    spinMinScore->setRange(-1,1);
    spinMinScore->setValue(0.2);
    spinMinScore->setSingleStep(0.05);
    spinMinScore->setFixedWidth(60);
    paramLayout->addWidget(spinMinScore);

    paramLayout->addWidget(new QLabel("贪心:"));
    spinGreed = new QDoubleSpinBox();
    spinGreed->setRange(0.1, 0.95);
    spinGreed->setSingleStep(0.05);
    spinGreed->setValue(0.9);
    spinGreed->setFixedWidth(60);
    paramLayout->addWidget(spinGreed);

    paramLayout->addStretch();
    mainLayout->addLayout(paramLayout);

    QHBoxLayout *advLayout = new QHBoxLayout();
    chkRotate = new QCheckBox("旋转搜索");
    chkRotate->setChecked(true);
    chkScale = new QCheckBox("尺度搜索");
    chkScale->setChecked(true);
    chkEnableMask = new QCheckBox("启用掩码过滤");
    chkEnableMask->setChecked(false);
    advLayout->addWidget(chkRotate);
    advLayout->addWidget(chkScale);
    advLayout->addWidget(chkEnableMask);
    advLayout->addWidget(new QLabel("旋转:"));
    spinRotStart = new QDoubleSpinBox(); spinRotStart->setRange(-180,180); spinRotStart->setValue(-30);
    spinRotEnd   = new QDoubleSpinBox(); spinRotEnd->setRange(-180,180); spinRotEnd->setValue(30);
    spinRotStep  = new QDoubleSpinBox(); spinRotStep->setRange(0.5,20); spinRotStep->setValue(5);
    advLayout->addWidget(spinRotStart);
    advLayout->addWidget(new QLabel("~"));
    advLayout->addWidget(spinRotEnd);
    advLayout->addWidget(new QLabel("步长"));
    advLayout->addWidget(spinRotStep);
    advLayout->addWidget(new QLabel("尺度:"));
    spinScaleStart = new QDoubleSpinBox(); spinScaleStart->setRange(0.2,2.0); spinScaleStart->setValue(0.8);
    spinScaleEnd   = new QDoubleSpinBox(); spinScaleEnd->setRange(0.2,2.0); spinScaleEnd->setValue(1.2);
    spinScaleStep  = new QDoubleSpinBox(); spinScaleStep->setRange(0.05,0.5); spinScaleStep->setValue(0.1);
    advLayout->addWidget(spinScaleStart);
    advLayout->addWidget(new QLabel("~"));
    advLayout->addWidget(spinScaleEnd);
    advLayout->addWidget(new QLabel("步长"));
    advLayout->addWidget(spinScaleStep);
    advLayout->addStretch();
    mainLayout->addLayout(advLayout);

    statusLabel = new QLabel("就绪");
    statusLabel->setStyleSheet("QLabel { color: #666; padding: 4px 0; }");
    mainLayout->addWidget(statusLabel);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setChildrenCollapsible(false);
    labTemplate = new QLabel("模板(裁剪后)");
    labSource   = new QLabel("待匹配(裁剪后)");
    labResult   = new QLabel("结果");
    for (auto lab : {labTemplate, labSource, labResult}) {
        lab->setAlignment(Qt::AlignCenter);
        lab->setMinimumSize(300, 200);
        lab->setStyleSheet("QLabel { border: 1px solid #aaa; border-radius: 8px; background: #fafafa; }");
        lab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        splitter->addWidget(lab);
    }
    splitter->setSizes({500, 500, 500});
    mainLayout->addWidget(splitter, 1);

    QHBoxLayout *resultLayout = new QHBoxLayout();
    QLabel *labelResult = new QLabel("匹配结果：");
    labelResult->setStyleSheet("QLabel { font-weight: bold; }");
    textResult = new QPlainTextEdit();
    textResult->setReadOnly(true);
    textResult->setMaximumHeight(80);
    textResult->setStyleSheet("QPlainTextEdit { background: #f9f9f9; border: 1px solid #ccc; border-radius: 4px; padding: 4px; font-family: monospace; }");
    textResult->setPlaceholderText("等待匹配...");
    resultLayout->addWidget(labelResult);
    resultLayout->addWidget(textResult);
    mainLayout->addLayout(resultLayout);

    connect(btnLoadTemp, &QPushButton::clicked, this, &MainWindow::OnLoadTemplate);
    connect(btnLoadSrc, &QPushButton::clicked, this, &MainWindow::OnLoadSource);
    connect(btnRunMatch, &QPushButton::clicked, this, &MainWindow::OnMatchClicked);
    connect(btnSaveResult, &QPushButton::clicked, this, &MainWindow::OnSaveResult);
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::OnPresetChanged);
    connect(sliderHighTh, &QSlider::valueChanged, this, &MainWindow::OnSliderValueChanged);
    connect(sliderLowTh, &QSlider::valueChanged, this, &MainWindow::OnSliderValueChanged);
    connect(sliderPyramid, &QSlider::valueChanged, this, &MainWindow::OnSliderValueChanged);
    connect(spinMinScore, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::OnSliderValueChanged);
    connect(spinGreed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::OnSliderValueChanged);

    ApplyPreset(1);
}

Image MainWindow::CropROI(const Image& src) {
    if (src.width == 0 || src.height == 0) return Image(0,0,0);
    int x = src.width / 4;
    int y = 20;
    int w = 3 * src.width / 4 - 60;
    int h = src.height - 60;
    if (w <= 0) w = src.width - x;
    if (h <= 0) h = src.height - y;
    if (x + w > src.width) w = src.width - x;
    if (y + h > src.height) h = src.height - y;
    return src.Crop(x, y, w, h);
}

void MainWindow::OnLoadTemplate() {
    QString path = QFileDialog::getOpenFileName(this, "选择模板", "./assets", "图像 (*.bmp *.png *.jpg *.jpeg)");
    if (path.isEmpty()) return;
    int w,h,c;
    uint8_t* data = stbi_load(path.toStdString().c_str(), &w, &h, &c, 3);
    if (!data) {
        QMessageBox::warning(this, "加载失败", "无法读取模板图片");
        return;
    }
    Image temp;
    temp.width = w; temp.height = h; temp.channel = 3;
    temp.data = new uint8_t[w * h * 3];
    memcpy(temp.data, data, w * h * 3);
    stbi_image_free(data);

    imgTemplate = CropROI(temp);
    temp.Release();

    if (imgTemplate.width == 0 || imgTemplate.height == 0) {
        QMessageBox::warning(this, "裁剪错误", "ROI裁剪结果为空，请检查图片尺寸");
        return;
    }

    labTemplate->setPixmap(ImageToPix(imgTemplate).scaled(labTemplate->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    UpdateStatus("模板已加载并裁剪: " + QFileInfo(path).fileName() + QString(" 尺寸 %1x%2").arg(imgTemplate.width).arg(imgTemplate.height));
#ifdef USE_OPENCV
    cvTemp = cv::imread(path.toStdString());
#endif
}

void MainWindow::OnLoadSource() {
    QString path = QFileDialog::getOpenFileName(this, "选择待匹配图", "./assets", "图像 (*.bmp *.png *.jpg *.jpeg)");
    if (path.isEmpty()) return;
    int w,h,c;
    uint8_t* data = stbi_load(path.toStdString().c_str(), &w, &h, &c, 3);
    if (!data) {
        QMessageBox::warning(this, "加载失败", "无法读取待匹配图");
        return;
    }
    Image temp;
    temp.width = w; temp.height = h; temp.channel = 3;
    temp.data = new uint8_t[w * h * 3];
    memcpy(temp.data, data, w * h * 3);
    stbi_image_free(data);

    imgSearch = CropROI(temp);
    temp.Release();

    if (imgSearch.width == 0 || imgSearch.height == 0) {
        QMessageBox::warning(this, "裁剪错误", "ROI裁剪结果为空，请检查图片尺寸");
        return;
    }

    labSource->setPixmap(ImageToPix(imgSearch).scaled(labSource->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    UpdateStatus("待匹配图已加载并裁剪: " + QFileInfo(path).fileName() + QString(" 尺寸 %1x%2").arg(imgSearch.width).arg(imgSearch.height));
#ifdef USE_OPENCV
    cvSrc = cv::imread(path.toStdString());
#endif
}

void MainWindow::OnPresetChanged(int idx) { ApplyPreset(idx); }

void MainWindow::ApplyPreset(int idx) {
    switch (idx) {
        case 0:
            sliderHighTh->setValue(80);
            sliderLowTh->setValue(5);
            sliderPyramid->setValue(1);
            spinMinScore->setValue(0.15);
            spinGreed->setValue(0.9);
            break;
        case 1:
            sliderHighTh->setValue(120);
            sliderLowTh->setValue(10);
            sliderPyramid->setValue(1);
            spinMinScore->setValue(0.2);
            spinGreed->setValue(0.9);
            break;
        case 2:
            sliderHighTh->setValue(150);
            sliderLowTh->setValue(20);
            sliderPyramid->setValue(1);
            spinMinScore->setValue(0.25);
            spinGreed->setValue(0.9);
            break;
    }
    UpdateStatus("参数已切换为 " + presetCombo->currentText());
}

void MainWindow::OnSliderValueChanged() { UpdateStatus("参数已手动调节"); }

void MainWindow::UpdateStatus(const QString &msg, bool isBusy) {
    statusLabel->setText(msg);
    btnRunMatch->setDisabled(isBusy);
    QCoreApplication::processEvents();
}

QPixmap MainWindow::ImageToPix(const Image& img) {
    if (img.width == 0 || img.height == 0 || img.data == nullptr)
        return QPixmap();
    QImage qimg(img.width, img.height, QImage::Format_RGB888);
    memcpy(qimg.bits(), img.data, img.width * img.height * 3);
    return QPixmap::fromImage(qimg);
}

void MainWindow::DrawResultInfo(QPixmap &pix, const QString &info) {
    QPainter painter(&pix);
    painter.setPen(QPen(Qt::white, 2));
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    QRect rect(10, 10, pix.width()-20, 100);
    painter.fillRect(rect, QColor(0, 0, 0, 150));
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop, info);
}

void MainWindow::OnMatchClicked() {
    if (imgTemplate.width == 0 || imgSearch.width == 0) {
        QMessageBox::warning(this, "提示", "请先加载模板和待匹配图（已自动裁剪）");
        return;
    }
    UpdateStatus("匹配中...", true);
    QCoreApplication::processEvents();

    Image tempGray = imgTemplate.ToGray();
    Image srcGray = imgSearch.ToGray();
    int highT = sliderHighTh->value();
    int lowT = sliderLowTh->value();
    int pyrLv = (int)spinPyramid->value();
    float minSc = (float)spinMinScore->value();
    float greed = (float)spinGreed->value();
    bool useRot = chkRotate->isChecked();
    bool useScale = chkScale->isChecked();
    bool enableMask = chkEnableMask->isChecked();
    matcher.SetEnableMask(enableMask);

    double rotS = spinRotStart->value(), rotE = spinRotEnd->value(), rotStep = spinRotStep->value();
    double scS = spinScaleStart->value(), scE = spinScaleEnd->value(), scStep = spinScaleStep->value();

    double outScore, outAngle, outScale;
    MyPoint2i matchPos = matcher.PyramidMatch(tempGray, srcGray, pyrLv, lowT, highT, minSc, greed,
                                              outScore, outAngle, outScale,
                                              useRot, useScale, rotS, rotE, rotStep, scS, scE, scStep);

    Image resImg = imgSearch.Clone();
    QString info;
    if (outScore >= 0.0 && matchPos.x >= 0 && matchPos.y >= 0) {
        matcher.DrawMatchPoint(resImg, matchPos);
        info = QString("坐标 (%1,%2)\n分数: %3\n角度: %4°\n尺度: %5")
               .arg(matchPos.x).arg(matchPos.y)
               .arg(outScore, 0, 'f', 4)
               .arg(outAngle, 0, 'f', 2)
               .arg(outScale, 0, 'f', 3);
    } else {
        info = "未检测到匹配目标";
    }

    QPixmap resultPix = ImageToPix(resImg).scaled(labResult->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    DrawResultInfo(resultPix, info);
    labResult->setPixmap(resultPix);
    textResult->setPlainText(info);
    UpdateStatus("匹配完成", false);

    if (outScore < 0) {
        QMessageBox::information(this, "结果", "未检测到有效匹配目标");
    } else if (outScore < minSc) {
        QMessageBox::warning(this, "结果", QString("低分匹配 (%.4f < %.2f) 坐标(%1,%2)")
                             .arg(outScore).arg(minSc).arg(matchPos.x).arg(matchPos.y));
    }
}

void MainWindow::OnSaveResult() {
    QString path = QFileDialog::getSaveFileName(this, "保存匹配结果", "./match_result.bmp", "BMP (*.bmp);;PNG (*.png)");
    if (path.isEmpty()) return;
    labResult->pixmap().save(path);
    UpdateStatus("结果保存至 " + path);
}
