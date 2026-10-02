#ifndef SHOW_MORE_LABEL_TEST_H
#define SHOW_MORE_LABEL_TEST_H

#include <QObject>

class ShowMoreLabelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSetAndResetExpandedState();
    void testMouseClickTogglesAndEmitsSignals();
    void testEventFilterIgnoresNonIconAndTimerActiveBranches();
};

#endif // SHOW_MORE_LABEL_TEST_H
