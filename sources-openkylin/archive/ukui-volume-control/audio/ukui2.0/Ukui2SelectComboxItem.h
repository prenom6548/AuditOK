#ifndef UKUI2SELECTCOMBOXITEM_H
#define UKUI2SELECTCOMBOXITEM_H

#include <QComboBox>

#include "ISelectComboxItem.h"

///**
//* @brief 包含SelectCombox子项的类型
//*/
//enum class SelectComboxType
//{
//    SELECT_COMBOX_OUTPUT = 0, // 输出设备子项
//    SELECT_COMBOX_INPUT       // 输入设备子项
//};

class Ukui2SelectCombox : public ISelectBox
{
public:
    Ukui2SelectCombox(QComboBox*);
    ~Ukui2SelectCombox() = default;

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

class Ukui2SelectComboxItem : public QFrame, public ISelectComboxItem
{
public:
    Ukui2SelectComboxItem(std::shared_ptr<ISelectBox>, QLabel*, QWidget* = nullptr);
    ~Ukui2SelectComboxItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void setEnabled(bool) override;

    virtual QWidget* getWidget() override;

private Q_SLOTS:
    void currentIndexChangedSlots(int);
};

#endif // UKUI2SELECTCOMBOXITEM_H
