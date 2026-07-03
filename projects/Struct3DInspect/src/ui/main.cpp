#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("结构光3D检测系统");
    w.resize(1400, 900);
    w.show();
    return a.exec();
}
