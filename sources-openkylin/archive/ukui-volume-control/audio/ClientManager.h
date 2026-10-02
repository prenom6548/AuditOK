#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <unordered_map>

#include "ISwitchButtonItem.h"
#include "IVolumeSliderItem.h"
#include "IDetailSettingsItem.h"
#include "TitleLabelItem.h"
#include "ukui4.0/Ukui4SelectComboxItem.h"
#include "ukui4.0/Ukui4SwitchButtonItem.h"
#include "ukui4.0/Ukui4VolumeSliderItem.h"
#include "ISelectComboxItem.h"

class ClientManager
{
public:
    ClientManager(std::unordered_map<SwitchButtonType, ISwitchButtonItem*>&,
                  std::unordered_map<VolumeSliderType, IVolumeSliderItem*>&,
                  std::unordered_map<SelectComboxType, ISelectComboxItem*>&);
    virtual ~ClientManager() = default;

public:
    void setValue(int, int);

    void setMute(int, bool);

    void setEnabled(int, bool);

    void setBalanceEnabled(bool);

    void setRange(int, int, int);

    void setChecked(int, bool);

//    void setIcon(int, const QIcon&);

    void setCurrentIndex(int, int);

    int findData(int, const QVariant&, int = Qt::UserRole) const;

    int findText(int, const QString&) const;

    void addItem(int, const QString&, const QVariant& = QVariant());

    void removeItem(int, int);

    void clear(int);

private:
    std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& m_switchBtnItemMap;
    std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& m_volumeSliderItemMap;
    std::unordered_map<SelectComboxType, ISelectComboxItem*>& m_selectComboxItemMap;
};

#endif // CLIENTMANAGER_H
