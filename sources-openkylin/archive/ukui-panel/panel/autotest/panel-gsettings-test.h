#ifndef PANEL_GSETTINGS_TEST_H
#define PANEL_GSETTINGS_TEST_H

#include <QObject>

class PanelGSettingsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanup();
    void testInstanceAndMetaObject();
    void testConstructsQGSettingsWrapper();
    void testPanelLengthRoundTrip();
    void testSetPanelTypeWritesValue();
    void testSetIslandPositionRoutesKeys();
    void testSetTopBarSizeWritesValue();
    void testIsKeysContain();
    void testGetPanelLengthMap();
    void testInstanceThreadSafety();
    void testSchemaWithoutPanelLengthKey();
    void testDestructor();
};

#endif // PANEL_GSETTINGS_TEST_H
