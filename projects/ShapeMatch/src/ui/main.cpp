#include <QApplication>
#include "MainWindow.h"
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    a.setStyleSheet("QMainWindow { background: #f5f5f5; }"
                    "QLabel { color: #333; }"
                    "QPushButton { font: 10pt; }"
                    "QSlider::groove:horizontal { height: 6px; background: #ddd; border-radius: 3px; }"
                    "QSlider::handle:horizontal { width: 16px; height: 16px; background: #4CAF50; border-radius: 8px; }"
                    "QDoubleSpinBox, QComboBox { border: 1px solid #ccc; border-radius: 4px; padding: 2px; }"
                    "QCheckBox { spacing: 4px; }"
                    "QPlainTextEdit { font-family: monospace; }");
    MainWindow w;
    w.show();
    return a.exec();
}
