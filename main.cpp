#include <QApplication>

#include "widget.h"
#include "ElaApplication.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElaApplication::getInstance()->init();
    Widget w;
    return a.exec();
}
