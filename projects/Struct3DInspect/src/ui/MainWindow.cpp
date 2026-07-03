#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <cmath>
#include "core/ImageProc.h"
#include "algorithm/GrayCodeGen.h"
#include "algorithm/PhaseShiftSolver.h"
#include "algorithm/MultiFreqUnwrap.h"
#include "core/Triangulate.h"
#include "core/PointCloudFilter.h"
#include "io/CSVWriter.h"
#include "io/PointCloudIO.h"
#include "../../third_party/stb_image.h"
#include "../../third_party/stb_image_write.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    InitUI();
    calib.baseline_mm = 120.0;
    calib.cam_focal_px = 1850.0;
    calib.gray_bits = 8;
    chkMultiFreq->setChecked(false);
}

void MainWindow::InitUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* btnLoad = new QPushButton("加载序列");
    QPushButton* btnGen = new QPushButton("生成投影条纹");
    QPushButton* btnRun = new QPushButton("3D重建");
    QPushButton* btnDepth = new QPushButton("保存深度图");
    QPushButton* btnCloud = new QPushButton("保存点云");
    QPushButton* btnCSV = new QPushButton("导出CSV");
    for (auto b : {btnLoad, btnGen, btnRun, btnDepth, btnCloud, btnCSV})
        btnLayout->addWidget(b);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

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
    paramLayout->addWidget(new QLabel("格雷码位数:"));
    spinBits = new QSpinBox();
    spinBits->setRange(4, 12);
    spinBits->setValue(8);
    paramLayout->addWidget(spinBits);
    chkMultiFreq = new QCheckBox("多频外差");
    chkMultiFreq->setChecked(false);
    paramLayout->addWidget(chkMultiFreq);
    paramLayout->addStretch();
    mainLayout->addLayout(paramLayout);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    labGray = new QLabel("格雷码解码图");
    labPhase = new QLabel("相位图");
    labDepth = new QLabel("深度图");
    for (auto lab : {labGray, labPhase, labDepth}) {
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

    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::OnLoadSeq);
    connect(btnGen, &QPushButton::clicked, this, &MainWindow::OnGenPattern);
    connect(btnRun, &QPushButton::clicked, this, &MainWindow::OnRunReconstruct);
    connect(btnDepth, &QPushButton::clicked, this, &MainWindow::OnSaveDepth);
    connect(btnCloud, &QPushButton::clicked, this, &MainWindow::OnSaveCloud);
    connect(btnCSV, &QPushButton::clicked, this, &MainWindow::OnExportCSV);
}

QPixmap MainWindow::ImageToPix(const Image& img) {
    if (!img.IsValid()) return QPixmap();
    QImage qimg(img.width, img.height, QImage::Format_RGB888);
    if (img.channels == 1) {
        for (int i=0; i<img.height; ++i)
            for (int j=0; j<img.width; ++j) {
                uint8_t v = img.data[i*img.width+j];
                qimg.setPixel(j, i, qRgb(v,v,v));
            }
    } else if (img.channels == 3) {
        memcpy(qimg.bits(), img.data, img.width*img.height*3);
    }
    return QPixmap::fromImage(qimg);
}

QPixmap MainWindow::FloatToPix(const std::vector<float>& data, int w, int h, bool color) {
    if (data.empty()) return QPixmap();
    QImage qimg(w, h, QImage::Format_RGB888);
    float minV = *std::min_element(data.begin(), data.end());
    float maxV = *std::max_element(data.begin(), data.end());
    float range = maxV - minV;
    if (range < 1e-6) range = 1.0f;
    for (int i=0; i<h; ++i) {
        for (int j=0; j<w; ++j) {
            float v = (data[i*w+j] - minV) / range;
            uint8_t gray = static_cast<uint8_t>(v * 255);
            qimg.setPixel(j, i, qRgb(gray, gray, gray));
        }
    }
    return QPixmap::fromImage(qimg);
}

void MainWindow::OnLoadSeq() {
    QString dir = QFileDialog::getExistingDirectory(this, "选择拍摄序列", "./assets/capture");
    if (dir.isEmpty()) return;
    capture_seq.clear();
    QDir d(dir);
    QStringList filters = {"*.png","*.jpg","*.bmp"};
    d.setNameFilters(filters);
    auto files = d.entryList(QDir::Files);
    for (auto& f : files) {
        QString fullPath = dir + "/" + f;
        std::string path = fullPath.toStdString();
        int w,h,c;
        uint8_t* data = stbi_load(path.c_str(), &w, &h, &c, 3);
        if (data) {
            Image img(w,h,3);
            memcpy(img.data, data, w*h*3);
            capture_seq.push_back(img);
            stbi_image_free(data);
        }
    }
    txtReport->setText(QString("已加载 %1 帧").arg(capture_seq.size()));
    if (!capture_seq.empty())
        labGray->setPixmap(ImageToPix(capture_seq[0]).scaled(labGray->size(), Qt::KeepAspectRatio));
}

void MainWindow::OnGenPattern() {
    QString saveDir = QFileDialog::getExistingDirectory(this, "保存条纹图", "./assets/pattern");
    if (saveDir.isEmpty()) return;
    auto seq = GrayCodeGen::GenFullSeq(calib);
    auto shift = PhaseShiftSolver::Gen4StepShift(1280, 1024, calib.freqs[0]);
    int idx = 0;
    for (auto& img : seq) {
        QString path = saveDir + "/gray_" + QString::number(idx++) + ".png";
        std::string pathStr = path.toStdString();
        stbi_write_png(pathStr.c_str(), img.width, img.height, 1, img.data, img.width);
    }
    for (auto& img : shift) {
        QString path = saveDir + "/shift_" + QString::number(idx++) + ".png";
        std::string pathStr = path.toStdString();
        stbi_write_png(pathStr.c_str(), img.width, img.height, 1, img.data, img.width);
    }
    QMessageBox::information(this, "完成", "条纹图生成完毕");
}

void MainWindow::OnRunReconstruct() {
    if (capture_seq.size() < 20) {
        QMessageBox::warning(this, "错误", "序列帧数不足，至少需要 2*gray_bits + shift_steps 帧");
        return;
    }
    calib.baseline_mm = spinBaseline->value();
    calib.cam_focal_px = spinFocal->value();
    calib.gray_bits = spinBits->value();

    int bits = calib.gray_bits;
    int step = 4;
    std::vector<Image> gray_frames;
    for (int i=0; i<2*bits; ++i) gray_frames.push_back(capture_seq[i]);
    std::vector<Image> shift_frames;
    for (int i=2*bits; i<2*bits+step; ++i) shift_frames.push_back(capture_seq[i]);

    auto code_map = GrayCodeGen::Decode(gray_frames, bits);
    auto wrap = PhaseShiftSolver::SolveWrap(shift_frames);

    std::vector<float> abs_phase;
    if (chkMultiFreq->isChecked()) {
        std::vector<std::vector<float>> wraps = {wrap};
        abs_phase = MultiFreqUnwrap::Unwrap(wraps, {calib.freqs[0]}, code_map);
    } else {
        abs_phase.resize(wrap.size());
        double pi = 3.1415926535;
        for (size_t i=0; i<wrap.size(); ++i)
            abs_phase[i] = code_map[i] * 2 * pi + wrap[i];
    }

    int w = capture_seq[0].width, h = capture_seq[0].height;
    Triangulate::CalcDepthAndCloud(abs_phase, calib, w, h, depth_map, point_cloud);

    PointCloudFilter::StatisticalOutlierRemoval(point_cloud);
    PointCloudFilter::VoxelDownsample(point_cloud, 0.5f);

    last_result.valid = false;
    if (!point_cloud.empty()) {
        float minX=1e9,maxX=-1e9,minY=1e9,maxY=-1e9,minZ=1e9,maxZ=-1e9;
        for (auto& p : point_cloud) {
            minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
            minZ = std::min(minZ, p.z); maxZ = std::max(maxZ, p.z);
        }
        last_result.width_mm = maxX - minX;
        last_result.height_mm = maxY - minY;
        last_result.thickness_mm = maxZ - minZ;
        last_result.point_cloud = point_cloud;
        last_result.valid = true;
        txtReport->setText(QString("宽: %1 mm, 高: %2 mm, 厚: %3 mm\n点云数: %4")
                           .arg(last_result.width_mm,0,'f',2)
                           .arg(last_result.height_mm,0,'f',2)
                           .arg(last_result.thickness_mm,0,'f',2)
                           .arg(point_cloud.size()));
    } else {
        txtReport->setText("重建失败：未生成有效点云，请检查参数或输入序列");
    }

    labPhase->setPixmap(FloatToPix(wrap, w, h, false).scaled(labPhase->size(), Qt::KeepAspectRatio));
    labDepth->setPixmap(FloatToPix(depth_map, w, h, false).scaled(labDepth->size(), Qt::KeepAspectRatio));
}

void MainWindow::OnSaveDepth() {
    if (depth_map.empty()) {
        QMessageBox::warning(this, "提示", "深度图为空，无法保存");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "保存深度图", "./depth.png", "PNG (*.png)");
    if (path.isEmpty()) return;
    int w = capture_seq[0].width, h = capture_seq[0].height;
    std::vector<uint8_t> img_data(w*h);
    float minV=*std::min_element(depth_map.begin(), depth_map.end());
    float maxV=*std::max_element(depth_map.begin(), depth_map.end());
    float range = maxV - minV;
    if (range < 1e-6) range = 1.0f;
    for (size_t i=0; i<depth_map.size(); ++i)
        img_data[i] = static_cast<uint8_t>((depth_map[i]-minV)/range*255);
    std::string pathStr = path.toStdString();
    if (stbi_write_png(pathStr.c_str(), w, h, 1, img_data.data(), w)) {
        QMessageBox::information(this, "成功", "深度图保存成功");
    } else {
        QMessageBox::warning(this, "错误", "保存深度图失败");
    }
}

void MainWindow::OnSaveCloud() {
    if (point_cloud.empty()) {
        QMessageBox::warning(this, "提示", "点云为空，无法保存。请先执行成功的重建。");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "保存点云", "./cloud.ply", "PLY (*.ply)");
    if (path.isEmpty()) return;
    if (SavePLY(path.toStdString(), point_cloud)) {
        QMessageBox::information(this, "成功", "点云保存成功");
    } else {
        QMessageBox::warning(this, "错误", "保存点云失败");
    }
}

void MainWindow::OnExportCSV() {
    if (!last_result.valid) {
        QMessageBox::warning(this, "提示", "当前没有有效的测量结果，请先执行成功的重建。");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "导出CSV", "./measure.csv", "CSV (*.csv)");
    if (path.isEmpty()) return;
    if (ExportCSV(path.toStdString(), last_result)) {
        QMessageBox::information(this, "成功", "CSV报表导出成功");
    } else {
        QMessageBox::warning(this, "错误", "导出CSV失败");
    }
}
