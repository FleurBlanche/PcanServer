#include "mainwindow.h"

#include <QApplication>

/*
 * pointer of the mainwindow
 * for global usage
*/
MainWindow * mw;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mw = new MainWindow();
    mw->show();
    return a.exec();
}
