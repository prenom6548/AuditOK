#ifndef UKUI4SELECTCOMBOXITEM_H
#define UKUI4SELECTCOMBOXITEM_H

#include <QComboBox>
#include <QListWidget>

#include "ISelectComboxItem.h"
#include "Ukui4DevicePortSelectItem.h"

///**
//* @brief 包含SelectCombox子项的类型
//*/
//enum class SelectComboxType
//{
//    SELECT_COMBOX_INPUT = 0,    // 输入设备子项
//    SELECT_COMBOX_OUTPUT,       // 输出设备子项
//    SELECT_COMBOX_SOUNDEFFECT,  // 音效主题子项
//    SELECT_COMBOX_VOLUMEADJUST, // 音量调节子项
//    SELECT_COMBOX_NOTIFICTION,  // 接收通知子项
//};

class Ukui4SelectCombox : public ISelectBox
{
public:
    Ukui4SelectCombox(QComboBox*, const QString& = "");
    ~Ukui4SelectCombox() = default;

public:
    virtual void setCurrentIndex(int) override;

    virtual void setEnabled(bool) override;

    virtual bool isEnabled() const override;

    virtual int findData(const QVariant&, int = Qt::UserRole) override;

    virtual int findText(const QString&) override;

    virtual void insertItem(const QString&, const QVariant& = QVariant()) override;

    virtual void removeItem(int) override;

    virtual QVariant currentData(int = Qt::UserRole) const override;

    virtual std::list<QVariant> getDataList() const override;

    virtual int count() const override;

    virtual void clear() override;

    virtual QWidget* getWidget() override;

    virtual void setParent(QWidget*) override;

private:
    QComboBox* m_pCombox = nullptr;
};

class Ukui4SelectComboxItem : public QFrame, public ISelectComboxItem
{
    Q_OBJECT
public:
    Ukui4SelectComboxItem(std::shared_ptr<ISelectBox>, QLabel*, SelectComboxType, QWidget* = nullptr);
    ~Ukui4SelectComboxItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void setEnabled(bool) override;

    virtual QWidget* getWidget() override;

private:
    SelectComboxType m_type = SelectComboxType::SELECT_COMBOX_INPUT;
};

class Ukui5SelectCombox : public ISelectBox
{
public:
    Ukui5SelectCombox(QListWidget*, const QString& = "");
    ~Ukui5SelectCombox() = default;

public:
    virtual void setCurrentIndex(int) override;

    virtual void setEnabled(bool) override;

    virtual bool isEnabled() const override;

    virtual int findData(const QVariant&, int = Qt::UserRole) override;

    virtual int findText(const QString&) override;

    virtual void insertItem(const QString&, const QVariant& = QVariant()) override;

    virtual void removeItem(int) override;

    virtual QVariant currentData(int = Qt::UserRole) const override;

    virtual std::list<QVariant> getDataList() const override;

    virtual int count() const override;

    virtual void clear() override;

    virtual QWidget* getWidget() override;

    virtual void setParent(QWidget*) override;

private:
    void setListWidgetSelectItem(int);

private:
    QListWidget* m_pCombox = nullptr;
    std::unordered_map<int, std::shared_ptr<IDevicePortItem>> m_devicePortItemMap{};
};

class Ukui5SelectComboxItem : public QFrame, public ISelectComboxItem
{
    Q_OBJECT
public:
    Ukui5SelectComboxItem(std::shared_ptr<ISelectBox>, QLabel*, SelectComboxType, QWidget* = nullptr);
    ~Ukui5SelectComboxItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void setEnabled(bool) override;

    virtual QWidget* getWidget() override;

private:
    SelectComboxType m_type = SelectComboxType::SELECT_COMBOX_INPUT;
};

#endif // UKUI4SELECTCOMBOXITEM_H
