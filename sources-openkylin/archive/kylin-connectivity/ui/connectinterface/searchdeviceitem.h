#ifndef SEARCHDEVICEITEM_H
#define SEARCHDEVICEITEM_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>

#include "util.h"
#include "publicattributes.hpp"

class SearchDeviceItem : public QPushButton
{
    Q_OBJECT
public:
    explicit SearchDeviceItem(QPushButton *parent = nullptr);
    ~SearchDeviceItem();

    //设置项按钮
    void setItemIcon(QString path);

    //设置项名称
    void setItemName(QString name);

    //设置手机型号
    void setItemDeviceInfo(KCommon::DiscoveryDeviceInfo deviceInfo);

    //设置项IP
    void setItemIP(QString ip);
    // 设置主题
    void setTheme(PublicAttributes::Theme theme);
    void changeFontSize(double fontSize);

    //获取项的名称
    QString getItemName();

    //获取项的型号
    KCommon::DiscoveryDeviceInfo getItemDeviceInfo();

    //获取项的IP
    QString getItemIp();

private:
    void initUI();

private:
    //项按钮
    QLabel *m_itemIconLabel = nullptr;
    //用户名
    QLabel *m_itemNameLabel = nullptr;
    //手机型号
    //    QLabel *m_itemModelLabel = nullptr;

    QString m_name;
    KCommon::DiscoveryDeviceInfo m_deviceInfo;
    QString m_itemIP;
};

#endif // SEARCHDEVICEITEM_H
