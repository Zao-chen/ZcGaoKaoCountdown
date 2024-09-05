#include "widget.h"
#include "ui_widget.h"
#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QBuffer>
#include <windows.h>
#include <QSettings>
#include <string>
#include <QDateTime>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);


    // 创建一个QImage对象，用于存储图片。
    // 参数分别是：宽度、高度、格式。这里使用ARGB32，即包含alpha通道的32位颜色。
    QImage image(1920, 1080, QImage::Format_ARGB32);
    // 用白色填充图片背景
    image.fill(QColor(78, 162, 236));
    // 创建一个QPainter对象，用于在image上绘制
    QPainter painter(&image);
    // 设置画笔颜色
    painter.setPen(Qt::black);
    // 设置字体
    QFont font = painter.font();
    font.setPointSize(50);
    painter.setFont(font);
    // 在图片上绘制文本
    // 参数分别是：要绘制的文本，文本的起始位置

    /*时间差计算*/
    // 获取当前日期和时间
    QDateTime now = QDateTime::currentDateTime();
    // 设置目标日期（2025年6月7日）
    QDate targetDate(2025, 6, 7);
    // 创建一个目标日期时间对象，时间部分设为0时0分0秒
    QDateTime targetDateTime(targetDate, QTime(0, 0, 0));
    // 计算两个日期之间的天数差
    // 注意：daysTo()函数返回的是两个日期之间的完整天数差，包括开始和结束日期之间的每一天
    // 如果需要不包括今天在内的天数差，可以通过计算差值后减去1来实现
    qint64 daysDiff = targetDateTime.daysTo(now);
    // 由于我们想要计算的是从今天到目标日期的天数差，所以需要使用now.daysTo(targetDateTime)
    // 并且注意，如果目标日期在之前，这将返回一个负数
    daysDiff = now.daysTo(targetDateTime);
    // 如果daysDiff是负数，说明目标日期已经过去，可以输出相应的信息


    painter.drawText(QRectF(0, 0, 2800, 500), Qt::AlignCenter, "距离高考还有"+QString::number(daysDiff)+"天");
    // 绘制完成后，释放QPainter对象
    painter.end();
    // 将图片保存到文件
    // 参数分别是：文件路径、图片格式（如果文件扩展名正确，此参数可以省略）
    bool saved = image.save(qApp->applicationDirPath()+"/img.png");
    if(saved) {
        qDebug() << "图片保存成功！";
    } else {
        qDebug() << "图片保存失败！";
    }
    //壁纸注册表表
    QSettings wallPaper("HKEY_CURRENT_USER\\Control Panel\\Desktop",
                        QSettings::NativeFormat);
    //新的桌面图片路径
    QString path(qApp->applicationDirPath()+"/img1.png");
    //给壁纸注册表设置新的值（新的图片路径）
    wallPaper.setValue("Wallpaper",path);
    QByteArray byte = path.toLocal8Bit();
    //调用windowsAPI
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, byte.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);


    setMyAppAutoRun(true);
    qApp->exec();
}

Widget::~Widget()
{
    delete ui;
}

#define AUTO_RUN_KEY	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
//设置/取消自启动
//isStart: true(开机启动)    false(开机不启动)
void Widget::setMyAppAutoRun(bool isStart)
{
    QString application_name = QApplication::applicationName();//获取应用名称
    QSettings *settings = new QSettings(AUTO_RUN_KEY, QSettings::NativeFormat);//创建QSetting, 需要添加QSetting头文件
    if(isStart)
    {
        QString application_path = QApplication::applicationFilePath();//找到应用的目录
        settings->setValue(application_name, application_path.replace("/", "\\"));//写入注册表
    }
    else
    {
        settings->remove(application_name);		//从注册表中删除
    }
}
