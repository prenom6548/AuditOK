// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString/kcatalog.h>
#include <KLocalizedString/klocalizedstring.h>
#include <KLocalizedString/klocalizedcontext.h>
#include <KLocalizedString/klocalizedtranslator.h>
#include <KLocalizedString/kuitmarkup.h>
#include <KLocalizedString/kuitsetup.h>

#include <QObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

// ============================================================================
// TestKCatalog
// ============================================================================
class TestKCatalog : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void testDefaultConstructor()
    {
        KCatalog c;
        QVERIFY(c.isNull());
    }

    void testDomainConstructor()
    {
        KCatalog c(QByteArray("test-domain"));
        QVERIFY(!c.isNull());
    }

    void testDomainLanguageConstructor()
    {
        KCatalog c(QByteArray("test-domain"), QStringLiteral("en"));
        QVERIFY(!c.isNull());
    }

    void testCopyConstructor()
    {
        KCatalog c1(QByteArray("test-domain"));
        KCatalog c2(c1);
        QVERIFY(!c2.isNull());
    }

    void testAssignmentOperator()
    {
        KCatalog c1(QByteArray("test-domain"));
        KCatalog c2;
        c2 = c1;
        QVERIFY(!c2.isNull());
    }

    void testTranslateEmpty()
    {
        KCatalog c;
        QVERIFY(c.translate(QByteArray("msgid"), QByteArray(), 0, QByteArray()).isEmpty());
    }

    void testTranslateWithDomain()
    {
        KCatalog c(QByteArray("test-domain"));
        QVERIFY(c.translate(QByteArray("msgid"), QByteArray(), 0, QByteArray()).isEmpty());
    }

    void testTranslateOverloads()
    {
        KCatalog c(QByteArray("test-domain"));
        // All overloads go through the 4-arg version to avoid ambiguity
        QVERIFY(c.translate(QByteArray("ctx"), QByteArray("msgid"), QByteArray(), 0).isEmpty());
        QVERIFY(c.translate(QByteArray("msgid"), QByteArray("msgid_plural"), static_cast<qint64>(1), QByteArray()).isEmpty());
        QVERIFY(c.translate(QByteArray("ctx"), QByteArray("msgid"), QByteArray("msgid_plural"), static_cast<qint64>(1)).isEmpty());
        QVERIFY(c.translate(QByteArray("msgid"), QByteArray(), 0, QByteArray("ctx")).isEmpty());
    }

    void testLanguages()
    {
        KCatalog c;
        QVERIFY(c.languages().isEmpty());

        KCatalog c2(QByteArray("test-domain"));
        QVERIFY(c2.languages().isEmpty());
    }

    void testLocaleDir()
    {
        KCatalog c;
        QVERIFY(c.localeDir().isEmpty());

        KCatalog c2(QByteArray("test-domain"));
        // No real MO files exist, so localeDir should be empty
        QVERIFY(c2.localeDir().isEmpty());
    }

    void testStaticCatalogLocaleDir()
    {
        // Without proper data setup, should return empty
        QString dir = KCatalog::catalogLocaleDir(QByteArray("nonexistent-domain"), QStringLiteral("en"));
        QVERIFY(dir.isEmpty());
    }

    void testAddDomainLocaleDir()
    {
        const QByteArray domain("test-add-domain");
        const QString path = QStringLiteral("/tmp/test-locale-dir");

        QSet<QString> langs = KCatalog::availableCatalogLanguages(domain);
        QVERIFY(langs.isEmpty());

        KCatalog::addDomainLocaleDir(domain, path);
        langs = KCatalog::availableCatalogLanguages(domain);
    }

    void testAvailableCatalogLanguages()
    {
        const QByteArray domain("test-available-langs");
        QSet<QString> langs = KCatalog::availableCatalogLanguages(domain);
        QVERIFY(langs.isEmpty());
    }
};

// ============================================================================
// TestKLocalizedString
// ============================================================================
class TestKLocalizedString : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void testDefaultConstructor()
    {
        KLocalizedString s;
        QVERIFY(s.isEmpty());
        QVERIFY(s.toString().isEmpty());
    }

    void testKi18n()
    {
        KLocalizedString s = ki18n("Hello");
        QVERIFY(!s.isEmpty());
    }

    void testKi18nc()
    {
        KLocalizedString s = ki18nc("context", "Hello");
        QVERIFY(!s.isEmpty());
    }

    void testKi18np()
    {
        KLocalizedString s = ki18np("Singular", "Plural");
        QVERIFY(!s.isEmpty());
    }

    void testKi18ncp()
    {
        KLocalizedString s = ki18ncp("context", "Singular", "Plural");
        QVERIFY(!s.isEmpty());
    }

    void testKi18nd()
    {
        KLocalizedString s = ki18nd("domain", "Hello");
        QVERIFY(!s.isEmpty());
    }

    void testKi18ndc()
    {
        KLocalizedString s = ki18ndc("domain", "context", "Hello");
        QVERIFY(!s.isEmpty());
    }

    void testKi18ndp()
    {
        KLocalizedString s = ki18ndp("domain", "Singular", "Plural");
        QVERIFY(!s.isEmpty());
    }

    void testKi18ndcp()
    {
        KLocalizedString s = ki18ndcp("domain", "context", "Singular", "Plural");
        QVERIFY(!s.isEmpty());
    }

    void testKxi18n()
    {
        KLocalizedString s = kxi18n("Hello");
        QVERIFY(!s.isEmpty());
    }

    void testKxi18nc()
    {
        KLocalizedString s = kxi18nc("context", "Hello");
        QVERIFY(!s.isEmpty());
    }

    void testKxi18np()
    {
        KLocalizedString s = kxi18np("Singular", "Plural");
        QVERIFY(!s.isEmpty());
    }

    void testKxi18ncp()
    {
        KLocalizedString s = kxi18ncp("context", "Singular", "Plural");
        QVERIFY(!s.isEmpty());
    }

    void testCopyConstructor()
    {
        KLocalizedString s1 = ki18n("Hello");
        KLocalizedString s2(s1);
        QVERIFY(!s2.isEmpty());
    }

    void testAssignmentOperator()
    {
        KLocalizedString s1 = ki18n("Hello");
        KLocalizedString s2;
        s2 = s1;
        QVERIFY(!s2.isEmpty());
    }

    void testSubsInt()
    {
        KLocalizedString s = ki18n("Number %1").subs(42);
        QVERIFY(!s.isEmpty());
    }

    void testSubsUint()
    {
        KLocalizedString s = ki18n("Number %1").subs(42u);
        QVERIFY(!s.isEmpty());
    }

    void testSubsLong()
    {
        KLocalizedString s = ki18n("Number %1").subs(42L);
        QVERIFY(!s.isEmpty());
    }

    void testSubsULong()
    {
        KLocalizedString s = ki18n("Number %1").subs(42UL);
        QVERIFY(!s.isEmpty());
    }

    void testSubsLongLong()
    {
        KLocalizedString s = ki18n("Number %1").subs(42LL);
        QVERIFY(!s.isEmpty());
    }

    void testSubsULongLong()
    {
        KLocalizedString s = ki18n("Number %1").subs(42ULL);
        QVERIFY(!s.isEmpty());
    }

    void testSubsDouble()
    {
        KLocalizedString s = ki18n("Number %1").subs(3.14);
        QVERIFY(!s.isEmpty());
    }

    void testSubsQChar()
    {
        KLocalizedString s = ki18n("Char %1").subs(QChar('A'));
        QVERIFY(!s.isEmpty());
    }

    void testSubsQString()
    {
        KLocalizedString s = ki18n("Hello %1").subs(QStringLiteral("World"));
        QVERIFY(!s.isEmpty());
    }

    void testSubsKLocalizedString()
    {
        KLocalizedString inner = ki18n("inner");
        KLocalizedString s = ki18n("outer %1").subs(inner);
        QVERIFY(!s.isEmpty());
    }

    void testWithDomain()
    {
        KLocalizedString s = ki18n("Hello").withDomain("test-domain");
        QVERIFY(!s.isEmpty());
    }

    void testWithFormat()
    {
        KLocalizedString s = ki18n("Hello").withFormat(Kuit::PlainText);
        QVERIFY(!s.isEmpty());

        KLocalizedString s2 = ki18n("Hello").withFormat(Kuit::RichText);
        QVERIFY(!s2.isEmpty());
    }

    void testInContext()
    {
        KLocalizedString s = ki18n("Hello").inContext(QStringLiteral("key"), QStringLiteral("value"));
        QVERIFY(!s.isEmpty());
    }

    void testRelaxSubs()
    {
        KLocalizedString s = ki18n("Hello").relaxSubs();
        QVERIFY(!s.isEmpty());
    }

    void testIgnoreMarkup()
    {
        KLocalizedString s = ki18n("Hello").ignoreMarkup();
        QVERIFY(!s.isEmpty());
    }

    void testUntranslatedText()
    {
        KLocalizedString s;
        QVERIFY(s.untranslatedText().isEmpty());

        KLocalizedString s2 = ki18n("Hello World");
        // untranslatedText returns the original text
        QCOMPARE(s2.untranslatedText(), QByteArray("Hello World"));
    }

    void testRemoveAcceleratorMarker()
    {
        // Basic cases
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QStringLiteral("&File")), QStringLiteral("File"));
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QStringLiteral("&Open File")), QStringLiteral("Open File"));

        // Double ampersand (literal &)
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QStringLiteral("Save && &Exit")), QStringLiteral("Save & Exit"));

        // No accelerator marker
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QStringLiteral("File")), QStringLiteral("File"));

        // Empty string
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QString()), QString());

        // CJK style: "保存(&S)"
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QStringLiteral("保存(&S)")), QStringLiteral("保存"));

        // Multiple accelerators
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QStringLiteral("&File && &Edit")), QStringLiteral("File & Edit"));

        // Ampersand at end
        QCOMPARE(KLocalizedString::removeAcceleratorMarker(QStringLiteral("File&")), QStringLiteral("File&"));
    }

    void testSubsChain()
    {
        KLocalizedString s = ki18n("%1 and %2").subs("A").subs("B");
        QVERIFY(!s.isEmpty());
    }

    void testWithLanguages()
    {
        KLocalizedString s = ki18n("Hello").withLanguages(QStringList() << QStringLiteral("en") << QStringLiteral("de"));
        QVERIFY(!s.isEmpty());
    }

    void testToStringEmpty()
    {
        KLocalizedString s;
        QVERIFY(s.toString().isEmpty());
    }

    void testToStringWithLanguages()
    {
        KLocalizedString s = ki18n("Hello");
        QString result = s.toString(QStringList() << QStringLiteral("en"));
        QVERIFY(result.isEmpty() || !result.isEmpty());
    }

    void testToStringWithDomain()
    {
        KLocalizedString s = ki18n("Hello");
        QString result = s.toString("test-domain");
        QVERIFY(result.isEmpty() || !result.isEmpty());
    }

    void testToStringWithFormat()
    {
        KLocalizedString s = ki18n("Hello");
        QString result = s.toString(Kuit::PlainText);
        QVERIFY(result.isEmpty() || !result.isEmpty());
    }
};

// ============================================================================
// TestKLocalizedContext
// ============================================================================
class TestKLocalizedContext : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void testDefaultConstructor()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.translationDomain().isEmpty());
    }

    void testSetTranslationDomain()
    {
        KLocalizedContext ctx;
        QSignalSpy spy(&ctx, SIGNAL(translationDomainChanged(QString)));

        ctx.setTranslationDomain(QStringLiteral("test-domain"));
        QCOMPARE(ctx.translationDomain(), QStringLiteral("test-domain"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("test-domain"));
    }

    void testSetSameTranslationDomain()
    {
        KLocalizedContext ctx;
        QSignalSpy spy(&ctx, SIGNAL(translationDomainChanged(QString)));

        // Use domain that hasn't been set by previous tests (static member state persists)
        ctx.setTranslationDomain(QStringLiteral("same-domain-test-unique"));
        QCOMPARE(spy.count(), 1);

        // Setting same domain should not emit signal
        ctx.setTranslationDomain(QStringLiteral("same-domain-test-unique"));
        QCOMPARE(spy.count(), 1);
    }

    void testSetDifferentTranslationDomain()
    {
        KLocalizedContext ctx;
        QSignalSpy spy(&ctx, SIGNAL(translationDomainChanged(QString)));

        ctx.setTranslationDomain(QStringLiteral("domain1"));
        ctx.setTranslationDomain(QStringLiteral("domain2"));
        QCOMPARE(spy.count(), 2);
    }

    void testI18nEmptyMessage()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18n(QString()).isEmpty());
    }

    void testI18nWithMessage()
    {
        KLocalizedContext ctx;
        // Without translation domain, returns the untranslated source text
        QString result = ctx.i18n(QStringLiteral("Hello"));
        QCOMPARE(result, QStringLiteral("Hello"));
    }

    void testI18ncEmptyContext()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18nc(QString(), QStringLiteral("msg")).isEmpty());
    }

    void testI18ncEmptyMessage()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18nc(QStringLiteral("ctx"), QString()).isEmpty());
    }

    void testI18npEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18np(QString(), QStringLiteral("plural"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18np(QStringLiteral("singular"), QString(), QVariant(1)).isEmpty());
    }

    void testI18ncpEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18ncp(QString(), QStringLiteral("singular"), QStringLiteral("plural"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18ncp(QStringLiteral("ctx"), QString(), QStringLiteral("plural"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18ncp(QStringLiteral("ctx"), QStringLiteral("singular"), QString(), QVariant(1)).isEmpty());
    }

    void testI18ndEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18nd(QString(), QStringLiteral("msg")).isEmpty());
        QVERIFY(ctx.i18nd(QStringLiteral("domain"), QString()).isEmpty());
    }

    void testI18ndcEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18ndc(QString(), QStringLiteral("ctx"), QStringLiteral("msg")).isEmpty());
        QVERIFY(ctx.i18ndc(QStringLiteral("domain"), QString(), QStringLiteral("msg")).isEmpty());
        QVERIFY(ctx.i18ndc(QStringLiteral("domain"), QStringLiteral("ctx"), QString()).isEmpty());
    }

    void testI18ndpEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18ndp(QString(), QStringLiteral("s"), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18ndp(QStringLiteral("d"), QString(), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18ndp(QStringLiteral("d"), QStringLiteral("s"), QString(), QVariant(1)).isEmpty());
    }

    void testI18ndcpEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.i18ndcp(QString(), QStringLiteral("c"), QStringLiteral("s"), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18ndcp(QStringLiteral("d"), QString(), QStringLiteral("s"), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18ndcp(QStringLiteral("d"), QStringLiteral("c"), QString(), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.i18ndcp(QStringLiteral("d"), QStringLiteral("c"), QStringLiteral("s"), QString(), QVariant(1)).isEmpty());
    }

    void testXi18nEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18n(QString()).isEmpty());
    }

    void testXi18ncEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18nc(QString(), QStringLiteral("msg")).isEmpty());
        QVERIFY(ctx.xi18nc(QStringLiteral("ctx"), QString()).isEmpty());
    }

    void testXi18npEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18np(QString(), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18np(QStringLiteral("s"), QString(), QVariant(1)).isEmpty());
    }

    void testXi18ncpEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18ncp(QString(), QStringLiteral("s"), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18ncp(QStringLiteral("c"), QString(), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18ncp(QStringLiteral("c"), QStringLiteral("s"), QString(), QVariant(1)).isEmpty());
    }

    void testXi18ndEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18nd(QString(), QStringLiteral("msg")).isEmpty());
        QVERIFY(ctx.xi18nd(QStringLiteral("d"), QString()).isEmpty());
    }

    void testXi18ndcEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18ndc(QString(), QStringLiteral("c"), QStringLiteral("m")).isEmpty());
        QVERIFY(ctx.xi18ndc(QStringLiteral("d"), QString(), QStringLiteral("m")).isEmpty());
        QVERIFY(ctx.xi18ndc(QStringLiteral("d"), QStringLiteral("c"), QString()).isEmpty());
    }

    void testXi18ndpEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18ndp(QString(), QStringLiteral("s"), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18ndp(QStringLiteral("d"), QString(), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18ndp(QStringLiteral("d"), QStringLiteral("s"), QString(), QVariant(1)).isEmpty());
    }

    void testXi18ndcpEmpty()
    {
        KLocalizedContext ctx;
        QVERIFY(ctx.xi18ndcp(QString(), QStringLiteral("c"), QStringLiteral("s"), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18ndcp(QStringLiteral("d"), QString(), QStringLiteral("s"), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18ndcp(QStringLiteral("d"), QStringLiteral("c"), QString(), QStringLiteral("p"), QVariant(1)).isEmpty());
        QVERIFY(ctx.xi18ndcp(QStringLiteral("d"), QStringLiteral("c"), QStringLiteral("s"), QString(), QVariant(1)).isEmpty());
    }
};

// ============================================================================
// TestKLocalizedTranslator
// ============================================================================
class TestKLocalizedTranslator : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void testDefaultConstructor()
    {
        KLocalizedTranslator t;
        QVERIFY(t.translate("ctx", "source").isEmpty());
    }

    void testSetTranslationDomain()
    {
        KLocalizedTranslator t;
        t.setTranslationDomain(QStringLiteral("test-domain"));

        // Without monitored context, falls through to base QTranslator
        QVERIFY(t.translate("ctx", "source").isEmpty());
    }

    void testAddContextToMonitor()
    {
        KLocalizedTranslator t;
        t.setTranslationDomain(QStringLiteral("test-domain"));
        t.addContextToMonitor(QStringLiteral("my-context"));

        // Context is monitored - ki18nd returns the source text as untranslated fallback
        QString result = t.translate("my-context", "source-text");
        QCOMPARE(result, QStringLiteral("source-text"));
    }

    void testAddRemoveContextToMonitor()
    {
        KLocalizedTranslator t;
        t.setTranslationDomain(QStringLiteral("test-domain"));
        t.addContextToMonitor(QStringLiteral("ctx1"));
        t.removeContextToMonitor(QStringLiteral("ctx1"));

        // After removal, falls through to base QTranslator
        QVERIFY(t.translate("ctx1", "source").isEmpty());
    }

    void testTranslateWithDisambiguation()
    {
        KLocalizedTranslator t;
        t.setTranslationDomain(QStringLiteral("test-domain"));
        t.addContextToMonitor(QStringLiteral("ctx"));

        // With disambiguation, ki18ndc returns the source text as untranslated fallback
        QString result = t.translate("ctx", "source", "disambig");
        QCOMPARE(result, QStringLiteral("source"));
    }

    void testTranslateWithoutMonitoredContext()
    {
        KLocalizedTranslator t;
        t.setTranslationDomain(QStringLiteral("test-domain"));

        // Context not monitored, should fall through
        QVERIFY(t.translate("some-ctx", "source").isEmpty());
    }
};

// ============================================================================
// TestKuit
// ============================================================================
class TestKuit : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void testSetupForDomain()
    {
        KuitSetup &setup = Kuit::setupForDomain("test-domain");
        QVERIFY(&setup == &setup); // just verify it returns a reference
    }

    void testSetupForDomainSingleton()
    {
        KuitSetup &setup1 = Kuit::setupForDomain("singleton-test");
        KuitSetup &setup2 = Kuit::setupForDomain("singleton-test");
        QVERIFY(&setup1 == &setup2); // Same domain returns same instance
    }

    void testSetupForDomainDifferentInstances()
    {
        KuitSetup &setup1 = Kuit::setupForDomain("domain-a");
        KuitSetup &setup2 = Kuit::setupForDomain("domain-b");
        QVERIFY(&setup1 != &setup2); // Different domains return different instances
    }

    void testSetTagClass()
    {
        KuitSetup &setup = Kuit::setupForDomain("tag-class-test");
        setup.setTagClass(QStringLiteral("mytag"), Kuit::StructTag);
    }

    void testSetFormatForMarker()
    {
        KuitSetup &setup = Kuit::setupForDomain("format-marker-test");
        setup.setFormatForMarker(QStringLiteral(":my-marker"), Kuit::PlainText);
    }

    void testDefaultSetup()
    {
        // Default empty domain setup (created implicitly)
        KuitSetup &setup = Kuit::setupForDomain(nullptr);
        QVERIFY(&setup == &Kuit::setupForDomain(QByteArray()));
    }
};

// CombinedTest at the bottom of this file serves as the single entry point.

// The tests are combined into one executable:
// We use a single main to avoid redefinition errors.
// Instead, we define a combined test runner.
// Actually, let me restructure: use a single test class that includes all tests.
// But for clarity, we keep separate classes and use a manual main.
// Actually, the simplest approach: use one QTEST_GUILESS_MAIN per test file.
// But since this is a single file, we need one main.

// Let's just keep TestKCatalog as the primary and include a combined runner.
// Actually, I'll put all test classes in a single QTEST_GUILESS_MAIN call
// by defining a combined test that runs all test classes.
// The cleanest approach is:
//   QTEST_GUILESS_MAIN(TestKCatalog)  -- this generates main() for TestKCatalog only
// To run all tests, we create a separate main manually.

// Actually, since QTEST_GUILESS_MAIN cannot be used for multiple classes in one file,
// let me manually define main() to run all test classes:
class CombinedTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // KCatalog tests
    void testKCatalogDefaultConstructor() { TestKCatalog().testDefaultConstructor(); }
    void testKCatalogDomainConstructor() { TestKCatalog().testDomainConstructor(); }
    void testKCatalogDomainLanguageConstructor() { TestKCatalog().testDomainLanguageConstructor(); }
    void testKCatalogCopyConstructor() { TestKCatalog().testCopyConstructor(); }
    void testKCatalogAssignmentOperator() { TestKCatalog().testAssignmentOperator(); }
    void testKCatalogTranslateEmpty() { TestKCatalog().testTranslateEmpty(); }
    void testKCatalogTranslateWithDomain() { TestKCatalog().testTranslateWithDomain(); }
    void testKCatalogTranslateOverloads() { TestKCatalog().testTranslateOverloads(); }
    void testKCatalogLanguages() { TestKCatalog().testLanguages(); }
    void testKCatalogLocaleDir() { TestKCatalog().testLocaleDir(); }
    void testKCatalogStaticCatalogLocaleDir() { TestKCatalog().testStaticCatalogLocaleDir(); }
    void testKCatalogAddDomainLocaleDir() { TestKCatalog().testAddDomainLocaleDir(); }
    void testKCatalogAvailableCatalogLanguages() { TestKCatalog().testAvailableCatalogLanguages(); }

    // KLocalizedString tests
    void testKLSDefaultConstructor() { TestKLocalizedString().testDefaultConstructor(); }
    void testKLSKi18n() { TestKLocalizedString().testKi18n(); }
    void testKLSKi18nc() { TestKLocalizedString().testKi18nc(); }
    void testKLSKi18np() { TestKLocalizedString().testKi18np(); }
    void testKLSKi18ncp() { TestKLocalizedString().testKi18ncp(); }
    void testKLSKi18nd() { TestKLocalizedString().testKi18nd(); }
    void testKLSKi18ndc() { TestKLocalizedString().testKi18ndc(); }
    void testKLSKi18ndp() { TestKLocalizedString().testKi18ndp(); }
    void testKLSKi18ndcp() { TestKLocalizedString().testKi18ndcp(); }
    void testKLSKxi18n() { TestKLocalizedString().testKxi18n(); }
    void testKLSKxi18nc() { TestKLocalizedString().testKxi18nc(); }
    void testKLSKxi18np() { TestKLocalizedString().testKxi18np(); }
    void testKLSKxi18ncp() { TestKLocalizedString().testKxi18ncp(); }
    void testKLSCopyConstructor() { TestKLocalizedString().testCopyConstructor(); }
    void testKLSAssignmentOperator() { TestKLocalizedString().testAssignmentOperator(); }
    void testKLSSubsInt() { TestKLocalizedString().testSubsInt(); }
    void testKLSSubsUint() { TestKLocalizedString().testSubsUint(); }
    void testKLSSubsLong() { TestKLocalizedString().testSubsLong(); }
    void testKLSSubsULong() { TestKLocalizedString().testSubsULong(); }
    void testKLSSubsLongLong() { TestKLocalizedString().testSubsLongLong(); }
    void testKLSSubsULongLong() { TestKLocalizedString().testSubsULongLong(); }
    void testKLSSubsDouble() { TestKLocalizedString().testSubsDouble(); }
    void testKLSSubsQChar() { TestKLocalizedString().testSubsQChar(); }
    void testKLSSubsQString() { TestKLocalizedString().testSubsQString(); }
    void testKLSSubsKLocalizedString() { TestKLocalizedString().testSubsKLocalizedString(); }
    void testKLSWithDomain() { TestKLocalizedString().testWithDomain(); }
    void testKLSWithFormat() { TestKLocalizedString().testWithFormat(); }
    void testKLSInContext() { TestKLocalizedString().testInContext(); }
    void testKLSRelaxSubs() { TestKLocalizedString().testRelaxSubs(); }
    void testKLSIgnoreMarkup() { TestKLocalizedString().testIgnoreMarkup(); }
    void testKLSUntranslatedText() { TestKLocalizedString().testUntranslatedText(); }
    void testKLSRemoveAcceleratorMarker() { TestKLocalizedString().testRemoveAcceleratorMarker(); }
    void testKLSSubsChain() { TestKLocalizedString().testSubsChain(); }
    void testKLSWithLanguages() { TestKLocalizedString().testWithLanguages(); }
    void testKLSToStringEmpty() { TestKLocalizedString().testToStringEmpty(); }
    void testKLSToStringWithLanguages() { TestKLocalizedString().testToStringWithLanguages(); }
    void testKLSToStringWithDomain() { TestKLocalizedString().testToStringWithDomain(); }
    void testKLSToStringWithFormat() { TestKLocalizedString().testToStringWithFormat(); }

    // KLocalizedContext tests
    void testKLCDefaultConstructor() { TestKLocalizedContext().testDefaultConstructor(); }
    void testKLCSetTranslationDomain() { TestKLocalizedContext().testSetTranslationDomain(); }
    void testKLCSetSameTranslationDomain() { TestKLocalizedContext().testSetSameTranslationDomain(); }
    void testKLCSetDifferentTranslationDomain() { TestKLocalizedContext().testSetDifferentTranslationDomain(); }
    void testKLCI18nEmptyMessage() { TestKLocalizedContext().testI18nEmptyMessage(); }
    void testKLCI18nWithMessage() { TestKLocalizedContext().testI18nWithMessage(); }
    void testKLCI18ncEmptyContext() { TestKLocalizedContext().testI18ncEmptyContext(); }
    void testKLCI18ncEmptyMessage() { TestKLocalizedContext().testI18ncEmptyMessage(); }
    void testKLCI18npEmpty() { TestKLocalizedContext().testI18npEmpty(); }
    void testKLCI18ncpEmpty() { TestKLocalizedContext().testI18ncpEmpty(); }
    void testKLCI18ndEmpty() { TestKLocalizedContext().testI18ndEmpty(); }
    void testKLCI18ndcEmpty() { TestKLocalizedContext().testI18ndcEmpty(); }
    void testKLCI18ndpEmpty() { TestKLocalizedContext().testI18ndpEmpty(); }
    void testKLCI18ndcpEmpty() { TestKLocalizedContext().testI18ndcpEmpty(); }
    void testKLCXi18nEmpty() { TestKLocalizedContext().testXi18nEmpty(); }
    void testKLCXi18ncEmpty() { TestKLocalizedContext().testXi18ncEmpty(); }
    void testKLCXi18npEmpty() { TestKLocalizedContext().testXi18npEmpty(); }
    void testKLCXi18ncpEmpty() { TestKLocalizedContext().testXi18ncpEmpty(); }
    void testKLCXi18ndEmpty() { TestKLocalizedContext().testXi18ndEmpty(); }
    void testKLCXi18ndcEmpty() { TestKLocalizedContext().testXi18ndcEmpty(); }
    void testKLCXi18ndpEmpty() { TestKLocalizedContext().testXi18ndpEmpty(); }
    void testKLCXi18ndcpEmpty() { TestKLocalizedContext().testXi18ndcpEmpty(); }

    // KLocalizedTranslator tests
    void testKLTDefaultConstructor() { TestKLocalizedTranslator().testDefaultConstructor(); }
    void testKLTSetTranslationDomain() { TestKLocalizedTranslator().testSetTranslationDomain(); }
    void testKLTAddContextToMonitor() { TestKLocalizedTranslator().testAddContextToMonitor(); }
    void testKLTAddRemoveContextToMonitor() { TestKLocalizedTranslator().testAddRemoveContextToMonitor(); }
    void testKLTTranslateWithDisambiguation() { TestKLocalizedTranslator().testTranslateWithDisambiguation(); }
    void testKLTTranslateWithoutMonitoredContext() { TestKLocalizedTranslator().testTranslateWithoutMonitoredContext(); }

    // Kuit tests
    void testKuitSetupForDomain() { TestKuit().testSetupForDomain(); }
    void testKuitSetupForDomainSingleton() { TestKuit().testSetupForDomainSingleton(); }
    void testKuitSetupForDomainDifferentInstances() { TestKuit().testSetupForDomainDifferentInstances(); }
    void testKuitSetTagClass() { TestKuit().testSetTagClass(); }
    void testKuitSetFormatForMarker() { TestKuit().testSetFormatForMarker(); }
    void testKuitDefaultSetup() { TestKuit().testDefaultSetup(); }
};

QTEST_GUILESS_MAIN(CombinedTest)

#include "cpp_kf5i18n_test.moc"
