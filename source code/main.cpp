#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(w.StartOK) {
        w.setWindowTitle("DDSAR Ver2.1.0");
        w.show();
    }
    else {
        w.~MainWindow();
    }
    return a.exec();
}
