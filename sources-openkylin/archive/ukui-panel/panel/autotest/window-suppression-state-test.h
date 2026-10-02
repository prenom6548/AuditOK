#ifndef WINDOW_SUPPRESSION_STATE_TEST_H
#define WINDOW_SUPPRESSION_STATE_TEST_H

#include <QObject>

class WindowSuppressionStateTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultStateNotSuppressed();
    void testScreenLockDoesNotClearControlHidden();
    void testScreenLockDoesNotClearShellHidden();
    void testClearingLastSuppressorRestoresVisibility();
};

#endif // WINDOW_SUPPRESSION_STATE_TEST_H
