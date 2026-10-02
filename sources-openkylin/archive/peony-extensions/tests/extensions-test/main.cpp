#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QStandardPaths>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("peony-extensions-unit-test"));
    QCoreApplication::setApplicationName(QStringLiteral("extensions-test"));
    return RUN_ALL_TESTS();
}
