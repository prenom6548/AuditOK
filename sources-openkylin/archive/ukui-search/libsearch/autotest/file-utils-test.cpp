#include "file-utils-test.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QDBusConnection>
#include <QDBusContext>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QUrl>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

#include "common.h"
#include "file-utils.h"

using namespace UkuiSearch;

namespace {

class FakePrivateDirWatcherService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.search.fileindex")

public Q_SLOTS:
    QStringList currentIndexableDir() const
    {
        return QStringList{QDir::homePath()};
    }

    QStringList blackListOfDir(const QString &) const
    {
        return {};
    }

    QStringList blockDirsForUser() const
    {
        return {};
    }

    QStringList currentBlackListOfIndex() const
    {
        return {};
    }

    QStringList extraDirs() const
    {
        return {};
    }

    QStringList currentSearchableDir() const
    {
        return {};
    }

    QStringList searchableDirForSearchApplication() const
    {
        return {};
    }

    QStringList extraDirsOfPaths(const QStringList &) const
    {
        return {};
    }

    int appendSearchDir(const QString &)
    {
        return 0;
    }

    void removeSearchDir(const QString &)
    {
    }

    int appendIndexableListItem(const QString &)
    {
        return 0;
    }

    void removeIndexableListItem(const QString &)
    {
    }

    void initDbusService()
    {
    }
};

class FakeFileManager1Service : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")

public:
    QStringList lastItems;
    QString lastStartupId;

public Q_SLOTS:
    void ShowItems(const QStringList &items, const QString &startupId)
    {
        lastItems = items;
        lastStartupId = startupId;
    }
};

class BrokenFileManager1Service : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")

public Q_SLOTS:
    void ShowItems(const QStringList &, const QString &)
    {
        sendErrorReply(QStringLiteral("org.freedesktop.FileManager1.Error.Failed"),
                       QStringLiteral("forced failure"));
    }
};

class FakeAppLauncher : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")

public:
    QString lastUrl;
    int launchUrlCalls = 0;

public Q_SLOTS:
    void LaunchDefaultAppWithUrl(const QString &url)
    {
        lastUrl = url;
        ++launchUrlCalls;
    }
};

class ScopedDbusRegistration
{
public:
    ScopedDbusRegistration(const QString &service,
                           const QString &path,
                           QObject *object,
                           const QDBusConnection &connection = QDBusConnection::sessionBus())
        : m_service(service),
          m_path(path),
          m_connection(connection)
    {
        if (!m_connection.isConnected()) {
            return;
        }
        m_serviceRegistered = m_connection.registerService(service);
        m_objectRegistered = m_connection.registerObject(path, object,
                                                         QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    }

    ~ScopedDbusRegistration()
    {
        if (m_objectRegistered) {
            m_connection.unregisterObject(m_path);
        }
        if (m_serviceRegistered) {
            m_connection.unregisterService(m_service);
        }
    }

    bool isRegistered() const
    {
        return m_serviceRegistered && m_objectRegistered;
    }

private:
    QString m_service;
    QString m_path;
    QDBusConnection m_connection{QString()};
    bool m_serviceRegistered = false;
    bool m_objectRegistered = false;
};

class ScopedEnvVar
{
public:
    ScopedEnvVar(const QByteArray &name, const QByteArray &value)
        : m_name(name),
          m_hadValue(qEnvironmentVariableIsSet(name.constData())),
          m_oldValue(qgetenv(name.constData()))
    {
        qputenv(m_name, value);
    }

    ~ScopedEnvVar()
    {
        if (m_hadValue) {
            qputenv(m_name, m_oldValue);
        } else {
            qunsetenv(m_name);
        }
    }

private:
    QByteArray m_name;
    bool m_hadValue;
    QByteArray m_oldValue;
};

// Keep the fake D-Bus service alive until process teardown without running a
// QObject global destructor, which can deadlock during late exit.
FakePrivateDirWatcherService *g_dirWatcherService = new FakePrivateDirWatcherService;
std::unique_ptr<ScopedDbusRegistration> g_dirWatcherRegistration;

} // namespace

void FileUtilsTest::initTestCase()
{
    g_dirWatcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.ukui.search.fileindex.service"),
                                                              QStringLiteral("/org/ukui/search/privateDirWatcher"),
                                                              g_dirWatcherService));
}

void FileUtilsTest::cleanupTestCase()
{
    g_dirWatcherRegistration.reset();
}

void FileUtilsTest::testMakeDocUterm()
{
    QCOMPARE(QString::fromStdString(FileUtils::makeDocUterm(QStringLiteral("/tmp/a.txt"))).size(), 32);
    QCOMPARE(FileUtils::makeDocUterm(QStringLiteral("/tmp/a.txt")),
             FileUtils::makeDocUterm(QStringLiteral("/tmp/a.txt")));
    QVERIFY(FileUtils::makeDocUterm(QStringLiteral("/tmp/a.txt")) !=
            FileUtils::makeDocUterm(QStringLiteral("/tmp/b.txt")));
}

void FileUtilsTest::testIsOrUnder()
{
    QVERIFY(FileUtils::isOrUnder(QStringLiteral("/home/user/file"), QStringLiteral("/")));
    QVERIFY(FileUtils::isOrUnder(QStringLiteral("/home/user"), QStringLiteral("/home/user")));
    QVERIFY(FileUtils::isOrUnder(QStringLiteral("/home/user/docs/file"), QStringLiteral("/home/user")));
    QVERIFY(!FileUtils::isOrUnder(QStringLiteral("/home/user2"), QStringLiteral("/home/user")));
    QVERIFY(!FileUtils::isOrUnder(QStringLiteral("/home"), QStringLiteral("/home/user")));
}

void FileUtilsTest::testEscapeHtmlAndBoldHtml()
{
    QCOMPARE(FileUtils::escapeHtml(QStringLiteral("<tag>")), QStringLiteral("&lt;tag&gt;"));

    const QString bold = FileUtils::setAllTextBold(QStringLiteral("<name>"));
    QVERIFY(bold.contains(QStringLiteral("&lt;name&gt;")));
    QVERIFY(bold.startsWith(QStringLiteral("<h3")));

    const QString html = FileUtils::getHtmlText(QStringLiteral("a<b>\n"), QStringLiteral("<"));
    QVERIFY(html.contains(QStringLiteral("&lt;")));
    QVERIFY(html.contains(QStringLiteral("<br />")));
    QVERIFY(html.startsWith(QStringLiteral("<pre>")));
}

void FileUtilsTest::testFindMultiToneWords()
{
    const QStringList result = FileUtils::findMultiToneWords(QStringLiteral("中"));

    QCOMPARE(result.size(), 2);
    QVERIFY(!result.at(0).isEmpty());
    QVERIFY(!result.at(1).isEmpty());
}

void FileUtilsTest::testOcrSupportSize()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString smallPath = dir.filePath(QStringLiteral("small.png"));
    QImage small(OCR_MIN_SIZE - 1, OCR_MIN_SIZE - 1, QImage::Format_RGB32);
    small.fill(Qt::white);
    QVERIFY(small.save(smallPath));
    QVERIFY(!FileUtils::isOcrSupportSize(smallPath));

    const QString largePath = dir.filePath(QStringLiteral("large.png"));
    QImage large(OCR_MIN_SIZE, OCR_MIN_SIZE, QImage::Format_RGB32);
    large.fill(Qt::white);
    QVERIFY(large.save(largePath));
    QVERIFY(FileUtils::isOcrSupportSize(largePath));
}

void FileUtilsTest::testSnippetFormattingBranches()
{
    const std::string shortText = "alpha keyword beta";
    const QString shortSnippet = FileUtils::getSnippet(shortText, 6, QStringLiteral("keyword"));
    QVERIFY(shortSnippet.contains(QStringLiteral("keyword")));

    const QString repeated = QStringLiteral("prefix ") + QString(260, QLatin1Char('a')) + QStringLiteral(" keyword tail");
    const QString leftElided = FileUtils::getSnippet(repeated.toStdString(), 260, QStringLiteral("keyword"));
    QVERIFY(leftElided.contains(QStringLiteral("keyword")));

    const QString longKeyword = FileUtils::getSnippet(QString(520, QLatin1Char('k')).toStdString(), 0, QStringLiteral("k"));
    QVERIFY(!longKeyword.isEmpty());

    const qreal mixedAdvance = FileUtils::horizontalAdvanceContainsKeyword(QStringLiteral("abcKEYxyz"), QStringLiteral("KEY"));
    QVERIFY(mixedAdvance > 0);
}

void FileUtilsTest::testSnippetBoundaryAndElideBranches()
{
    const QString prefix(420, QLatin1Char('p'));
    const QString suffix(420, QLatin1Char('s'));
    const QString keyword = QStringLiteral("needle");
    const QString content = prefix + keyword + suffix;
    const uint keywordStart = static_cast<uint>(prefix.size());

    const QString centered = FileUtils::getSnippet(content.toStdString(), keywordStart, keyword);
    QVERIFY(centered.contains(keyword));
    QVERIFY(centered.contains(QStringLiteral("…")) || centered.contains(QStringLiteral("\n")));

    const QString trailingKeyword = QString(520, QLatin1Char('a')) + keyword;
    const QString leftElided = FileUtils::getSnippet(trailingKeyword.toStdString(),
                                                     static_cast<uint>(trailingKeyword.size() - keyword.size()),
                                                     keyword);
    QVERIFY(leftElided.contains(keyword));

    const QString noKeyword = FileUtils::getSnippet(QString(520, QLatin1Char('x')).toStdString(),
                                                    0,
                                                    QStringLiteral("z"));
    QVERIFY(!noKeyword.isEmpty());
    QVERIFY(noKeyword.contains(QStringLiteral("…")) || noKeyword.contains(QStringLiteral("\n")));

    const QString mixed = QStringLiteral("abc") + keyword + QStringLiteral("def") + keyword;
    const QString exact = FileUtils::getSnippet(mixed.toStdString(), 0, keyword);
    QVERIFY(exact.contains(keyword));
}

void FileUtilsTest::testSnippetWithoutKeywordBranches()
{
    const QString explicitLine = FileUtils::getSnippetWithoutKeyword(QStringLiteral("first line\nsecond line"), 1);
    QVERIFY(explicitLine.endsWith(QStringLiteral("…")) || explicitLine.contains(QStringLiteral("\n")));

    const QString wrapped = FileUtils::getSnippetWithoutKeyword(QString(260, QLatin1Char('w')), 1);
    QVERIFY(wrapped.endsWith(QStringLiteral("…")) || wrapped.contains(QStringLiteral("\n")));

    const QString twoLines = FileUtils::getSnippetWithoutKeyword(QStringLiteral("short text"), 2);
    QVERIFY(twoLines.contains(QStringLiteral("short")));

    const QString exactBreak = FileUtils::getSnippetWithoutKeyword(QStringLiteral("line one\nline two\nline three"), 2);
    QVERIFY(exactBreak.contains(QStringLiteral("line one")));

    const QString oneLineElide = FileUtils::getSnippetWithoutKeyword(QString(520, QLatin1Char('q')), 1);
    QVERIFY(oneLineElide.endsWith(QStringLiteral("…")) || !oneLineElide.isEmpty());
}

void FileUtilsTest::testHtmlAndAdvanceBoundaryBranches()
{
    const QString html = FileUtils::getHtmlText(QStringLiteral("Ab<c>\nxy"), QStringLiteral("ac"));
    QVERIFY(html.contains(QStringLiteral("<span>")));
    QVERIFY(html.contains(QStringLiteral("&lt;")));
    QVERIFY(html.contains(QStringLiteral("<br />")));
    QVERIFY(html.endsWith(QStringLiteral("</pre>")));

    const QString noBold = FileUtils::getHtmlText(QStringLiteral("plain text"), QStringLiteral("ZZ"));
    QVERIFY(!noBold.contains(QStringLiteral("<span>")));

    QVERIFY(FileUtils::horizontalAdvanceContainsKeyword(QStringLiteral("abc"), QStringLiteral("abc")) > 0);
    QVERIFY(FileUtils::horizontalAdvanceContainsKeyword(QStringLiteral("abc"), QStringLiteral("z")) > 0);
    QVERIFY(FileUtils::horizontalAdvanceContainsKeyword(QStringLiteral("aZbZc"), QStringLiteral("z")) > 0);
}

void FileUtilsTest::testWrapDataAndClipboard()
{
    QLabel label;
    label.setFont(QApplication::font());

    const QString shortText = QStringLiteral("short");
    QCOMPARE(FileUtils::wrapData(&label, shortText), shortText);

    const QString longText(260, QLatin1Char('x'));
    const QString wrapped = FileUtils::wrapData(&label, longText);
    QVERIFY(wrapped.contains(QStringLiteral("\n")) || wrapped == longText);

    QString clipboardText = QStringLiteral("/tmp/copied-file");
    QVERIFY(FileUtils::copyPath(clipboardText));
    QCOMPARE(QApplication::clipboard()->text(), clipboardText);
}

void FileUtilsTest::testEncryptionAndFormatSupportBranches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString tinyPath = dir.filePath(QStringLiteral("tiny.docx"));
    {
        QFile file(tinyPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("PK");
    }
    QVERIFY(FileUtils::isOpenXMLFileEncrypted(tinyPath));

    const QString zipPath = dir.filePath(QStringLiteral("plain.docx"));
    {
        QFile file(zipPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("PK\003\004content");
    }
    QVERIFY(!FileUtils::isOpenXMLFileEncrypted(zipPath));
    QVERIFY(!FileUtils::isEncrypedOrUnsupport(zipPath, QStringLiteral("docx")));

    const QString genericZipPath = dir.filePath(QStringLiteral("archive.zip"));
    {
        QFile file(genericZipPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        const char emptyZip[] = {
            'P', 'K', 0x05, 0x06,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0
        };
        file.write(emptyZip, sizeof(emptyZip));
    }
    if (FileUtils::getMimetype(genericZipPath).name() == QStringLiteral("application/zip")) {
        QVERIFY(!FileUtils::isEncrypedOrUnsupport(genericZipPath, QStringLiteral("uof")));
        QVERIFY(!FileUtils::isEncrypedOrUnsupport(genericZipPath, QStringLiteral("ofd")));
        QVERIFY(FileUtils::isEncrypedOrUnsupport(genericZipPath, QStringLiteral("unsupported")));
    } else {
        QVERIFY(FileUtils::isEncrypedOrUnsupport(genericZipPath, QStringLiteral("unsupported")));
    }

    const QString textPath = dir.filePath(QStringLiteral("plain.txt"));
    {
        QFile file(textPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("hello");
    }
    QVERIFY(!FileUtils::isEncrypedOrUnsupport(textPath, QStringLiteral("txt")));
    QVERIFY(FileUtils::isEncrypedOrUnsupport(textPath, QStringLiteral("bin")));

    const QString htmlPath = dir.filePath(QStringLiteral("page.html"));
    {
        QFile file(htmlPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("<html></html>");
    }
    QVERIFY(!FileUtils::isEncrypedOrUnsupport(htmlPath, QStringLiteral("html")));
    QVERIFY(FileUtils::isEncrypedOrUnsupport(htmlPath, QStringLiteral("txt")));
}

void FileUtilsTest::testAdditionalFormatAndOcrBranches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString pdfPath = dir.filePath(QStringLiteral("doc.pdf"));
    {
        QFile file(pdfPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("%PDF-1.4\n1 0 obj\n<<>>\nendobj\n%%EOF\n");
    }
    if (FileUtils::getMimetype(pdfPath).name() == QStringLiteral("application/pdf")) {
        QVERIFY(!FileUtils::isEncrypedOrUnsupport(pdfPath, QStringLiteral("pdf")));
        QVERIFY(FileUtils::isEncrypedOrUnsupport(pdfPath, QStringLiteral("txt")));
    }

    const QString xmlPath = dir.filePath(QStringLiteral("doc.uof"));
    {
        QFile file(xmlPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("<?xml version=\"1.0\"?><uof></uof>");
    }
    const QString xmlMime = FileUtils::getMimetype(xmlPath).name();
    if (xmlMime == QStringLiteral("application/xml") || xmlMime == QStringLiteral("application/uof")) {
        QVERIFY(!FileUtils::isEncrypedOrUnsupport(xmlPath, QStringLiteral("uof")));
        QVERIFY(FileUtils::isEncrypedOrUnsupport(xmlPath, QStringLiteral("xml")));
    }

    const QString imagePath = dir.filePath(QStringLiteral("ocr.png"));
    QImage image(OCR_MIN_SIZE + 4, OCR_MIN_SIZE + 4, QImage::Format_RGB32);
    image.fill(Qt::white);
    QVERIFY(image.save(imagePath));
    if (FileUtils::getMimetype(imagePath).name().startsWith(QStringLiteral("image/"))) {
        QVERIFY(!FileUtils::isEncrypedOrUnsupport(imagePath, QStringLiteral("png")));
    }

    const QString unknownPath = dir.filePath(QStringLiteral("unknown.bin"));
    {
        QFile file(unknownPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("\x00\x01\x02\x03", 4);
    }
    QVERIFY(FileUtils::isEncrypedOrUnsupport(unknownPath, QStringLiteral("bin")));
}

void FileUtilsTest::testIconAndOpenFileFallbacks()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString missingUri = QUrl::fromLocalFile(dir.filePath(QStringLiteral("missing.file"))).toString();
    QVERIFY(!FileUtils::getFileIcon(missingUri).isNull());
    QVERIFY(!FileUtils::getSettingIcon().isNull());

    QString missingPath = dir.filePath(QStringLiteral("missing.file"));
    if (QDBusConnection::sessionBus().isConnected()) {
        BrokenFileManager1Service brokenService;
        {
            ScopedDbusRegistration brokenRegistration(QStringLiteral("org.freedesktop.FileManager1"),
                                                      QStringLiteral("/org/freedesktop/FileManager1"),
                                                      &brokenService);
            if (brokenRegistration.isRegistered()) {
                QCOMPARE(FileUtils::openFile(missingPath, true), -1);
            }
        }

        FakeFileManager1Service fileManagerService;
        ScopedDbusRegistration fileManagerRegistration(QStringLiteral("org.freedesktop.FileManager1"),
                                                       QStringLiteral("/org/freedesktop/FileManager1"),
                                                       &fileManagerService);
        if (fileManagerRegistration.isRegistered()) {
            QCOMPARE(FileUtils::openFile(missingPath, true), 0);
            QCOMPARE(fileManagerService.lastItems, QStringList{missingPath});
            QCOMPARE(fileManagerService.lastStartupId, QStringLiteral("ukui-search"));
            return;
        }
    }

    QCOMPARE(FileUtils::openFile(missingPath, true), -1);
}

void FileUtilsTest::testOpenFileUsesProcessManagerWhenDefaultAppExists()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("session D-Bus is unavailable in this environment");
    }

    FakeAppLauncher launcher;
    ScopedDbusRegistration launcherRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                &launcher);
    if (!launcherRegistration.isRegistered()) {
        QSKIP("ProcessManager service is already owned in this environment");
    }

    const QString homePath = QDir::homePath();
    if (!homePath.contains(QStringLiteral("test-home"))) {
        QSKIP("requires the isolated test HOME used by runUnitTest.sh");
    }

    const QString configDir = homePath + QStringLiteral("/.config");
    const QString dataHome = qEnvironmentVariable(QStringLiteral("XDG_DATA_HOME").toUtf8().constData(),
                                                  homePath + QStringLiteral("/.local/share"));
    const QString applicationsDir = dataHome + QStringLiteral("/applications");
    QVERIFY(QDir().mkpath(configDir));
    QVERIFY(QDir().mkpath(applicationsDir));

    QFile mimeApps(configDir + QStringLiteral("/mimeapps.list"));
    QVERIFY(mimeApps.open(QIODevice::WriteOnly | QIODevice::Truncate));
    mimeApps.write("[Default Applications]\ntext/plain=ukui-search-open-file.desktop\n");
    mimeApps.close();

    QFile desktopFile(applicationsDir + QStringLiteral("/ukui-search-open-file.desktop"));
    QVERIFY(desktopFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    desktopFile.write("[Desktop Entry]\n"
                      "Type=Application\n"
                      "Name=Fake Open\n"
                      "Exec=/bin/true %u\n"
                      "MimeType=text/plain;\n"
                      "NoDisplay=true\n");
    desktopFile.close();

    const QString filePath = homePath + QStringLiteral("/plain.txt");
    QFile textFile(filePath);
    QVERIFY(textFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    textFile.write("hello");
    textFile.close();

    QString mutablePath = filePath;
    QCOMPARE(FileUtils::openFile(mutablePath, false), 0);
    QCOMPARE(launcher.launchUrlCalls, 1);
    QCOMPARE(launcher.lastUrl, QUrl::fromLocalFile(filePath).toString());
}

QTEST_MAIN(FileUtilsTest)

#include "file-utils-test.moc"
