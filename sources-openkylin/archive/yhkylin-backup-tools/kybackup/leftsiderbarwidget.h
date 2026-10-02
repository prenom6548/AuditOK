#ifndef LEFTSIDERBARWIDGET_H
#define LEFTSIDERBARWIDGET_H

#include <QWidget>
#include <QObject>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QGSettings>
#include "functypeconverter.h"
#include "component/mylabel.h"
#include "component/pixmaplabel.h"
#include "knavigationbar.h"
#include "kconf2.h"

#include <ukuistylehelper/ukuistylehelper.h>
#include <ukuiwindowhelper.h>

using namespace kdk;

class LeftsiderbarWidget : public QWidget
{
    Q_OBJECT
public:
    // 启动模式，不同模式启动时展示的功能模块不同
    enum StartMode {
        normal,
        casper,
        stdUser
    };
    LeftsiderbarWidget(QWidget *parent = nullptr, StartMode mode = StartMode::normal);
    ~LeftsiderbarWidget();

    void initUi();

public slots:
    void setBusy(bool isBusy);
    void setCheckedFunc(int funcType);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void selected(int funcType);
    void doubleClicked();
    void funcType(QList<int> funcTypeList);
private:
    QGSettings *m_personalQgsettings;
    qreal m_transparency = 0.8;
    QWidget *m_parent;
    StartMode m_mode;

    QVBoxLayout *m_leftSideBarVLayout = nullptr;
    PixmapLabel  *m_mTitleIcon = nullptr;
    MyLabel      *m_titleLabel = nullptr;
    QButtonGroup  *m_funcGroup = nullptr;

    QList<int> m_deplayFuncs;
    QMap<int, int> m_idxToFunctype;
    QMap<int, int> m_functypeToIdx;
    KNavigationBar * m_navigationBar;


    UkuiWindowHelper *m_windowHelper = nullptr;
    bool m_active;
};

#endif // LEFTSIDERBARWIDGET_H
