#ifndef NETDETAIL_H
#define NETDETAIL_H

#include <QDialog>
#include <QPainter>
#include <QObject>
#include <QStackedWidget>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariantMap>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
#include <QSettings>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCloseEvent>
#include <kylin-nm/kylin-nm-interface.h>

#include "detailpage.h"
#include "ipv4page.h"
#include "ipv6page.h"
#include "coninfo.h"
#include "kwidget.h"
#include "ktabbar.h"
#include "oobetabbar.h"

using namespace kdk;

class NetDetail : public QWidget
{
    Q_OBJECT

public:
    NetDetail(PluginType type, QString interface, QString name, QString uuid, bool isPppoe, ConInfo info, QWidget *parent = nullptr);

    void updateConnect(KyConnectSetting &connetSetting);
    QString getInterface() {
        return m_deviceName;
    }
    
    PluginType getPluginType() const {
        return m_pluginType;
    }
protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *w, QEvent *event);

private:
    void initUI();
    void centerToScreen();
    void initComponent();
    void loadPage();
    void pagePadding();

    void setConfirmEnable();

    void setNetdetailSomeEnable(bool on);

    void checkNeedUpdate();
private:

    QStackedWidget * stackWidget;

    DetailPage     * detailPage;
    Ipv4Page       * ipv4Page;
    Ipv6Page       * ipv6Page;

    QWidget      * centerWidget;
    QWidget      * bottomWidget;

    QPushButton  * cancelBtn;
    QPushButton  * forgetBtn;
    QPushButton  * confimBtn;

    QFrame       * pageFrame;
    KTabBar      *m_netTabBar = nullptr;
    OobeTabBar      *m_oobeTabBar = nullptr;

    QString      m_name;
    QString      m_uuid;
    QString      m_deviceName;
    PluginType   m_pluginType;

    bool         m_isPppoe;

    bool         isIpv4Ok;
    bool         isIpv6Ok;
    bool         isConfirmBtnEnable;

    ConInfo      m_info;

private Q_SLOTS:
    void on_btnConfirm_clicked();
    void on_btnForget_clicked();
    void onPaletteChanged();

protected Q_SLOTS:
    void currentRowChangeSlot(int row);

Q_SIGNALS:
    void detailPageClose(QString, QString);
    void currentChanged(int);
    void deleteConnect(QString);
    void needUpdate(QString, QString);
};



#endif // NETDETAIL_H
