#ifndef SIGNAL_TRANSFORMER_TEST_H
#define SIGNAL_TRANSFORMER_TEST_H

#include <QObject>

class SignalTransformerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSingleton();
    void testBatchesInsertedItemsUntilFlush();
    void testSwitchingSignalTypeFlushesPreviousBatch();
    void testSwitchingFromUpdateAllUpdateAndDeleteFlushesPreviousBatch();
    void testUpdateItemsAreMergedByDesktopFile();
    void testFlushEmitsAllPendingSignalTypesInOrder();
    void testDirectTransformWithEmptyCurrentBatchDoesNotEmit();
    void testEmptyFlushDoesNotEmitSignals();
};

#endif // SIGNAL_TRANSFORMER_TEST_H
