#ifndef ACTION_LABEL_TEST_H
#define ACTION_LABEL_TEST_H

#include <QObject>

class ActionLabelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testTextAndCursorInitialized();
    void testMouseReleaseEmitsActionTriggered();
};

#endif // ACTION_LABEL_TEST_H
