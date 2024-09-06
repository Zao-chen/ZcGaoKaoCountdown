#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>

class ElaLineEdit;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
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

private:
    Ui::Widget *ui;

    QSystemTrayIcon *m_sysTrayIcon; //系统托盘
    void initSysTrayIcon();

    QMenu *m_menu;                  //菜单
    QAction *m_showGithubAction;      //动作
    QAction *m_showMainAction;      //动作
    QAction *m_exitAppAction;       //动作
    void createActions();


    ElaLineEdit* _edit_year{nullptr};
    ElaLineEdit* _edit_month{nullptr};
    ElaLineEdit* _edit_day{nullptr};

protected:
    void closeEvent (QCloseEvent *event) override;
};
#endif // WIDGET_H
