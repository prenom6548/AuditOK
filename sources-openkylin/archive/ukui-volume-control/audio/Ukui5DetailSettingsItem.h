#ifndef UKUI5DETAILSETTINGSITEM_H
#define UKUI5DETAILSETTINGSITEM_H

#include <QPushButton>

#include "IDetailSettingsItem.h"

class Ukui5DetailSettings : public IDetailSettings
{
public:
    Ukui5DetailSettings(QPushButton*, const QString& = "");
    ~Ukui5DetailSettings() = default;

public:
    virtual QWidget* getWidget() override;

private:
    QPushButton* m_pSettingBtn;
};

class Ukui5DetailSettingsItem : public QFrame, public IDetailSettingsItem
{
public:
    Ukui5DetailSettingsItem(std::shared_ptr<IDetailSettings>, QLabel*, QWidget* = nullptr);

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;
};

#endif // UKUI5DETAILSETTINGSITEM_H
