#include "test_mylittleparse.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QTemporaryFile>

static void writeFileContent(const QString& path, const QString& content)
{
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    QTextStream out(&f);
    out << content;
    f.close();
}

static QString readFileContent(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream in(&f);
    QString s = in.readAll();
    f.close();
    return s;
}

void TestMyLittleParse::initTestCase()
{
    // 使用临时目录，测试用例中在 init() 里创建具体文件
}

void TestMyLittleParse::cleanupTestCase()
{
}

void TestMyLittleParse::init()
{
    // 每个测试如需固定初始内容，在测试内自行写文件
}

// ---- add(key, value, e) ----

void TestMyLittleParse::testAdd_keyNotExists_appendAndSuccess()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.add("k1", "v1", e);

    QVERIFY(ok);
    QCOMPARE(e, MyLittleParse::SUCCESS);
    QCOMPARE(readFileContent(path).trimmed(), QString("k1=v1"));
}

void TestMyLittleParse::testAdd_keyExistsWithNonEmptyValue_returnsKeyExits()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k1=v1\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.add("k1", "v2", e);

    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::KEY_EXITS);
    QCOMPARE(readFileContent(path).trimmed(), QString("k1=v1"));
}

void TestMyLittleParse::testAdd_keyExistsWithEmptyValue_callsModAndSuccess()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k1=\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.add("k1", "newval", e);

    QVERIFY(ok);
    QCOMPARE(e, MyLittleParse::SUCCESS);
    QString content = readFileContent(path);
    QVERIFY(content.contains("k1=newval"));
}

void TestMyLittleParse::testAdd_openFileFail_returnsOpenFileFail()
{
    QTemporaryDir dir;
    QString path = dir.path(); // 目录路径，无法以 ReadWrite 打开为普通文件
    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.add("k", "v", e);

    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::OPEN_FILE_FAIL);
}

// ---- Add(key, value) ----

void TestMyLittleParse::testAddCapital_keyNotExists_success()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "");

    MyLittleParse p(path);
    bool ok = p.Add("key", "val");
    QVERIFY(ok);
    QCOMPARE(readFileContent(path).trimmed(), QString("key=val"));
}

void TestMyLittleParse::testAddCapital_keyExists_overwritesViaMod()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "key=old\n");

    MyLittleParse p(path);
    bool ok = p.Add("key", "new");
    QVERIFY(ok);
    QString content = readFileContent(path);
    QVERIFY(content.contains("key=new"));
}

// ---- add 重载 ----

void TestMyLittleParse::testAdd_twoArgs_usesMemberErrorEnum()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "");

    MyLittleParse p(path);
    bool ok = p.add("a", "b");
    QVERIFY(ok);
    QCOMPARE(p.getErrEnum(), MyLittleParse::SUCCESS);
}

void TestMyLittleParse::testAdd_rvalueRef_forwardsToLvalueAdd()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "");

    MyLittleParse p(path);
    bool ok = p.add(QString("x"), QString("y"));
    QVERIFY(ok);
    QCOMPARE(readFileContent(path).trimmed(), QString("x=y"));
}

// ---- del(key, e) ----

void TestMyLittleParse::testDel_keyNotExists_returnsFalse()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "a=1\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.del("b", e);

    QVERIFY(!ok);
    QCOMPARE(readFileContent(path).trimmed(), QString("a=1"));
}

void TestMyLittleParse::testDel_openFileFail_returnsOpenFileFail()
{
    QTemporaryDir dir;
    QString path = dir.path();
    writeFileContent(path + "/dummy", ""); // 确保目录存在
    MyLittleParse p(path);                 // 用目录路径，del 里 find_only 会先 open 文件
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.del("any", e);
    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::OPEN_FILE_FAIL);
}

void TestMyLittleParse::testDel_keyExists_removesLineAndSuccess()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "a=1\nb=2\nc=3\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.del("b", e);

    QVERIFY(ok);
    QCOMPARE(e, MyLittleParse::SUCCESS);
    QString content = readFileContent(path);
    QVERIFY(!content.contains("b=2"));
    QVERIFY(content.contains("a=1"));
    QVERIFY(content.contains("c=3"));
}

void TestMyLittleParse::testDel_skipsEmptyLines()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "x=1\n\n\ny=2\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.del("x", e);
    QVERIFY(ok);
    QString content = readFileContent(path);
    QVERIFY(content.contains("y=2"));
    QVERIFY(!content.contains("x=1"));
}

void TestMyLittleParse::testDel_skipsMatchingKeyOnly()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k=v1\nk=v2\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.del("k", e);
    QVERIFY(ok);
    // 当前实现会删除所有 key 为 k 的行
    QString content = readFileContent(path);
    QVERIFY(!content.contains("k=v1"));
    QVERIFY(!content.contains("k=v2"));
}

void TestMyLittleParse::testDel_twoArgs_usesMemberErrorEnum()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "a=1\n");

    MyLittleParse p(path);
    bool ok = p.del("a");
    QVERIFY(ok);
    QCOMPARE(p.getErrEnum(), MyLittleParse::SUCCESS);
}

void TestMyLittleParse::testDel_rvalueRef_forwardsToLvalueDel()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "r=1\n");

    MyLittleParse p(path);
    bool ok = p.del(QString("r"));
    QVERIFY(ok);
    QVERIFY(!readFileContent(path).contains("r=1"));
}

// ---- mod(key, value, e) ----

void TestMyLittleParse::testMod_keyNotExists_returnsFalse()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "a=1\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.mod("b", "2", e);
    QVERIFY(!ok);
    QCOMPARE(readFileContent(path).trimmed(), QString("a=1"));
}

void TestMyLittleParse::testMod_openFileFail_returnsOpenFileFail()
{
    QTemporaryDir dir;
    QString path = dir.path();
    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.mod("k", "v", e);
    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::OPEN_FILE_FAIL);
}

void TestMyLittleParse::testMod_sameValue_returnsRepeat()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k=same\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.mod("k", "same", e);
    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::REPEAT);
    QCOMPARE(readFileContent(path).trimmed(), QString("k=same"));
}

void TestMyLittleParse::testMod_differentValue_updatesAndSuccess()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k=old\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.mod("k", "new", e);
    QVERIFY(ok);
    QCOMPARE(e, MyLittleParse::SUCCESS);
    QVERIFY(readFileContent(path).contains("k=new"));
}

// ---- Mod(key, value) ----

void TestMyLittleParse::testModCapital_sameValue_returnsTrue()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k=val\n");

    MyLittleParse p(path);
    bool ok = p.Mod("k", "val");
    QVERIFY(ok);
}

void TestMyLittleParse::testModCapital_differentValue_returnsModResult()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k=old\n");

    MyLittleParse p(path);
    bool ok = p.Mod("k", "new");
    QVERIFY(ok);
    QVERIFY(readFileContent(path).contains("k=new"));
}

void TestMyLittleParse::testMod_rvalueRef_forwardsToLvalueMod()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k=v\n");

    MyLittleParse p(path);
    bool ok = p.mod(QString("k"), QString("v2"));
    QVERIFY(ok);
    QVERIFY(readFileContent(path).contains("k=v2"));
}

// ---- find(key, value, e) ----

void TestMyLittleParse::testFind_openFileFail_returnsFalse()
{
    QTemporaryDir dir;
    QString path = dir.path();
    MyLittleParse p(path);
    QString value;
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.find("k", value, e);
    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::OPEN_FILE_FAIL);
}

void TestMyLittleParse::testFind_keyNotFound_returnsKeyNoexits()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "a=1\n");

    MyLittleParse p(path);
    QString value;
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.find("b", value, e);
    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::KEY_NOEXITS);
}

void TestMyLittleParse::testFind_emptyLine_skipped()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "\n\nkey=val\n\n");

    MyLittleParse p(path);
    QString value;
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.find("key", value, e);
    QVERIFY(ok);
    QCOMPARE(value, QString("val"));
    QCOMPARE(e, MyLittleParse::SUCCESS);
}

void TestMyLittleParse::testFind_keyFound_fillsValueAndSuccess()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "k1=v1\nk2=v2\n");

    MyLittleParse p(path);
    QString value;
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.find("k2", value, e);
    QVERIFY(ok);
    QCOMPARE(value, QString("v2"));
    QCOMPARE(e, MyLittleParse::SUCCESS);
}

void TestMyLittleParse::testFind_twoArgs_usesMemberErrorEnum()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "a=1\n");

    MyLittleParse p(path);
    QString value;
    bool ok = p.find("a", value);
    QVERIFY(ok);
    QCOMPARE(value, QString("1"));
    QCOMPARE(p.getErrEnum(), MyLittleParse::SUCCESS);
}

void TestMyLittleParse::testFindOnly_keyExists_returnsTrue()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "x=y\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.find_only("x", e);
    QVERIFY(ok);
}

void TestMyLittleParse::testFindOnly_keyNotExists_returnsFalse()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "a=1\n");

    MyLittleParse p(path);
    MyLittleParse::error_enum e = MyLittleParse::DEFAULT;
    bool ok = p.find_only("b", e);
    QVERIFY(!ok);
    QCOMPARE(e, MyLittleParse::KEY_NOEXITS);
}

void TestMyLittleParse::testFindOnly_rvalueRef_forwardsToLvalue()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "r=s\n");

    MyLittleParse p(path);
    bool ok = p.find_only(QString("r"));
    QVERIFY(ok);
}

// ---- getErrEnum / getErrMsg ----

void TestMyLittleParse::testGetErrEnum_afterAddSuccess_returnsSuccess()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();
    writeFileContent(path, "");

    MyLittleParse p(path);
    p.add("a", "b");
    QCOMPARE(p.getErrEnum(), MyLittleParse::SUCCESS);
}

void TestMyLittleParse::testGetErrMsg_allErrorEnumValues()
{
    QTemporaryFile tmp;
    QVERIFY(tmp.open());
    QString path = tmp.fileName();
    tmp.close();

    // DEFAULT: 构造后未操作
    writeFileContent(path, "");
    MyLittleParse p0(path);
    QCOMPARE(p0.getErrMsg(), QString("default"));

    // KEY_EXITS
    writeFileContent(path, "k=v\n");
    MyLittleParse p1(path);
    p1.add("k", "x");
    QCOMPARE(p1.getErrMsg(), QString("KEY_EXITS"));

    // OPEN_FILE_FAIL: 用目录路径触发打开失败
    QTemporaryDir dir;
    MyLittleParse p2(dir.path());
    QString dummy;
    p2.find("k", dummy);
    QCOMPARE(p2.getErrMsg(), QString("OPEN_FILE_FAIL"));

    // KEY_NOEXITS
    writeFileContent(path, "a=1\n");
    MyLittleParse p3(path);
    QString v;
    p3.find("nonexistent", v);
    QCOMPARE(p3.getErrMsg(), QString("KEY_NOEXITS"));

    // REPEAT
    writeFileContent(path, "k=val\n");
    MyLittleParse p4(path);
    MyLittleParse::error_enum e;
    p4.mod("k", "val", e);
    QCOMPARE(p4.getErrMsg(), QString("REPEAT"));

    // SUCCESS
    writeFileContent(path, "");
    MyLittleParse p5(path);
    p5.add("a", "b");
    QCOMPARE(p5.getErrMsg(), QString("SUCCESS"));
}
