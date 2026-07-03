#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QFont>
#include "algorithm/ShapeMatch.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void OnLoadTemplate();
    void OnLoadSource();
    void OnMatchClicked();
    void OnSaveResult();
    void OnPresetChanged(int index);
    void OnSliderValueChanged();
private:
    QLabel *labTemplate, *labSource, *labResult;
    QPushButton *btnLoadTemp, *btnLoadSrc, *btnRunMatch, *btnSaveResult;
    QSlider *sliderHighTh, *sliderLowTh, *sliderPyramid;
    QDoubleSpinBox *spinHighTh, *spinLowTh, *spinPyramid;
    QDoubleSpinBox *spinMinScore;
    QDoubleSpinBox *spinGreed;
    QComboBox *presetCombo;
    QCheckBox *chkRotate, *chkScale;
    QCheckBox *chkEnableMask;
    QDoubleSpinBox *spinRotStart, *spinRotEnd, *spinRotStep, *spinScaleStart, *spinScaleEnd, *spinScaleStep;
    QLabel *statusLabel;
    QPlainTextEdit *textResult;

    ShapeMatching matcher;
    Image imgTemplate, imgSearch;
    Image imgTemplateFull, imgSearchFull;

#ifdef USE_OPENCV
    cv::Mat cvTemp, cvSrc;
#endif

    void InitUI();
    QPixmap ImageToPix(const Image& img);
    void DrawResultInfo(QPixmap &pix, const QString &info);
    void UpdateStatus(const QString &msg, bool isBusy = false);
    void ApplyPreset(int idx);
    Image CropROI(const Image& src);
};
