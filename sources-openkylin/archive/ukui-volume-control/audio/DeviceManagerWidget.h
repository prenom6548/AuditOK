#ifndef DEVICEMANAGERWIDGET_H
#define DEVICEMANAGERWIDGET_H

#include <memory>
#include <unordered_map>

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "IDeviceManagerItem.h"
#include "DBusClient.h"
#include "BaseType.h"

using namespace UkuiAudioFramwork;

class DeviceManagerItem : public QFrame, public IDeviceManagerItem
{
public:
    DeviceManagerItem(QLabel*, std::shared_ptr<ISwitchButton>, const QString&, const QString&, QWidget* = nullptr);
    ~DeviceManagerItem() = default;

public:
    virtual bool isChecked() const override;

    virtual void setChecked(bool) override;

    virtual QWidget* getWidget() override;

    virtual std::shared_ptr<ISwitchButton> getSwitchBtn() const override;

    virtual QVariant getData() const override;

private:
    void initUi();
    void initSlots();

private:
    QString m_cardName;
    QString m_portName;
    QLabel* m_pLabel = nullptr;
    std::shared_ptr<ISwitchButton> m_pSwitchBtn = nullptr;
};

class DeviceManagerWidget : public QWidget
{
    Q_OBJECT
public:
    DeviceManagerWidget(QLabel*, QWidget* = nullptr);
    ~DeviceManagerWidget() = default;

public:
    void insertToWidget(int);

    void addItem(const QString&, const QString&, const QString&);

    void removeItem(int);

    void setChecked(int, bool);

    std::list<QVariant> getDataList() const;

    void updateDeviceStatus(bool hasDevice);

Q_SIGNALS:
    void enableDeviceSignals(const QString&, const QString&, bool);

    void closeWindowSignals();

private:
    void initUi();

    void initSlots();

private Q_SLOTS:
    void closeWindow();

private:
    QLabel* m_pLabel = nullptr;
    QLabel* m_pNoDeviceImg = nullptr;
    QLabel* m_pNoDeviceText = nullptr;
    QVBoxLayout* m_pNoDeviceVLayout = nullptr;
    QScrollArea* m_pScrollArea = nullptr;
    QWidget* m_pDisDevManagerWidget = nullptr;
    QVBoxLayout* m_pVlayout = nullptr;
    QPushButton* m_pConfirmBtn;
    QVBoxLayout* m_pMainVLayout = nullptr;
    QStackedWidget* m_pStackWidget;
    std::vector<std::shared_ptr<IDeviceManagerItem>> m_devManagerItemVec {};
//    std::unordered_map<int, std::shared_ptr<IDeviceManagerItem>> m_devManagerItemMap {};
    NodeType m_type;
};

#endif // DEVICEMANAGERWIDGET_H
