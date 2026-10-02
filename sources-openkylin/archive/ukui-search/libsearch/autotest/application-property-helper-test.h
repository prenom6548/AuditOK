#ifndef APPLICATION_PROPERTY_HELPER_TEST_H
#define APPLICATION_PROPERTY_HELPER_TEST_H

#include <QObject>

class ApplicationPropertyHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultConstructorAndUnknownConversion();
    void testPropertyMappings_data();
    void testPropertyMappings();
    void testCopyAndAssignment();
};

#endif // APPLICATION_PROPERTY_HELPER_TEST_H
