#include "widget.h"
#include "ui_widget.h"


#include <QHBoxLayout>
#include <QIcon>
#include <QSettings>
#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QBuffer>
#include <windows.h>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTimer>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>

#include "ElaWidget.h"
#include "ElaMessageBar.h"

Widget::Widget(QWidget *parent)
    : ElaWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("早晨的高考倒计时");
    setWindowIcon(QIcon(":/img/logo.png"));


    /*初始化*/
    ui->pushButton_color->setLightDefaultColor(QColor(78, 162, 236));
    ui->pushButton_color->setLightDefaultColor(QColor(78, 162, 236));
    ui->pushButton_color->setLightHoverColor(QColor(78, 162, 236));
    ui->pushButton_color->setLightPressColor(QColor(78, 162, 236));
    ui->pushButton_color->setDarkDefaultColor(QColor(78, 162, 236));
    ui->pushButton_color->setDarkHoverColor(QColor(78, 162, 236));
    ui->pushButton_color->setDarkPressColor(QColor(78, 162, 236));

    QSettings *settings = new QSettings("Setting.ini",QSettings::IniFormat);
    ui->lineEdit_year->setText(settings->value("GlobelSetting/time_year").toString());
    ui->lineEdit_month->setText(settings->value("GlobelSetting/time_month").toString());
    ui->lineEdit_day->setText(settings->value("GlobelSetting/time_day").toString());
    ui->lineEdit_subyear->setText(settings->value("GlobelSetting/time_subyear").toString());
    ui->lineEdit_submonth->setText(settings->value("GlobelSetting/time_submonth").toString());
    ui->lineEdit_subday->setText(settings->value("GlobelSetting/time_subday").toString());

    /*初始化托盘*/
    initSysTrayIcon();
    /*绘制图片并设置壁纸*/
    printText();
    /*开机自启*/
    setMyAppAutoRun(true);


    fTimer = new QTimer(this);
    fTimer->stop();
    fTimer->setInterval(600000);//设置定时周期，单位：ms
    connect(fTimer,SIGNAL(timeout()),this,SLOT(on_timer_timeout()));
    fTimer->start();//定时器开始工作
}
Widget::~Widget()
{
    delete ui;
}

/*隐藏窗口*/
void Widget::hideWindow()
{
    this->hide();
}

/*开机自启*/
#define AUTO_RUN_KEY	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
void Widget::setMyAppAutoRun(bool isStart)
{
    QString application_name = QApplication::applicationName(); //获取应用名称
    QSettings *settings = new QSettings(AUTO_RUN_KEY, QSettings::NativeFormat); //创建QSetting, 需要添加QSetting头文件
    if(isStart)
    {
        QString application_path = QApplication::applicationFilePath(); //找到应用的目录
        settings->setValue(application_name, application_path.replace("/", "\\")); //写入注册表
    }
    else
    {
        settings->remove(application_name); //从注册表中删除
    }
}

/*创建系统托盘*/
void Widget::initSysTrayIcon()
{

    m_sysTrayIcon = new QSystemTrayIcon(this); //新建QSystemTrayIcon对象
    QIcon icon = QIcon(":/img/logo.png"); //资源文件添加的图标
    m_sysTrayIcon->setIcon(icon);
    m_sysTrayIcon->setToolTip("早晨的高考倒计时"); //当鼠标移动到托盘上的图标时，会显示此处设置的内容
    connect(m_sysTrayIcon, &QSystemTrayIcon::activated, //给QSystemTrayIcon添加槽函数
            [=](QSystemTrayIcon::ActivationReason reason)
    {
        switch(reason)
        {
        case QSystemTrayIcon::Trigger: //单击托盘图标
            this->show();
            break;
        case QSystemTrayIcon::DoubleClick: //双击托盘图标
            m_sysTrayIcon->showMessage("早晨的高考倒计时",
                                       "左键打开主界面，右键打开菜单",
                                       QSystemTrayIcon::Information,
                                       1000);
            break;
        default:
            break;
        }
    });
    createActions(); //建立托盘操作的菜单
    createMenu();
    m_sysTrayIcon->show(); //在系统托盘显示此对象
}

void Widget::printText()
{
    /*绘制图片*/
    QImage image(1920, 1080, QImage::Format_ARGB32); //创建一个QImage对象，用于存储图片。
    image.fill(QColor(67, 142, 207)); //填充图片背景
    QPainter painter(&image); //创建一个QPainter对象，用于在image上绘制
    painter.setPen(Qt::white); //设置画笔颜色
    QFont font = painter.font(); //设置字体

    /*时间差计算*/
    QDateTime now = QDateTime::currentDateTime(); //获取当前日期和时间
    QSettings *settings = new QSettings("Setting.ini",QSettings::IniFormat);
    QDate targetDate(settings->value("GlobelSetting/time_year").toInt(),settings->value("GlobelSetting/time_month").toInt(), settings->value("GlobelSetting/time_day").toInt());
    QDateTime targetDateTime(targetDate, QTime(0, 0, 0)); //创建一个目标日期时间对象，时间部分设为0时0分0秒
    qint64 daysDiff = targetDateTime.daysTo(now); // 计算两个日期之间的天数差
    daysDiff = now.daysTo(targetDateTime);
    /*绘制计时文字*/
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(0, -50, 2800, 500), Qt::AlignCenter, "距离高考还有");
    font.setPointSize(63);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(215, -60, 2800, 500), Qt::AlignCenter, QString::number(daysDiff));
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(315, -50, 2800, 500), Qt::AlignCenter, "天");

    /*时间差计算*/
    now = QDateTime::currentDateTime(); //获取当前日期和时间
    QDate targetDate2(settings->value("GlobelSetting/subtime_year").toInt(),settings->value("GlobelSetting/subtime_month").toInt(), settings->value("GlobelSetting/subtime_day").toInt());
    QDateTime targetDateTime2(targetDate2, QTime(0, 0, 0)); //创建一个目标日期时间对象，时间部分设为0时0分0秒
    daysDiff = targetDateTime2.daysTo(now); // 计算两个日期之间的天数差
    daysDiff = now.daysTo(targetDateTime2);
    /*绘制计时文字*/
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(0, 50, 2800, 500), Qt::AlignCenter, "距离春考还有");
    font.setPointSize(63);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(215, 40, 2800, 500), Qt::AlignCenter, QString::number(daysDiff));
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(315, 50, 2800, 500), Qt::AlignCenter, "天");

    /*绘制值日生表*/
    font.setPointSize(23);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(1050, 400+150, 700, 500), Qt::AlignRight, "今日帕鲁:");
    QDate currentDate = QDate::currentDate();
    int dayOfWeek = currentDate.dayOfWeek()-1;

    struct palu_list {
        QStringList get_food ;
        QStringList work;
    };
    palu_list _palu_list;

    _palu_list.get_food = settings->value("GlobelSetting/day1").toStringList();
    _palu_list.work = settings->value("GlobelSetting/day2").toStringList();

    font.setPointSize(19);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(1050, 450+150, 700, 500), Qt::AlignRight, "拿饭："+_palu_list.get_food[dayOfWeek]);
    painter.drawText(QRectF(1050, 485+150, 700, 500), Qt::AlignRight, "值日："+_palu_list.work[dayOfWeek]);
    qDebug()<<"ok1";
    /*获取图片*/
    QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);
    QEventLoop loop1;

    QNetworkReply *reply1 = m_manager->get(QNetworkRequest(QUrl("https://zaochenpan.oss-cn-shanghai.aliyuncs.com/sewoo.png")));
    connect(reply1, &QNetworkReply::finished, &loop1, &QEventLoop::quit);

    loop1.exec();

    if (reply1->error() == QNetworkReply::NoError) {
        QByteArray bytes = reply1->readAll();
        QImage img;
        if (img.loadFromData(bytes)) {
            qDebug() << "Image loaded successfully";
            // 假设这里有一个有效的QPainter对象painter
            painter.drawImage(0, 0, img);
        } else {
            qDebug() << "Failed to load image from data";
        }
    } else {
        qDebug() << "Network error:" << reply1->errorString();
    }

    reply1->deleteLater();

    /*绘制完毕*/
    painter.end(); //绘制完成后，释放QPainter对象
    image.save(qApp->applicationDirPath()+"/img.png"); //保存图片
    QSettings wallPaper("HKEY_CURRENT_USER\\Control Panel\\Desktop",
                        QSettings::NativeFormat); //壁纸注册表表
    QString path(qApp->applicationDirPath()+"/img.png"); //设置壁纸
    wallPaper.setValue("Wallpaper",path); //给壁纸注册表设置新的值（新的图片路径）
    QByteArray byte = path.toLocal8Bit();
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, byte.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE); //调用windowsAPI
}
/*托盘动作*/
void Widget::createActions()
{
    m_showMainAction = new QAction("主界面", this);
    connect(m_showMainAction,SIGNAL(triggered()),this,SLOT(on_showMainAction()));
    m_showGithubAction = new QAction("Github", this);
    connect(m_showGithubAction,SIGNAL(triggered()),this,SLOT(on_showGithubAction()));
    m_exitAppAction = new QAction("退出", this);
    connect(m_exitAppAction,SIGNAL(triggered()),this,SLOT(on_exitAppAction()));
}
/*创建托盘菜单*/
void Widget::createMenu()
{
    m_menu = new QMenu(this);
    m_menu->addAction(m_showMainAction); //新增菜单项
    m_menu->addAction(m_showGithubAction); //新增菜单项
    m_menu->addSeparator(); //增加分隔符
    m_menu->addAction(m_exitAppAction); //新增菜单项---退出程序
    m_sysTrayIcon->setContextMenu(m_menu); //把QMenu赋给QSystemTrayIcon对象
}

/*托盘Github*/
void Widget::on_showGithubAction()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Zao-chen/ZcGaoKaoCountdown", QUrl::TolerantMode));

}
/*托盘主界面*/
void Widget::on_showMainAction()
{
    this->show();

}
/*托盘推出*/
void Widget::on_exitAppAction()
{
    qApp->exit();
    /*绘制图片*/
    QImage image(1920, 1080, QImage::Format_ARGB32); //创建一个QImage对象，用于存储图片。
    image.fill(QColor(67, 142, 207)); //填充图片背景
    image.save(qApp->applicationDirPath()+"/img.png"); //保存图片
    QSettings wallPaper("HKEY_CURRENT_USER\\Control Panel\\Desktop",
                        QSettings::NativeFormat); //壁纸注册表表
    QString path(qApp->applicationDirPath()+"/img.png"); //设置壁纸
    wallPaper.setValue("Wallpaper",path); //给壁纸注册表设置新的值（新的图片路径）
    QByteArray byte = path.toLocal8Bit();
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, byte.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE); //调用windowsAPI
}

/*窗口关闭*/
void Widget::closeEvent(QCloseEvent *event)
{
    //忽略窗口关闭事件
    QApplication::setQuitOnLastWindowClosed( true );
    this->hide();
    event->ignore();
}


void Widget::on_pushButton_clicked()
{
    QSettings *settings = new QSettings("Setting.ini",QSettings::IniFormat);
    //ini配置文件默认不支持直接读写中文，需要手动设置下编码格式才行
    //configIni->setIniCodec("utf-8");//添上这句就不会中文出现乱码了
    //强烈建议统一用utf-8编码，包括代码文件。
    // 写入第一组数据
    settings->beginGroup("GlobelSetting");
    settings->setValue("time_year",ui->lineEdit_year->text());
    settings->setValue("time_month",ui->lineEdit_month->text());
    settings->setValue("time_day",ui->lineEdit_subday->text());
    settings->setValue("subtime_year",ui->lineEdit_subyear->text());
    settings->setValue("subtime_month",ui->lineEdit_submonth->text());
    settings->setValue("subtime_day",ui->lineEdit_subday->text());
    settings->endGroup();
    delete settings;
    settings =nullptr;

    printText();

    ElaMessageBar::success(ElaMessageBarType::TopRight, "保存成功", "配置文件已保存并刷新", 1000, this);
}


void Widget::on_pushButton_2_clicked()
{
    this->hide();
}
void Widget::on_timer_timeout(){
    //定时器中断响
    printText();
}
