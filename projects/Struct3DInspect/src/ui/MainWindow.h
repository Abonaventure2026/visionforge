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
#include <vector>
#include "core/ImageBase.h"
#include "core/StructParam.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void OnLoadSeq();
    void OnGenPattern();
    void OnRunReconstruct();
    void OnSaveDepth();
    void OnSaveCloud();
    void OnExportCSV();
private:
    void InitUI();
    QPixmap ImageToPix(const Image& img);
    QPixmap FloatToPix(const std::vector<float>& data, int w, int h, bool color=true);
    CalibParam calib;
    std::vector<Image> capture_seq;
    std::vector<float> depth_map;
    std::vector<Point3D> point_cloud;
    InspectResult last_result;
    QLabel *labGray, *labPhase, *labDepth;
    QTextEdit *txtReport;
    QDoubleSpinBox *spinBaseline, *spinFocal;
    QSpinBox *spinBits;
    QCheckBox *chkMultiFreq;
};
