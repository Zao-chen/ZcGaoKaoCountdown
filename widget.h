#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QNetworkAccessManager>

#include "ElaWidget.h"

class ElaLineEdit;


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public ElaWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    void setMyAppAutoRun(bool isStart);
    ~Widget();

private slots:
    void on_showMainAction();
    void on_showGithubAction();
    void on_exitAppAction();    void createMenu();
    void hideWindow();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::Widget *ui;
    QSystemTrayIcon *m_sysTrayIcon; //系统托盘
    void initSysTrayIcon();
    void printText();
    QMenu *m_menu;                  //菜单
    QAction *m_showGithubAction;      //动作
    QAction *m_showMainAction;      //动作
    QAction *m_exitAppAction;       //动作
    void createActions();
    QNetworkAccessManager *m_manager;

protected:
    void closeEvent (QCloseEvent *event) override;
};
#endif // WIDGET_H
