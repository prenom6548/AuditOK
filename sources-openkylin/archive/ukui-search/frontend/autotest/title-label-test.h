#ifndef TITLE_LABEL_TEST_H
#define TITLE_LABEL_TEST_H

#include <QObject>

class TitleLabelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testShowMoreVisibilityFollowsListLength();
    void testSetVisibleForcesExpandedState();
    void testSignalsForwardedFromShowMoreLabel();
    void testLabelResetCollapsesShowMoreLabel();
    void testPaintAndThemeChangeBranches();
};

#endif // TITLE_LABEL_TEST_H
