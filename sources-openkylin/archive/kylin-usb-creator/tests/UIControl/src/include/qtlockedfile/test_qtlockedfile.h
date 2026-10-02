#ifndef TEST_QTLOCKEDFILE_H
#define TEST_QTLOCKEDFILE_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryFile>
#include "../../../../../UIControl/src/include/qtlockedfile.h"

using namespace QtLP_Private;

class TestQtLockedFile : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDefaultConstructor();
    void testNamedConstructor();
    void testOpenWithTruncate();
    void testOpenWithoutTruncate();
    void testLockUnlock();
    void testIsLocked();
    void testLockMode();
    void testDestructor();

private:
    QtLockedFile* m_lockedFile;
    QTemporaryFile* m_tempFile;
};

#endif // TEST_QTLOCKEDFILE_H