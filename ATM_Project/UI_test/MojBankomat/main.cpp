#include "atmgui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AtmGui w;
    w.show();
    return a.exec();
}