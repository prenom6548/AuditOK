#ifndef IDETAILSETTINGSITEM_H
#define IDETAILSETTINGSITEM_H

#include <memory>
#include <QWidget>
#include <QLabel>

/**
* @brief 包含DetailSettings子项的类型
*/
enum class DetailSettingType
{
    DETAIL_SETTING_DEVMANAGER = 0,  // 设备管理子项
    DETAIL_SETTING_APPMANAGER,      // 应用声音子项
};

class IDetailSettings {
public:
    virtual QWidget* getWidget() = 0;
};


class IDetailSettingsItem
{
public:
    IDetailSettingsItem(std::shared_ptr<IDetailSettings>, QLabel*);
    virtual ~IDetailSettingsItem() = default;

public:
    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual QWidget* getWidget() = 0;

    std::shared_ptr<IDetailSettings> getDetailSettings() const;

protected:
    std::shared_ptr<IDetailSettings> m_pDetailSettings = nullptr;
    QLabel* m_pDisplayLabel = nullptr;
};

#endif // IDETAILSETTINGSITEM_H
