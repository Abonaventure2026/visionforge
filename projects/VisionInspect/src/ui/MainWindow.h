#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QSplitter>
#include <QCheckBox>
#include <QComboBox>
#include <QTabWidget>
#include <vector>
#include "../core/ImageBase.h"
#include "../core/Timer.h"
#include "../calibration/CameraCalib.h"
#include "../algorithm/DefectDetect2D.h"
#include "../algorithm/Measure2D.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void OnLoadImage();
    void OnLoadMultipleImages();  // 批量加载
    void OnCalibrate();
    void OnDetect();
    void OnSaveResult();
    void OnExportCSV();
    void OnSaveCalib();           // 保存标定参数
    void OnLoadCalib();           // 加载标定参数

private:
    void InitUI();
    QPixmap ImageToPix(const Image& img);
    void UpdateStatus(const QString& msg);
    void UpdateReport(const QString& text);
    void BatchCalibrate(const std::vector<Image>& images);
    void DrawDefectBoxes(Image& img, const std::vector<DefectInfo>& defects);
    QString GetCalibFilePath();

    CameraCalib calib;
    DefectDetect2D detector;
    CalibParam calib_result;
    DefectResult detect_result;
    MeasureResult measure_result;

    Image current_image;
    std::vector<Image> calib_images;
    bool is_calibrated = false;

    QTabWidget* tabWidget;
    QLabel* labImage;
    QLabel* labResult;
    QTextEdit* txtReport;
    QDoubleSpinBox* spinBaseline;
    QDoubleSpinBox* spinFocal;
    QSpinBox* spinCalibFrames;
    QPushButton* btnLoad;
    QPushButton* btnLoadMultiple;
    QPushButton* btnCalibrate;
    QPushButton* btnDetect;
    QPushButton* btnSave;
    QPushButton* btnExport;
    QPushButton* btnSaveCalib;
    QPushButton* btnLoadCalib;
};
