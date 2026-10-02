#ifndef UKUI4DEVICEPORTSELECTITEM_H
#define UKUI4DEVICEPORTSELECTITEM_H

#include <QLabel>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "IDevicePortSelectItem.h"

class Ukui4DevicePortItem : public QWidget, public IDevicePortItem
{
public:
    Ukui4DevicePortItem(QLabel*, QPushButton*, const QString&, const QString&, QWidget* = nullptr);
    ~Ukui4DevicePortItem() = default;

public:
    void initUi();

    virtual QWidget* getWidget() override;

    virtual bool isChecked() override;

    virtual void setChecked(bool) override;

    virtual void setIcon(const QIcon&) override;

private:
    QLabel* m_pDisplayLabel = nullptr;
    QPushButton* m_pIconBtn = nullptr;
};

class Ukui4DevicePortSelectItem : public QWidget, public IDevicePortSelectItem
{
public:
    Ukui4DevicePortSelectItem(QListWidget*, QWidget* = nullptr);

public:
    QListWidget* getListWidget() const;

    virtual void initSlots() override;

    virtual void setCurrentIndex(int) override;

    virtual void insertItem(std::shared_ptr<IDevicePortItem>) override;

    virtual void removeItem(const QString&, const QString&) override;

    virtual QVariant currentData(int = Qt::UserRole) const override;

    virtual int count() const override;

    virtual QWidget* getWidget() override;

Q_SIGNALS:
    void deviceChangedSignals(const QString&, const QString&);

private Q_SLOTS:
    void currentRowChangedSlots(int);

private:
    void setListWidgetSelectItem(int);

private:
    QListWidget* m_pListWidget = nullptr;
};

#endif // UKUI4DEVICEPORTSELECTITEM_H
