#include <QTemporaryDir>
#include <QTest>

#include <fstream>

#include "storage-base.hpp"

using namespace std;

namespace {

QString writeDict(const QString &dirPath, const QString &name, const QStringList &lines)
{
    const QString path = dirPath + QLatin1Char('/') + name;
    ofstream out(path.toStdString());
    for (const QString &line : lines) {
        out << line.toStdString() << '\n';
    }
    out.close();
    return path;
}

class TestStorage : public StorageBase<char, false, CacheFileHeaderBase>
{
public:
    TestStorage(const vector<string> &filePaths, const string &cachePath)
        : StorageBase<char, false, CacheFileHeaderBase>(filePaths, cachePath)
    {
    }

    void LoadSourceFile(const string &datCacheFile, const string &md5) override
    {
        CacheFileHeaderBase header;
        memcpy(header.md5_hex, md5.c_str(), md5.size());

        int offset = 0;
        int elementsNum = 0;
        vector<pair<string, string>> entries;
        for (const string &path : sourceFiles) {
            ifstream in(path);
            string key;
            string value;
            while (in >> key >> value) {
                Update(key.c_str(), key.size(), offset);
                offset += value.size() + 1;
                ++elementsNum;
                entries.push_back({key, value});
            }
        }

        header.elements_num = elementsNum;
        header.elements_size = offset;
        header.dat_size = GetDataTrieSize();

        string tmp = datCacheFile + "_XXXXXX";
        int fd = mkstemp(const_cast<char*>(tmp.data()));
        QVERIFY(fd >= 0);
        int written = write(fd, &header, sizeof(header));
        for (const auto &entry : entries) {
            written += write(fd, entry.second.c_str(), entry.second.size() + 1);
        }
        written += write(fd, GetDataTrieArray(), GetDataTrieTotalSize());
        QCOMPARE(static_cast<size_t>(written), sizeof(header) + header.elements_size + GetDataTrieTotalSize());
        close(fd);
        QCOMPARE(rename(tmp.c_str(), datCacheFile.c_str()), 0);
    }

    string FindValue(const string &key) const
    {
        const int offset = ExactMatchSearch(key.c_str(), key.size());
        if (offset < 0) {
            return {};
        }
        return string(GetElementPtr() + offset);
    }

    vector<string> sourceFiles;
};

}  // namespace

class StorageBaseWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void buildsAttachesAndExposesCacheData();
    void rebuildsWhenCacheIsInvalid();
    void ignoresMissingAndEmptySourceFilesForMd5();
    void helperFunctionsCoverRenameAndExistenceBranches();
};

void StorageBaseWhiteboxTest::buildsAttachesAndExposesCacheData()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString dict = writeDict(dir.path(), QStringLiteral("dict.txt"), {
        QStringLiteral("alpha one"),
        QStringLiteral("beta two")
    });
    const string cache = (dir.path() + QStringLiteral("/cache.dat")).toStdString();

    {
        TestStorage storage({dict.toStdString()}, cache);
        storage.sourceFiles = {dict.toStdString()};
        storage.Init();
        QCOMPARE(storage.FindValue("alpha"), string("one"));
        QCOMPARE(storage.FindValue("missing"), string());
        QVERIFY(storage.GetCacheFileHeaderPtr() != nullptr);
        QVERIFY(storage.GetElementPtr() != nullptr);
        QVERIFY(storage.GetDataTrieArray() != nullptr);
        QVERIFY(storage.GetDataTrieSize() > 0);
        QVERIFY(storage.GetDataTrieTotalSize() > 0);
    }

    {
        TestStorage storage({dict.toStdString()}, cache);
        storage.sourceFiles = {dict.toStdString()};
        storage.Init();
        QCOMPARE(storage.FindValue("beta"), string("two"));
    }
}

void StorageBaseWhiteboxTest::rebuildsWhenCacheIsInvalid()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString dict = writeDict(dir.path(), QStringLiteral("dict.txt"), {
        QStringLiteral("alpha one")
    });
    const string cache = (dir.path() + QStringLiteral("/cache.dat")).toStdString();

    {
        ofstream bad(cache);
        bad << "short";
    }

    TestStorage storage({dict.toStdString()}, cache);
    storage.sourceFiles = {dict.toStdString()};
    storage.Init();
    QCOMPARE(storage.FindValue("alpha"), string("one"));

    CacheFileHeaderBase *header = storage.GetCacheFileHeaderPtr();
    QVERIFY(header != nullptr);
    QVERIFY(header->elements_num > 0);
}

void StorageBaseWhiteboxTest::ignoresMissingAndEmptySourceFilesForMd5()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString empty = writeDict(dir.path(), QStringLiteral("empty.txt"), {});
    const QString dict = writeDict(dir.path(), QStringLiteral("dict.txt"), {
        QStringLiteral("gamma three")
    });
    const QString missing = dir.path() + QStringLiteral("/missing.txt");

    int sizeSum = -1;
    const string digest = CalcFileListMD5({missing.toStdString(), empty.toStdString(), dict.toStdString()}, sizeSum);
    QCOMPARE(sizeSum, static_cast<int>(QFileInfo(dict).size()));
    QCOMPARE(digest.size(), static_cast<size_t>(32));
}

void StorageBaseWhiteboxTest::helperFunctionsCoverRenameAndExistenceBranches()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString existing = writeDict(dir.path(), QStringLiteral("existing.txt"), {
        QStringLiteral("alpha one")
    });
    const QString missing = dir.path() + QStringLiteral("/missing.txt");
    QVERIFY(isFileExist(existing.toStdString()));
    QVERIFY(!isFileExist(missing.toStdString()));

    const QString src = writeDict(dir.path(), QStringLiteral("src.txt"), {
        QStringLiteral("beta two")
    });
    const QString dst = dir.path() + QStringLiteral("/dst.txt");
    tryRename(src.toStdString(), dst.toStdString());
    QVERIFY(!isFileExist(src.toStdString()));
    QVERIFY(isFileExist(dst.toStdString()));

    const QString duplicateTmp = writeDict(dir.path(), QStringLiteral("duplicate.tmp"), {
        QStringLiteral("tmp value")
    });
    const QString duplicateDst = writeDict(dir.path(), QStringLiteral("duplicate.dst"), {
        QStringLiteral("dst value")
    });
    tryRename(duplicateTmp.toStdString(), duplicateDst.toStdString());
    QVERIFY(isFileExist(duplicateDst.toStdString()));

    const QString blockedTmp = writeDict(dir.path(), QStringLiteral("blocked.tmp"), {
        QStringLiteral("blocked value")
    });
    const QString existingDir = dir.path() + QStringLiteral("/existing-dir");
    QVERIFY(QDir().mkdir(existingDir));
    QVERIFY(isFileExist(existingDir.toStdString()));
    tryRename(blockedTmp.toStdString(), existingDir.toStdString());
    QVERIFY(!isFileExist(blockedTmp.toStdString()));
}

QTEST_MAIN(StorageBaseWhiteboxTest)

#include "storage-base-whitebox-test.moc"
