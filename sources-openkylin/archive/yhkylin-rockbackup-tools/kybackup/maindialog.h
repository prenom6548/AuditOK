#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QDBusInterface>
#include <QDBusReply>
#include "leftsiderbarwidget.h"
#include "component/doubleclickwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QMainWindow
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

    void closeEvent(QCloseEvent *e);

protected:
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);

public slots:
    void sltMessageReceived(const QString &msg);
    void closeBtn();
    void tabletModeChange(bool isTabletMode);
    void selected(int funcType);

private:
    void initUI();
    void initTileBar();
    void initConnect();

    void mountBackupPartition();
    void umountBackupPartition();

    QStackedWidget * createWidgetFactory(int funcType, QWidget *parent = nullptr);
private:
    // 总体布局部分
    QHBoxLayout *m_totalHLayout = nullptr;
    LeftsiderbarWidget *m_leftSiderBarWidget = nullptr;
    QWidget *m_rightWidget = nullptr;
    QVBoxLayout *m_rightVLayout = nullptr;
    DoubleClickWidget *m_titleWidget = nullptr;
    QWidget *m_workWidget = nullptr;
    QVBoxLayout *m_workVLayout = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;

    // 标题栏部分
    QHBoxLayout *m_titleLayout = nullptr;
    QToolButton *m_menuOptionBtn = nullptr;
    QPushButton *m_minBtn = nullptr;
    QPushButton *m_maxBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;

    QAction *m_backupHelp = nullptr;
    QAction *m_backupAbout = nullptr;
    QAction *m_backupExit = nullptr;

    // 监控是否平板的dbus接口
    QDBusInterface * m_statusSessionDbus = nullptr;

    Ui::MainDialog *ui;
};
#endif // MAINDIALOG_H
