#include "MainWindow.h"
#include <QImage>
#include <QPainter>
#include <QFont>
#include <QApplication>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    setWindowTitle("结构件测量系统 v6");
    resize(1280, 800);
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧面板：结果文本框 + 图像
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setStyleSheet("QWidget { background: #ffffff; }");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(5, 5, 5, 5);
    leftLayout->setSpacing(5);

    resultText = new QTextEdit(this);
    resultText->setReadOnly(true);
    resultText->setMaximumHeight(150);
    resultText->setStyleSheet("QTextEdit { background: #f9f9f9; border: 1px solid #ccc; border-radius: 4px; font-family: monospace; font-size: 11px; }");
    resultText->setPlaceholderText("测量结果将显示在此...");
    leftLayout->addWidget(resultText);

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background: #ffffff; border: none; }");
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageLabel->setMinimumSize(400, 300);
    leftLayout->addWidget(imageLabel, 1);

    mainLayout->addWidget(leftPanel, 75);

    // 右侧控制面板 (占25%)
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(200);
    rightPanel->setStyleSheet("QWidget { background: #f0f0f0; }");
    QVBoxLayout *ctrlLayout = new QVBoxLayout(rightPanel);
    ctrlLayout->setAlignment(Qt::AlignTop);
    ctrlLayout->setSpacing(10);
    ctrlLayout->setContentsMargins(10, 20, 10, 20);

    // 标定输入
    QLabel *scaleLabel = new QLabel("标定 (像素/mm):", this);
    scaleLabel->setStyleSheet("font-weight: bold;");
    ctrlLayout->addWidget(scaleLabel);
    scaleEdit = new QLineEdit(this);
    scaleEdit->setText("1.0");
    scaleEdit->setPlaceholderText("输入像素/毫米");
    scaleEdit->setStyleSheet("padding: 4px;");
    ctrlLayout->addWidget(scaleEdit);

    // 操作按钮
    QPushButton *btnLoad = new QPushButton("加载图像", this);
    QPushButton *btnProcess = new QPushButton("处理图像", this);
    QPushButton *btnSave = new QPushButton("保存结果图", this);
    QPushButton *btnExport = new QPushButton("导出CSV", this);

    QString btnStyle = "QPushButton { background: #e0e0e0; border: 1px solid #aaa; border-radius: 4px; padding: 8px; font-size: 14px; }"
                       "QPushButton:hover { background: #d0d0d0; }";
    btnLoad->setStyleSheet(btnStyle);
    btnProcess->setStyleSheet(btnStyle);
    btnSave->setStyleSheet(btnStyle);
    btnExport->setStyleSheet(btnStyle);

    ctrlLayout->addWidget(btnLoad);
    ctrlLayout->addWidget(btnProcess);
    ctrlLayout->addWidget(btnSave);
    ctrlLayout->addWidget(btnExport);

    // 标注开关
    QLabel *annoLabel = new QLabel("显示标注:", this);
    annoLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    ctrlLayout->addWidget(annoLabel);
    showBoundary = new QCheckBox("边界线 (蓝)", this);
    showBoundary->setChecked(true);
    showCenter = new QCheckBox("中线 (绿)", this);
    showCenter->setChecked(true);
    showCross = new QCheckBox("横端线 (红)", this);
    showCross->setChecked(true);
    showText = new QCheckBox("数值标注", this);
    showText->setChecked(true);
    ctrlLayout->addWidget(showBoundary);
    ctrlLayout->addWidget(showCenter);
    ctrlLayout->addWidget(showCross);
    ctrlLayout->addWidget(showText);

    ctrlLayout->addStretch();

    mainLayout->addWidget(rightPanel, 25);

    // 连接信号
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadImage);
    connect(btnProcess, &QPushButton::clicked, this, &MainWindow::onProcess);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveResult);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExportCSV);
    connect(showBoundary, &QCheckBox::toggled, this, &MainWindow::onToggleAnnotations);
    connect(showCenter, &QCheckBox::toggled, this, &MainWindow::onToggleAnnotations);
    connect(showCross, &QCheckBox::toggled, this, &MainWindow::onToggleAnnotations);
    connect(showText, &QCheckBox::toggled, this, &MainWindow::onToggleAnnotations);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateDisplay();
}

void MainWindow::onLoadImage() {
    QString path = QFileDialog::getOpenFileName(this, "打开图像", "./assets", "Images (*.bmp *.png *.jpg *.jpeg)");
    if (path.isEmpty()) return;
    QImage img;
    if (!img.load(path)) {
        QMessageBox::warning(this, "错误", "无法加载图像");
        return;
    }
    currentImage = img;
    currentImageName = QFileInfo(path).fileName();
    originalPix = QPixmap::fromImage(img);
    processedPix = originalPix;
    isProcessed = false;
    displayPixmap(originalPix);
    resultText->setPlainText("已加载图像: " + currentImageName + "\n点击\"处理图像\"开始测量");
}

void MainWindow::onProcess() {
    if (currentImage.isNull()) {
        QMessageBox::information(this, "提示", "请先加载图像");
        return;
    }

    // 进度提示
    QProgressDialog progress("处理中...", "取消", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    double scale = scaleEdit->text().toDouble();
    if (scale <= 0) scale = 1.0;

    lastResult = measurer.process(currentImage, 2); // morph kernel=2

    progress.close();

    if (!lastResult.hasLength) {
        QMessageBox::warning(this, "测量失败", "未能提取有效工件轮廓，请检查图像");
        return;
    }

    // 绘制结果
    QPixmap pix = originalPix;
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    // 根据开关绘制
    if (showBoundary->isChecked()) {
        painter.setPen(QPen(Qt::blue, 2));
        for (const auto &line : lastResult.boundaryLines)
            painter.drawLine(line);
    }
    if (showCenter->isChecked() && lastResult.hasCenterLine) {
        painter.setPen(QPen(Qt::green, 3));
        painter.drawLine(lastResult.centerLine);
    }
    if (showCross->isChecked()) {
        painter.setPen(QPen(Qt::red, 1));
        painter.drawLine(lastResult.topLine);
        painter.drawLine(lastResult.bottomLine);
    }
    if (lastResult.hasBottomPoint) {
        painter.setBrush(Qt::green);
        painter.setPen(Qt::green);
        painter.drawEllipse(lastResult.bottomPoint, 8, 8);
        painter.setPen(Qt::red);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        QString coordTxt = QString("(%1,%2)").arg(lastResult.bottomCoord.x()).arg(lastResult.bottomCoord.y());
        painter.drawText(lastResult.bottomPoint + QPoint(10, -5), coordTxt);
    }
    if (showText->isChecked() && lastResult.hasLength) {
        painter.setPen(Qt::red);
        painter.setFont(QFont("Arial", 18, QFont::Bold));
        QString lengthText = QString::number(lastResult.length, 'f', 1) + "px";
        QPoint center = (lastResult.centerLine.p1() + lastResult.centerLine.p2()) / 2;
        painter.drawText(center - QPoint(40, 10), lengthText);
        // 显示角度
        painter.setFont(QFont("Arial", 12));
        painter.setPen(Qt::darkBlue);
        QString angleText = QString("L%1° R%2°").arg(lastResult.leftAngle, 0, 'f', 1).arg(lastResult.rightAngle, 0, 'f', 1);
        painter.drawText(QPoint(20, 40), angleText);
    }

    painter.end();
    processedPix = pix;
    isProcessed = true;
    displayPixmap(processedPix);

    // 更新文本报表
    updateResultText(lastResult, scale);
}

void MainWindow::onSaveResult() {
    if (processedPix.isNull()) {
        QMessageBox::information(this, "提示", "没有可保存的结果，请先加载并处理图像");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "保存结果图像", "./result.bmp", "Images (*.bmp *.png *.jpg *.jpeg)");
    if (path.isEmpty()) return;
    if (processedPix.save(path)) {
        QMessageBox::information(this, "完成", "结果图像已保存");
    } else {
        QMessageBox::warning(this, "错误", "保存失败");
    }
}

void MainWindow::onExportCSV() {
    if (!lastResult.hasLength) {
        QMessageBox::information(this, "提示", "请先进行测量");
        return;
    }
    double scale = scaleEdit->text().toDouble();
    if (scale <= 0) scale = 1.0;
    saveCSV(lastResult, scale);
}

void MainWindow::onToggleAnnotations() {
    if (isProcessed && !processedPix.isNull()) {
        // 重新绘制
        onProcess();
    }
}

void MainWindow::displayPixmap(const QPixmap &pix) {
    if (pix.isNull()) {
        imageLabel->setText("无图像");
        return;
    }
    QSize labelSize = imageLabel->size();
    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
        labelSize = QSize(400, 300);
    }
    QPixmap scaled = pix.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(scaled);
}

void MainWindow::updateDisplay() {
    if (isProcessed && !processedPix.isNull())
        displayPixmap(processedPix);
    else if (!originalPix.isNull())
        displayPixmap(originalPix);
    else
        imageLabel->setText("无图像");
}

void MainWindow::updateResultText(const StructureMeasure::Result &res, double scale) {
    QString report = formatReport(res, scale);
    resultText->setPlainText(report);
}

QString MainWindow::formatReport(const StructureMeasure::Result &res, double scale) {
    QString text;
    text += "========== 结构件测量报告 ==========\n";
    text += QString("图像: %1\n").arg(currentImageName);
    text += QString("标定系数: %1 像素/mm\n").arg(scale, 0, 'f', 3);
    text += "------------------------------------\n";
    text += QString("竖直总长: %1 px  (%.2f mm)\n").arg(res.length, 0, 'f', 1).arg(res.length / scale);
    text += QString("顶部宽度: %1 px  (%.2f mm)\n").arg(res.topWidth, 0, 'f', 1).arg(res.topWidth / scale);
    text += QString("底部宽度: %1 px  (%.2f mm)\n").arg(res.bottomWidth, 0, 'f', 1).arg(res.bottomWidth / scale);
    text += QString("左锥角: %1 °\n").arg(res.leftAngle, 0, 'f', 2);
    text += QString("右锥角: %1 °\n").arg(res.rightAngle, 0, 'f', 2);
    text += QString("对称度(左右锥角差): %1 °\n").arg(res.symDiff, 0, 'f', 2);
    text += QString("底部基准坐标: (%1, %2)\n").arg(res.bottomCoord.x()).arg(res.bottomCoord.y());
    text += "====================================\n";
    return text;
}

void MainWindow::saveCSV(const StructureMeasure::Result &res, double scale) {
    QString path = QFileDialog::getSaveFileName(this, "导出CSV", "./measurement.csv", "CSV (*.csv)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建CSV文件");
        return;
    }
    QTextStream out(&file);
    out << "图像,总长(px),总长(mm),顶宽(px),顶宽(mm),底宽(px),底宽(mm),左角(°),右角(°),对称差(°),底部X,底部Y\n";
    out << currentImageName << ","
        << QString::number(res.length, 'f', 1) << "," << QString::number(res.length / scale, 'f', 2) << ","
        << QString::number(res.topWidth, 'f', 1) << "," << QString::number(res.topWidth / scale, 'f', 2) << ","
        << QString::number(res.bottomWidth, 'f', 1) << "," << QString::number(res.bottomWidth / scale, 'f', 2) << ","
        << QString::number(res.leftAngle, 'f', 2) << ","
        << QString::number(res.rightAngle, 'f', 2) << ","
        << QString::number(res.symDiff, 'f', 2) << ","
        << res.bottomCoord.x() << "," << res.bottomCoord.y() << "\n";
    file.close();
    QMessageBox::information(this, "完成", "CSV已导出");
}
