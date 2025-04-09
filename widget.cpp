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
#include <QStandardPaths>

#include "ElaWidget.h"
#include "ElaMessageBar.h"

Widget::Widget(QWidget *parent)
    : ElaWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("早晨的高考倒计时");
    setWindowIcon(QIcon(":/img/logo.png"));
    /*配置读取*/
    QSettings *settings = new QSettings("Setting.ini",QSettings::IniFormat);
    ui->pushButton_mainData->setSelectedDate(settings->value("GlobelSetting/time_main").toDate());
    ui->pushButton_subData->setSelectedDate(settings->value("GlobelSetting/time_sub").toDate());
    ui->lineEdit_mainText->setText(settings->value("GlobelSetting/text_main").toString());
    ui->lineEdit_subText->setText(settings->value("GlobelSetting/text_sub").toString());
    ui->lineEdit_autoOpen->setText(settings->value("GlobelSetting/auto_open").toString());
    ui->lineEdit_imgUrl->setText(settings->value("GlobelSetting/img_url").toString());
    QStringList items = settings->value("GlobelSetting/daily").toStringList();
    for (const QString &itemText : items) {
        QListWidgetItem *item = new QListWidgetItem(itemText, ui->listWidget);
        item->setFlags(item->flags() | Qt::ItemIsEditable);  // 设置项为可编辑
    }

    /*初始化托盘*/
    initSysTrayIcon();
    /*绘制图片并设置壁纸*/
    printText();
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
/*绘制背景*/
void Widget::printText()
{
    QSettings *settings = new QSettings("Setting.ini",QSettings::IniFormat);
    /*绘制图片*/
    QImage image(1920, 1080, QImage::Format_ARGB32); //创建一个QImage对象，用于存储图片。
    image.fill(QColor(0, 191, 255)); //填充图片背景
    QPainter painter(&image); //创建一个QPainter对象，用于在image上绘制
    painter.setPen(Qt::white); //设置画笔颜色
    QFont font = painter.font(); //设置字体
    /*获取图片*/
    QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);
    QEventLoop loop1;
    QNetworkReply *reply1 = m_manager->get(QNetworkRequest(QUrl(settings->value("img_url").toString())));
    connect(reply1, &QNetworkReply::finished, &loop1, &QEventLoop::quit);
    loop1.exec();
    if (reply1->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply1->readAll();
        QImage img;
        if (img.loadFromData(bytes)) {
            qDebug() << "Image loaded successfully";
            painter.drawImage(0, 0, img);
        }
    }
    reply1->deleteLater();
    /*时间差计算*/
    QDate now = QDate::currentDate();
    QDate targetDate(settings->value("GlobelSetting/time_main").toDate());
    qint64 daysDiff = targetDate.daysTo(now); // 计算两个日期之间的天数差
    qInfo()<<"主计时时间差："<<daysDiff;
    /*绘制计时文字*/
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(0, -50, 2800, 500), Qt::AlignCenter, settings->value("GlobelSetting/text_main").toString());
    font.setPointSize(63);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(215, -60, 2800, 500), Qt::AlignCenter, QString::number(daysDiff));
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(315, -50, 2800, 500), Qt::AlignCenter, "天");

    /*时间差计算*/
    now = QDate::currentDate();
    targetDate = settings->value("GlobelSetting/time_sub").toDate();
    daysDiff = targetDate.daysTo(now); // 计算两个日期之间的天数差
    /*绘制计时文字*/
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(0, 50, 2800, 500), Qt::AlignCenter, settings->value("GlobelSetting/text_sub").toString());
    font.setPointSize(63);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(215, 40, 2800, 500), Qt::AlignCenter, QString::number(daysDiff));
    font.setPointSize(35);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(315, 50, 2800, 500), Qt::AlignCenter, "天");
    /*绘制值日生表*/
    font.setPointSize(23);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(1050, 400+150, 700, 500), Qt::AlignRight, "今日:");
    //当前是星期几
    QDate currentDate = QDate::currentDate();
    int dayOfWeek = currentDate.dayOfWeek()-1;
    font.setPointSize(19);
    painter.setFont(font); //在图片上绘制文本
    painter.drawText(QRectF(1050, 450+150, 700, 500), Qt::AlignRight,settings->value("GlobelSetting/daily").toStringList()[dayOfWeek]);
    /*绘制完毕*/
    painter.end(); //绘制完成后，释放QPainter对象
    image.save(qApp->applicationDirPath()+"/img.png"); //保存图片
    QSettings wallPaper("HKEY_CURRENT_USER\\Control Panel\\Desktop",
                        QSettings::NativeFormat); //壁纸注册表表
    QString path(qApp->applicationDirPath()+"/img.png"); //设置壁纸
    wallPaper.setValue("Wallpaper",path); //给壁纸注册表设置新的值（新的图片路径）
    QByteArray byte = path.toLocal8Bit();
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, byte.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE); //调用windowsAPI
    /*打开文件*/
    m_manager = new QNetworkAccessManager(this); //新建QNetworkAccessManager对象
    QEventLoop loop; //循环
    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(settings->value("GlobelSetting/auto_open").toString()))); //这里是请求网址
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit())); //绑定回复事件
    loop.exec(); //循环直到有回复
    QString read = reply->readAll();
    reply->deleteLater(); //释放内存
    qInfo() << "正在下载" << read;

    // 获取系统的下载目录路径
    QString downloadFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    // 从 URL 中提取文件名
    QString fileName = read.split("/").last();
    QString savePath = downloadFolder + "/" + fileName;  // 下载到下载文件夹，并命名为 file.pdf
    qInfo() << "保存为" << savePath;
    // 设置 QNetworkAccessManager 来下载文件
    QNetworkAccessManager manager;
    QNetworkRequest request((QUrl(read)));
    // 发起 GET 请求
    QNetworkReply *reply2 = manager.get(request);
    // 连接信号和槽

    QEventLoop loop2; //循环
    connect(reply2, SIGNAL(finished()), &loop2, SLOT(quit())); //绑定回复事件
    loop2.exec(); //循环直到有回复

    qInfo() << "获取到返回";
    if (reply2->error() != QNetworkReply::NoError) {
        qWarning() << "Download failed:" << reply2->errorString();
        return;
    }
    // 获取文件的内容并保存
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save file:" << file.errorString();
        return;
    }
    // 写入文件内容
    file.write(reply2->readAll());
    file.close();
    qDebug() << "Download completed, file saved to:" << savePath;
    // 下载完成后，打开文件
    QDesktopServices::openUrl(QUrl::fromLocalFile(savePath));
    reply2->deleteLater();  // 清理 reply 对象
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
    m_menu = new ElaMenu(this);
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
    settings->setValue("time_main",ui->pushButton_mainData->getSelectedDate());
    settings->setValue("time_sub",ui->pushButton_subData->getSelectedDate());
    settings->setValue("text_main",ui->lineEdit_mainText->text());
    settings->setValue("text_sub",ui->lineEdit_subText->text());
    settings->setValue("img_url",ui->lineEdit_imgUrl->text());
    settings->setValue("auto_open",ui->lineEdit_autoOpen->text());
    QStringList listContents;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        listContents << item->text();
    }
    settings->setValue("daily",listContents);
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
