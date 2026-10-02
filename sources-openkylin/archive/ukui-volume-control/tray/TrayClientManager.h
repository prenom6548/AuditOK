#ifndef TRAYCLIENTMANAGER_H
#define TRAYCLIENTMANAGER_H

#include <memory>
#include <unordered_map>
#include <QIcon>
#include <QVariant>

#include "ITrayVolumeSliderItem.h"
#include "IMainWindow.h"
#include "DBusClient.h"

class TrayClientManager
{
public:
    TrayClientManager(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                      std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&,
                      IMainWindow*);
    virtual ~TrayClientManager() = default;

public:
    void setValue(int, int, int);

    void setIcon(int, int, const QIcon&);

    void setCurrentIndex(int idx);

    void addDevicePortItem(int, const ClientDeviceInfo&);

    void removeDevicePortItem(const QString&, const QString&);

    void addAppItem(int, const QString&, const QString&);

    void removeAppItem(int);

    void updateDescription(const QString&, const QString&);

private:
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_volumeSliderItemMap;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_appVolumeSliderItemMap;
    IMainWindow* m_pMainWid = nullptr;
};


#endif // TRAYCLIENTMANAGER_H
