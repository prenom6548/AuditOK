#ifndef NODEVICEPAGE_H
#define NODEVICEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include "defaultgetset.h"

class NoDevicePage : public QWidget
{
    Q_OBJECT

public:
    explicit NoDevicePage(QWidget *parent = nullptr);
    void setTips(QString str1,QString str2);
    ~NoDevicePage();
private:
    DefaultGetSet *gsettingItem = nullptr;
    QWidget *iconContainer = nullptr;
    QLabel *iconLabel = nullptr;
    QHBoxLayout *iconHLayout = nullptr;
    QWidget *titleContainer = nullptr;
    QLabel *titleLabel = nullptr;
    QHBoxLayout *titleHLayout = nullptr;
    QWidget *detailContainer = nullptr;
    QLabel *detailLabel = nullptr;
    QHBoxLayout *detailHLayout = nullptr;

    QVBoxLayout *layout = nullptr;
private Q_SLOTS:
    void dealSystemGsettingChange();


};

#endif // NODEVICEPAGE_H
