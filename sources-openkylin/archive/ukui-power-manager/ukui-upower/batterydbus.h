#ifndef BATTERYDBUS_H
#define BATTERYDBUS_H

#include <dbus.h>
#include <batteryinfo/batteryinfo.h>
class BatteryDBus : public DBus
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.ukui.upower.battery")
public:
    BatteryDBus();

    ~BatteryDBus();

public slots:

    /**
     * @brief IconName
     * @return
     * 图标名称
     */
    QString IconName();

    /**
     * @brief Percentage
     * @return
     * 电量百分比
     */
    double Percentage();

    /**
     * @brief TimeToFull
     * @return
     * 充电过程中，还有多久充满电
     */
    int TimeToFull();

    /**
     * @brief TimeToEmpty
     * @return
     * 放电过程 还有多久耗尽
     */
    int TimeToEmpty();

    /**
     * @brief IsPresent
     * 电池是否存在
     * @return
     */
    bool IsPresent();

    /**
     * @brief BatteryState
     * 电池状态
     * @return
     */
    int BatteryState();

    /**
     * @brief LowBattery
     * 低电量
     * @return
     */
    bool LowBattery();
    bool ShowIcon();
    QString GetBatteryTooltip();
    void SetBatteryTooltip(const QString &tooltip);
    void SetShowIcon(const bool &show);
signals:

    /**
     * @brief lowBatteryState
     * @return
     * 低电量状态信号
     */
    void LowBatteryState(bool);

    /**
     * @brief VeryLowBatteryState
     */
    void VeryLowBatteryState(bool);

    void BatteryIcon(QString);

    void BatteryTooltip(QString);

    void BatteryShowIcon(bool);

    void PercentageChanged(double);

private:
    BatteryInfo *batteryinfo;

    QString m_tooltip;

    bool m_showIcon = false;

    void dealLowBattery(bool);

    void dealVeryLowBattery(bool);

    void dealBatteryIcon(QString);
};

#endif // BATTERYDBUS_H
