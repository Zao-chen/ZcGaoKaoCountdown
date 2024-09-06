#include "widget.h"
#include "ui_widget.h"


#include <QHBoxLayout>
#include <QIcon>

#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QBuffer>
#include <windows.h>
#include <QSettings>
#include <string>
#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTimer>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ElaLineEdit.h"
#include "ElaScrollPageArea.h"
#include "ElaText.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("关于..");

    ElaScrollPageArea* spinBoxArea = new ElaScrollPageArea(this);
    QHBoxLayout* spinBoxLayout = new QHBoxLayout(spinBoxArea);

    _edit_year = new ElaLineEdit(this);
    spinBoxLayout->addWidget(new ElaText("年", this));
    spinBoxLayout->addWidget(_edit_year);

    _edit_month = new ElaLineEdit(this);
    spinBoxLayout->addWidget(new ElaText("月", this));
    spinBoxLayout->addWidget(_edit_month);

    _edit_day = new ElaLineEdit(this);
    spinBoxLayout->addWidget(new ElaText("日", this));
    spinBoxLayout->addWidget(_edit_day);

    QVBoxLayout* centerLayout = new QVBoxLayout();
    centerLayout->addWidget(spinBoxArea);

    /*初始化托盘*/
    initSysTrayIcon();

    /*绘制图片*/
    QImage image(1920, 1080, QImage::Format_ARGB32); //创建一个QImage对象，用于存储图片。
    image.fill(QColor(78, 162, 236)); //填充图片背景
    QPainter painter(&image); //创建一个QPainter对象，用于在image上绘制
    painter.setPen(Qt::black); //设置画笔颜色
    QFont font = painter.font(); //设置字体
    font.setPointSize(50);
    painter.setFont(font); //在图片上绘制文本

    /*时间差计算*/
    QDateTime now = QDateTime::currentDateTime(); //获取当前日期和时间
    QDate targetDate(2025, 6, 7); //设置目标日期（2025年6月7日）
    QDateTime targetDateTime(targetDate, QTime(0, 0, 0)); //创建一个目标日期时间对象，时间部分设为0时0分0秒
    qint64 daysDiff = targetDateTime.daysTo(now); // 计算两个日期之间的天数差
    daysDiff = now.daysTo(targetDateTime);

    /*绘制文字*/
    painter.drawText(QRectF(0, 0, 2800, 500), Qt::AlignCenter, "距离高考还有"+QString::number(daysDiff)+"天");
    painter.end(); //绘制完成后，释放QPainter对象

    /*设置壁纸*/
    image.save(qApp->applicationDirPath()+"/img.png"); //保存图片
    QSettings wallPaper("HKEY_CURRENT_USER\\Control Panel\\Desktop",
                        QSettings::NativeFormat); //壁纸注册表表
    QString path(qApp->applicationDirPath()+"/img.png"); //设置壁纸
    wallPaper.setValue("Wallpaper",path); //给壁纸注册表设置新的值（新的图片路径）
    QByteArray byte = path.toLocal8Bit();
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, byte.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE); //调用windowsAPI

    /*开机自启*/
    setMyAppAutoRun(true);

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
}

/*窗口关闭*/
void Widget::closeEvent(QCloseEvent *event)
{
    //忽略窗口关闭事件
    QApplication::setQuitOnLastWindowClosed( true );
    this->hide();
    event->ignore();
}

