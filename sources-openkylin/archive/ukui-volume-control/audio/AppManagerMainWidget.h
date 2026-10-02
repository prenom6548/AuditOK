#ifndef APPMANAGERMAINWIDGET_H
#define APPMANAGERMAINWIDGET_H

#include <QWidget>
#include <kwidget.h>
#include <knavigationbar.h>
#include <QStackedWidget>

#include "AppManagerItemWidget.h"
#include "DBusClient.h"

class AppManagerMainWidget : public kdk::KWidget
{
    Q_OBJECT
public:
    AppManagerMainWidget(std::list<AppManagerItem>&, QWidget* = nullptr);
    ~AppManagerMainWidget() = default;

public:
    void addItem(int, int, int, bool, const QString&, const QString&, const QList<ClientDeviceInfo>&, const QList<ClientDeviceInfo>&);

    void removeItem(const QString&);

    void setValue(const QString&, int);

    void setMute(const QString&, bool);

    void setCurrentItem(int);

    void setCurrentIndex(const QString&, const NodeType&, int);

    void addPortToAppItem(const QString&, const NodeType&, const QString&, const QVariant&);

    void setAppCbxEnabled(const QString&, const NodeType&, bool);

    void updateOutputVolumeControls(const QString& name, bool enable, int value, bool muted);

    void removeProtFromAppItem(const QString&, const NodeType&, int);

    std::list<AppManagerItem>& getAppItems();

    AppManagerItemWidget* getAppWidgetByIndex(int);

Q_SIGNALS:
    void valueChangedSignals(int, int);

    void muteChangedSignals(int);

    void currentIndexChangedSignals(const NodeType&, int, const QString&, const QString&);

    void closeWindowSignals();

    void hideWindowSignals();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    void initUi();

    void initSlots();

private:
    kdk::KNavigationBar* m_pNavigationBar {};
    QStackedWidget* m_pStackedWidget {};
    std::map<QString, QStandardItem*> m_itemMap {};
    std::list<AppManagerItem>& m_appWidgetList;
    QStringList m_blackList {
        "sound-theme-player",
        "Remapped Stream",
        "ukui-control-center",
        "Echo-Cancel Sink Stream",
        "Echo-Cancel Source Stream",
        "kylin-device-daemon",
        "pacat",
        "ukui-powermanagement",
        "kylin-kmre-audio",
        "ukui-audio-service"
    };
};

#endif // APPMANAGERMAINWIDGET_H
