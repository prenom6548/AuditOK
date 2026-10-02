#ifndef ACTION_TRANSMITER_TEST_H
#define ACTION_TRANSMITER_TEST_H

#include <QObject>

class ActionTransmiterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSingletonIdentity();
    void testInvokeHideUiAction();
    void testInvokeNoneActionDoesNotEmit();
};

#endif // ACTION_TRANSMITER_TEST_H
