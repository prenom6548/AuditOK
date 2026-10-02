#ifndef FLOW_LAYOUT_TEST_H
#define FLOW_LAYOUT_TEST_H

#include <QObject>

class FlowLayoutTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testInitialStateAndSpacing();
    void testAddItemTakeAtAndMinimumSize();
    void testHeightForWidthWrapsItems();
    void testSetGeometryPositionsWrappedItems();
    void testSmartSpacingNullParent();
    void testSmartSpacingWidgetParent();
    void testDoLayoutWithNegativeSpacing();
};

#endif // FLOW_LAYOUT_TEST_H
