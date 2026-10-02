#ifndef __TEST_H__
#define __TEST_H__

#include <QObject>
#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QCoreApplication>
#include <QtTest/QtTest>

/*unit test param*/
#define TEST_PRINT_LOG  1
#define TEST_INTERVAL   1
/**/

#include "../src/panel_config.h"

class PanelConfigTest : public QObject
{
    Q_OBJECT
private:
    ConfigUkPanel *m_panelConfig; // 替换为你的实际类名
 
public:
    PanelConfigTest();
    ~PanelConfigTest();

    void testConnect();
private slots:
    void testSetMergeIcons();

    /**
     * @brief testSetPanelLocation
     * panel had bug
     */
    void testSetPanelLocation();

    void testSetPanelSize();

    void testSetAuotHide();

    void testSetLock();

    void testSetOnAllscreens();

    void testsetShowSystemTryOnAllPanels();

    void testsetTaskBarIconsShowedOn();
private:
    void initEnum();

private:
    QMetaEnum   m_mergeEnum;
    QMetaEnum   m_locationEnum;
    QMetaEnum   m_sizeEnum;
    QMetaEnum   m_taskBarShowOnEnum;

};


#endif // TEST_H
