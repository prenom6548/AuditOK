#ifndef CGROUP_HELPER_TEST_H
#define CGROUP_HELPER_TEST_H

#include <QObject>

class CgroupHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSetServiceCpuLimitFailsWithoutProcessManager();
    void testUserPathV2Resolution();
    void testUserPathV2IgnoresNonUserEntriesAndRecurses();
    void testUserPathV2AcceptsServiceAndSliceNameVariants();
    void testInvalidDbusInterfaceFailures();
    void testCgroupVersionAndPathAreStable();
    void testUniquePtrDeleterAcceptsNullAndObject();
    void testAddProcessManagerWhiteListHandlesExistingAndNewEntries();
    void testServiceCpuLimitAndWhitelistFailSafelyWithoutDbus();
};

#endif // CGROUP_HELPER_TEST_H
