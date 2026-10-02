#ifndef IDETAILSETTINGS_H
#define IDETAILSETTINGS_H

/**
* @brief 包含DetailSettings子项的类型
*/
enum class DetailSettingType
{
    DETAIL_SETTING_DEVMANAGER = 0,  // 设备管理子项
    DETAIL_SETTING_APPMANAGER,      // 应用声音子项
};

class IDetailSettings
{
public:
    IDetailSettings();
    virtual ~IDetailSettings() = default;
};

#endif // IDETAILSETTINGS_H
