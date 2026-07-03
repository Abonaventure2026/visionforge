#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QResizeEvent>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QProgressDialog>
#include <QStatusBar>
#include "StructureMeasure.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void onLoadImage();
    void onProcess();
    void onSaveResult();
    void onExportCSV();
    void onToggleAnnotations();
private:
    QLabel *imageLabel;
    QTextEdit *resultText;
    QPixmap originalPix, processedPix;
    bool isProcessed = false;
    QImage currentImage;
    QString currentImageName;
    StructureMeasure measurer;
    StructureMeasure::Result lastResult;

    // UI控件
    QLineEdit *scaleEdit;          // 标定系数 (像素/毫米)
    QCheckBox *showBoundary;       // 显示边界蓝线
    QCheckBox *showCenter;         // 显示中线
    QCheckBox *showCross;          // 显示上下横线
    QCheckBox *showText;           // 显示数值标注

    void setupUI();
    void displayPixmap(const QPixmap &pix);
    void updateDisplay();
    void updateResultText(const StructureMeasure::Result &res, double scale);
    QString formatReport(const StructureMeasure::Result &res, double scale);
    void saveCSV(const StructureMeasure::Result &res, double scale);
};
