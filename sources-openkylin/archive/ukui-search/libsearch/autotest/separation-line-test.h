#ifndef SEPARATION_LINE_TEST_H
#define SEPARATION_LINE_TEST_H

#include <QObject>

class SeparationLineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInitialGeometryAndStyle();
    void testSetLineStyleKeepsStyleSheetValid();
    void testPaletteChangedTriggersLineStyleUpdate();
};

#endif // SEPARATION_LINE_TEST_H
