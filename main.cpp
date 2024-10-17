#include <QApplication>
#include <QDir>

#include "widget.h"
#include "ElaApplication.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElaApplication::getInstance()->init();
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    Widget w;
    return a.exec();
}
