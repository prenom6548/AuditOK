#ifndef SEARCH_RESULT_PROPERTY_INFO_TEST_H
#define SEARCH_RESULT_PROPERTY_INFO_TEST_H

#include <QObject>

class SearchResultPropertyInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultState();
    void testKnownProperties();
    void testCopyAssignmentAndEquality();
    void testUnknownPropertyFallsBackToEmptyNames();
};

#endif // SEARCH_RESULT_PROPERTY_INFO_TEST_H
