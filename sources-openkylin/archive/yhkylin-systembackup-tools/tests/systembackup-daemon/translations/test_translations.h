#ifndef TEST_TRANSLATIONS_H
#define TEST_TRANSLATIONS_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/translations.h"

class TestTranslations : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testTrWithZhCNLocale();
    void testTrWithZhHKLocale();
    void testTrWithBoCNLocale();
    void testTrWithMnMNLocale();
    void testTrWithkyKGLocale();
    void testTrWithkkKZLocale();
    void testTrWithugCNLocale();
    void testTrWithviVNLocale();
    void testTrWithmsMYLocale();
    void testTrWiththTHLocale();
    void testTrWithUnknownLocale();
    void testSetLocale();
};

#endif // TEST_TRANSLATIONS_H
