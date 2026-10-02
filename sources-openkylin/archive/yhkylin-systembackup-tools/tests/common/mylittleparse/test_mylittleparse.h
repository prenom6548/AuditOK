#ifndef TEST_MYLITTLEPARSE_H
#define TEST_MYLITTLEPARSE_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../common/mylittleparse.h"

class TestMyLittleParse : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    // add(key, value, e) 分支覆盖
    void testAdd_keyNotExists_appendAndSuccess();
    void testAdd_keyExistsWithNonEmptyValue_returnsKeyExits();
    void testAdd_keyExistsWithEmptyValue_callsModAndSuccess();
    void testAdd_openFileFail_returnsOpenFileFail();

    // Add(key, value) 分支覆盖
    void testAddCapital_keyNotExists_success();
    void testAddCapital_keyExists_overwritesViaMod();

    // add 重载
    void testAdd_twoArgs_usesMemberErrorEnum();
    void testAdd_rvalueRef_forwardsToLvalueAdd();

    // del(key, e) 分支覆盖
    void testDel_keyNotExists_returnsFalse();
    void testDel_openFileFail_returnsOpenFileFail();
    void testDel_keyExists_removesLineAndSuccess();
    void testDel_skipsEmptyLines();
    void testDel_skipsMatchingKeyOnly();

    // del 重载
    void testDel_twoArgs_usesMemberErrorEnum();
    void testDel_rvalueRef_forwardsToLvalueDel();

    // mod(key, value, e) 分支覆盖
    void testMod_keyNotExists_returnsFalse();
    void testMod_openFileFail_returnsOpenFileFail();
    void testMod_sameValue_returnsRepeat();
    void testMod_differentValue_updatesAndSuccess();

    // Mod(key, value) 分支覆盖
    void testModCapital_sameValue_returnsTrue();
    void testModCapital_differentValue_returnsModResult();

    // mod 重载
    void testMod_rvalueRef_forwardsToLvalueMod();

    // find(key, value, e) 分支覆盖
    void testFind_openFileFail_returnsFalse();
    void testFind_keyNotFound_returnsKeyNoexits();
    void testFind_emptyLine_skipped();
    void testFind_keyFound_fillsValueAndSuccess();

    // find / find_only 重载
    void testFind_twoArgs_usesMemberErrorEnum();
    void testFindOnly_keyExists_returnsTrue();
    void testFindOnly_keyNotExists_returnsFalse();
    void testFindOnly_rvalueRef_forwardsToLvalue();

    // getErrEnum / getErrMsg
    void testGetErrEnum_afterAddSuccess_returnsSuccess();
    void testGetErrMsg_allErrorEnumValues();
};

#endif // TEST_MYLITTLEPARSE_H
