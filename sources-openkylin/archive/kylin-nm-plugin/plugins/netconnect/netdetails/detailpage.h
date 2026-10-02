#ifndef DETAILPAGE_H
#define DETAILPAGE_H
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QPainter>
#include <QCheckBox>
#include <QSpacerItem>
#include <QDebug>
#include <QListWidget>
#include <QApplication>

#include "detailwidget.h"
#include "coninfo.h"

class DetailPage : public QFrame
{
    Q_OBJECT
public:
    DetailPage(bool isPppoe, bool isOobe = false, QWidget *parent = nullptr);

    void setSSID(const QString &ssid);
    void setProtocol(const QString &protocol);
    void setSecType(const QString &secType);
    void setHz(const QString &hz);
    void setChan(const QString &chan);
    void setBandWidth(const QString &brandWidth);
    void setIpv4(const QString &ipv4);
    void setIpv4Dns(const QString &ipv4Dns);
    void setIpv6(const QString &ipv6);
    void setMac(const QString &mac);

private:
    void initUI();
    void addDetailItem(QListWidget *listWidget, DetailWidget *detailWidget);

public:
    QListWidget * m_listWidget = nullptr;
    DetailWidget * m_ssidWidget = nullptr;
    DetailWidget * m_protocolWidget = nullptr;
    DetailWidget * m_bandwidthWidget = nullptr;
    DetailWidget * m_ipv4Widget = nullptr;
    DetailWidget * m_ipv4DnsWidget = nullptr;
    DetailWidget * m_ipv6Widget = nullptr;
    DetailWidget * m_macWidget = nullptr;

    LineEdit *mSSIDEdit;
    FixLabel *mSSIDLabel;
    QLabel *mProtocol;
    QLabel *mBandWidth;
    QLabel *mIPV4;
    QLabel *mIPV4Dns;
    FixLabel *mIPV6;
    QLabel *mMac;

private:
    QVBoxLayout *layout;
    QVBoxLayout *mDetailLayout;
    bool m_isPppoe;
    bool m_isOobe = false;
private Q_SLOTS:
    void setEnableOfSaveBtn();

Q_SIGNALS:
     void setDetailPageState(bool);

};

#endif // DETAILPAGE_H
