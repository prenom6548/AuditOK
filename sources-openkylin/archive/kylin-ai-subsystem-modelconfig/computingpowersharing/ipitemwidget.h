#ifndef IPITEMWIDGET_H
#define IPITEMWIDGET_H

#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/klabel.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#include "ukccframe.h"

class IpItemWidget : public UkccFrame {
    Q_OBJECT
public:
    explicit IpItemWidget(const QString &ip, bool isConnected = false, QWidget *parent = nullptr);

    void setIp(const QString &ip);
    void setConnected(bool connected);
    void setSharedModelName(const QString &modelName);

    QString ip() const { return m_ip; }
    bool isConnected() const { return m_isConnected; }

signals:
    void requestConnection(const QString &ip);

private slots:
    void onRequestConnectionClicked();

private:
    QString m_ip;
    bool m_isConnected = false;

    QLabel *m_iconLabel = nullptr;
    kdk::KLabel *m_ipLabel = nullptr;

    kdk::KLabel *m_separatorLabel = nullptr;
    kdk::KLabel *m_modelLabel = nullptr;
    kdk::KLabel *m_shareModelNameLabel = nullptr;

    kdk::KLabel *m_connectLabel = nullptr;
    kdk::KBorderlessButton *m_connectButton = nullptr;

    QHBoxLayout *m_mainLayout = nullptr;
};

#endif  // IPITEMWIDGET_H
