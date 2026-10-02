#ifndef SEARCHDEVICEWIN_H
#define SEARCHDEVICEWIN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>

#include "searchdeviceitem.h"
#include "util.h"
#include "loadanimation.h"
#include "devicecodewidget.h"
#include "publicattributes.hpp"
#include "levitationbutton.h"

class SearchDeviceWin : public QWidget
{
    Q_OBJECT
public:
    explicit SearchDeviceWin(QWidget *parent = nullptr);
    ~SearchDeviceWin();

    void setTheme(PublicAttributes::Theme theme);

    void setItemInfo(const QMap<QString, KCommon::DiscoveryDeviceInfo> &map);

    void showLoad();

    void showNoNetWork();

    void setSearchBtnState(bool isShow);

    void changeFontSize(double fontSize);

protected:
    void resizeEvent(QResizeEvent *event) override;

Q_SIGNALS:
    void sigItemInfo(QString);
    void sigContinueSearch();
    void sigInitDeviceFinish();
    void sigDeviceCode(QString);

private:
    void initUI();
    void showSearchBtn(bool isShow);

private:
    enum WinIndex {
        WinLoad = 0,  // 加载页面
        WinRequest,   // 结果页面
        WinNoNetWork, // 无网络页面
    };

    QListWidget *m_listWidget = nullptr;
    LoadAnimation *m_loadAnimation = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    LevitationButton *m_searchBtn = nullptr;
    QLabel *m_noNetWorkLab = nullptr;

    bool isShowSearchBtn = false;
    bool isSearchInitFinish = false;
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light; // 当前主题
    double m_fontSize = 10;
};

#endif // SEARCHDEVICEWIN_H
