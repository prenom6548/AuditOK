#ifndef ONE_RESULT_TEST_H
#define ONE_RESULT_TEST_H

#include <QObject>

class OneResultTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultShowToolTipIsTrue();
    void testSetShowToolTip();
    void testInfoKeepsInputFields();
};

#endif // ONE_RESULT_TEST_H
