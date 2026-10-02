#ifndef AREATEST_H
#define AREATEST_H

#include "../../registeredSession/area/areainterface.h"
#include <QtTest>
#include <QCoreApplication>

class AreaTest : public QObject
{
    Q_OBJECT

public:
    AreaTest();
    ~AreaTest();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testFormatCountry() const;
    void testCalendar() const;
    void testFirstDay() const;
    void testDateFormat() const;
    void testTimeFormat() const;
    void testShowLanguageList() const;
    void testLanguage() const;

private:
    AreaInterface *area            = nullptr;
    QString       formatCountry    = "";
    QString       calendar         = "";
    QString       firstDay         = "";
    QString       dateFormat       = "";
    QString       timeFormat       = "";
    QStringList   showLanguageList = {};
    QString       language         = "";
};

#endif // AREATEST_H
