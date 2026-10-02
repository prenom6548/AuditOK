#ifndef APPMANAGERSELECTCOMBOXITEM_H
#define APPMANAGERSELECTCOMBOXITEM_H

#include <QHBoxLayout>

#include "ISelectComboxItem.h"

class AppManagerSelectComboxItem : public QFrame, public ISelectComboxItem
{
    Q_OBJECT
public:
    AppManagerSelectComboxItem(std::shared_ptr<ISelectBox>, QLabel*, QWidget* = nullptr);
    ~AppManagerSelectComboxItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void setEnabled(bool) override;

    virtual QWidget* getWidget() override;
};

class AppSelectComboxWithDescItem : public QFrame, public ISelectComboxItem
{   
    Q_OBJECT
public:
    AppSelectComboxWithDescItem(std::shared_ptr<ISelectBox>, QLabel*, QLabel*, QWidget* = nullptr);
    ~AppSelectComboxWithDescItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void setEnabled(bool) override;

    virtual QWidget* getWidget() override;

private:
    QLabel* m_pHintLabel {};
    QLabel* m_pHintIconLabel {};
    QVBoxLayout* m_pVLayout {};
    QHBoxLayout* m_pHintHLayout {};
};

#endif // APPMANAGERSELECTCOMBOXITEM_H
