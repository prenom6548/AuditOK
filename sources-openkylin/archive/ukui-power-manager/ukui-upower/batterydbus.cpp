#include "batterydbus.h"

BatteryDBus::BatteryDBus()
{
    batteryinfo = new BatteryInfo;
    connect(batteryinfo, &BatteryInfo::LowBatteryChanged, this, &BatteryDBus::dealLowBattery);
    connect(batteryinfo, &BatteryInfo::VeryLowBatteryChanged, this, &BatteryDBus::dealVeryLowBattery);
    connect(batteryinfo, &BatteryInfo::BatteryIconChanged, this, &BatteryDBus::dealBatteryIcon);
    connect(batteryinfo, &BatteryInfo::PercentageChanged, this, &BatteryDBus::PercentageChanged);
}

BatteryDBus::~BatteryDBus()
{
    delete batteryinfo;
}

QString BatteryDBus::IconName()
{
    showServiceMsg();
    return batteryinfo->IconName();
}

double BatteryDBus::Percentage()
{
    showServiceMsg();
    return batteryinfo->Percentage();
}

int BatteryDBus::TimeToFull()
{
    showServiceMsg();
    return batteryinfo->TimeToFull();
}

int BatteryDBus::TimeToEmpty()
{
    showServiceMsg();
    return batteryinfo->TimeToEmpty();
}

bool BatteryDBus::IsPresent()
{
    showServiceMsg();
    return batteryinfo->IsPresent();
}

int BatteryDBus::BatteryState()
{
    showServiceMsg();
    return batteryinfo->getBatteryState();
}

bool BatteryDBus::LowBattery()
{
    showServiceMsg();
    return batteryinfo->LowBatteryState();
}

void BatteryDBus::dealLowBattery(bool state)
{
    emit LowBatteryState(state);
}

void BatteryDBus::dealVeryLowBattery(bool state)
{
    emit VeryLowBatteryState(state);
}

void BatteryDBus::dealBatteryIcon(QString msg)
{
    emit BatteryIcon(msg);
}

QString BatteryDBus::GetBatteryTooltip()
{
    showServiceMsg();
    return m_tooltip;
}

void BatteryDBus::SetBatteryTooltip(const QString &tooltip)
{
    m_tooltip = tooltip;
    emit BatteryTooltip(m_tooltip);
}

void BatteryDBus::SetShowIcon(const bool &show)
{
    m_showIcon = show;
    emit BatteryShowIcon(m_showIcon);
}

bool BatteryDBus::ShowIcon(){
    showServiceMsg();
    return m_showIcon;
}