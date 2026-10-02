/*
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */
#include "office-extractortest.h"
#include "indexerextractortestsconfig.h"
#include "extractors/office-extractor.h"
#include "simple-extraction-result.h"
#include "mime-utils.h"

#include <QFile>
#include <QProcessEnvironment>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>
#include <QStringList>

using namespace UkuiFileMetadata;

#if defined(__has_feature)
#  if __has_feature(address_sanitizer)
#    define OFFICE_TEST_WITH_ASAN 1
#  endif
#endif

#if defined(__SANITIZE_ADDRESS__)
#  define OFFICE_TEST_WITH_ASAN 1
#endif

#ifndef OFFICE_TEST_WITH_ASAN
#  define OFFICE_TEST_WITH_ASAN 0
#endif

QString testFilePath(const QString& baseName, const QString& extension);

namespace {

QString shellQuote(const QString &value)
{
    QString escaped = value;
    escaped.replace(QLatin1Char('\''), QLatin1String("'\\''"));
    return QLatin1Char('\'') + escaped + QLatin1Char('\'');
}

QString officeHelperPath()
{
    return QCoreApplication::applicationDirPath()
        + QLatin1String("/OfficeExtractorHelper");
}

void runOfficeHelperWithMemoryLimit(const QString &filePath,
                                    int memoryLimitKb = 1048576,
                                    QString *stderrText = nullptr,
                                    QString *stdoutText = nullptr,
                                    bool disableMemoryLimitForAsan = false)
{
    const QString helperPath = officeHelperPath();
    QVERIFY2(QFileInfo::exists(helperPath), qPrintable(helperPath));

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    process.setProcessEnvironment(env);

#if OFFICE_TEST_WITH_ASAN
    const QString command = disableMemoryLimitForAsan
        ? QStringLiteral("exec %1 %2")
              .arg(shellQuote(helperPath), shellQuote(filePath))
        : QStringLiteral("ulimit -v %1; exec %2 %3")
              .arg(QString::number(memoryLimitKb), shellQuote(helperPath), shellQuote(filePath));
#else
    (void)disableMemoryLimitForAsan;
    const QString command = QStringLiteral("ulimit -v %1; exec %2 %3")
        .arg(QString::number(memoryLimitKb), shellQuote(helperPath), shellQuote(filePath));
#endif
    process.start(QStringLiteral("/bin/bash"), QStringList{QStringLiteral("-lc"), command});
    QVERIFY(process.waitForStarted());
    QVERIFY(process.waitForFinished(10000));

    if(stderrText != nullptr) {
        *stderrText = QString::fromLocal8Bit(process.readAllStandardError());
    }
    if(stdoutText != nullptr) {
        *stdoutText = QString::fromLocal8Bit(
            process.readAllStandardOutput());
    }
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), 0);
}

void verifyExtractedText(const QString &filePath,
                         int expectedLength,
                         const QString &expectedSha256)
{
    QString stdoutText;
    runOfficeHelperWithMemoryLimit(filePath,
                                   1048576,
                                   nullptr,
                                   &stdoutText,
                                   true);

    QVERIFY2(stdoutText.contains(
                 QStringLiteral("text_len=%1\n").arg(expectedLength)),
             qPrintable(stdoutText));
    QVERIFY2(stdoutText.contains(
                 QStringLiteral("text_sha256=%1\n").arg(expectedSha256)),
             qPrintable(stdoutText));
}

QByteArray littleEndian32(quint32 value)
{
    QByteArray bytes(4, Qt::Uninitialized);
    bytes[0] = static_cast<char>(value & 0xff);
    bytes[1] = static_cast<char>((value >> 8) & 0xff);
    bytes[2] = static_cast<char>((value >> 16) & 0xff);
    bytes[3] = static_cast<char>((value >> 24) & 0xff);
    return bytes;
}

void patchOfficeFile(const QString &targetPath,
                     qsizetype offset,
                     const QByteArray &payload,
                     const QByteArray &expectedOriginal = {})
{
    QFile file(targetPath);
    QVERIFY(file.open(QIODevice::ReadWrite));
    QVERIFY(offset >= 0);
    QVERIFY(offset + payload.size() <= file.size());

    if (!expectedOriginal.isEmpty()) {
        QVERIFY(file.seek(offset));
        QCOMPARE(file.read(expectedOriginal.size()),
                 expectedOriginal);
    }

    QVERIFY(file.seek(offset));
    QCOMPARE(file.write(payload), qint64(payload.size()));
}

void createPatchedOfficeFile(const QString &targetPath,
                             qsizetype offset,
                             const QByteArray &payload,
                             const QString &extension = QStringLiteral("doc"),
                             const QByteArray &expectedOriginal = {},
                             const QString &sourceBaseName = QStringLiteral("test"))
{
    const QString sourcePath = testFilePath(sourceBaseName, extension);
    QVERIFY2(QFile::copy(sourcePath, targetPath), qPrintable(sourcePath));
    patchOfficeFile(targetPath, offset, payload, expectedOriginal);
}

void createResizedOfficeFile(const QString &targetPath, qint64 finalSize)
{
    const QString sourcePath = testFilePath(QStringLiteral("test"), QStringLiteral("doc"));
    QVERIFY2(QFile::copy(sourcePath, targetPath), qPrintable(sourcePath));

    QFile file(targetPath);
    QVERIFY(file.open(QIODevice::ReadWrite));
    QVERIFY2(file.resize(finalSize), qPrintable(file.errorString()));
}

quint32 readLittleEndian32(QFile &file, qint64 offset)
{
    Q_ASSERT(offset >= 0);
    QByteArray bytes(4, Qt::Uninitialized);
    if (!file.seek(offset) || file.read(bytes.data(), 4) != 4) {
        return 0;
    }
    return static_cast<quint32>(static_cast<quint8>(bytes[0]))
        | (static_cast<quint32>(static_cast<quint8>(bytes[1])) << 8)
        | (static_cast<quint32>(static_cast<quint8>(bytes[2])) << 16)
        | (static_cast<quint32>(static_cast<quint8>(bytes[3])) << 24);
}

}

QString testFilePath(const QString& baseName, const QString& extension)
{
    return QLatin1String(INDEXER_TESTS_OFFICE_FILES_PATH)
        + QLatin1Char('/') + baseName + QLatin1Char('.') + extension;
}

void OfficeExtractorTest::testContent_data()
{
    QTest::addColumn<QString>("fileType");

    QTest::addRow("Doc")
        << QStringLiteral("doc");

    QTest::addRow("WPS")
        << QStringLiteral("wps");

    QTest::addRow("Dot")
        << QStringLiteral("dot");

    QTest::addRow("Xls")
        << QStringLiteral("xls");

    QTest::addRow("Et")
        << QStringLiteral("et");

    QTest::addRow("Dps")
        << QStringLiteral("dps");

    QTest::addRow("PPT")
        << QStringLiteral("ppt");

    QTest::addRow("PPS")
        << QStringLiteral("pps");
}

void OfficeExtractorTest::testContent()
{
    QFETCH(QString, fileType);

    OfficeExtractor plugin{this};

    QMimeDatabase mimeDb;
    QString fileName = testFilePath(QStringLiteral("test"), fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    QCOMPARE(result.text(), QStringLiteral("hwf "));
}

void OfficeExtractorTest::testControlCharacterNormalization_data()
{
    QTest::addColumn<QString>("fileType");
    QTest::addColumn<QStringList>("expectedContains");
    QTest::addColumn<QStringList>("expectedNotContains");

    QTest::addRow("Doc")
        << QStringLiteral("doc")
        << (QStringList{}
            << QStringLiteral("DOC_VISIBLE")
            << QStringLiteral("DOC_SOFTHYPHEN")
            << QStringLiteral("DOC_NON-BREAK")
            << QStringLiteral("DOC_VTA")
            << QStringLiteral("VTB")
            << QStringLiteral("DOC_FORMA")
            << QStringLiteral("FORMB")
            << QStringLiteral("DOC_COLA")
            << QStringLiteral("COLB")
            << QStringLiteral("DOC_TABLE_A DOC_TABLE_B"))
        << (QStringList{}
            << QString(QChar(0x001F))
            << QStringLiteral("DOC_SOFT-HYPHEN")
            << QStringLiteral("DOC_SOFTQHYPHEN")
            << QStringLiteral("DOC_NONQBREAK")
            << QStringLiteral("DOC_VTAQVTB")
            << QStringLiteral("DOC_VTAVTB")
            << QStringLiteral("DOC_FORMAQFORMB")
            << QStringLiteral("DOC_FORMAFORMB")
            << QStringLiteral("DOC_COLAQCOLB")
            << QStringLiteral("DOC_COLACOLB"));

    QTest::addRow("Xls")
        << QStringLiteral("xls")
        << (QStringList{}
            << QStringLiteral("A\rB"))
        << (QStringList{}
            << QString(QChar(0x000B))
            << QStringLiteral("AB"));

    QTest::addRow("PPT")
        << QStringLiteral("ppt")
        << (QStringList{}
            << QStringLiteral("PPT_VISIBLE_中")
            << QStringLiteral("PPT_DROPA")
            << QStringLiteral("DROPB_中")
            << QStringLiteral("PPT_SEPA")
            << QStringLiteral("SEPB_中")
            << QStringLiteral("PPT_FORMA")
            << QStringLiteral("FORMB_中"))
        << (QStringList{}
            << QString(QChar(0x0002))
            << QStringLiteral("PPT_DROPAQDROPB_中")
            << QStringLiteral("PPT_SEPAQSEPB_中")
            << QStringLiteral("PPT_SEPASEPB_中")
            << QStringLiteral("PPT_FORMAQFORMB_中")
            << QStringLiteral("PPT_FORMAFORMB_中"));
}

void OfficeExtractorTest::testControlCharacterNormalization()
{
    QFETCH(QString, fileType);
    QFETCH(QStringList, expectedContains);
    QFETCH(QStringList, expectedNotContains);

    OfficeExtractor plugin{this};

    QMimeDatabase mimeDb;
    QString fileName = testFilePath(QStringLiteral("control-chars"), fileType);
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(plugin.getSupportedMimeType(mimeType)));

    SimpleExtractionResult result(fileName, mimeType);
    plugin.extract(&result);

    const QString text = result.text();
    for (const QString &expected : expectedContains) {
        QVERIFY2(text.contains(expected),
                 qPrintable(QStringLiteral("Expected extracted text to contain '%1', got '%2'")
                            .arg(expected, text)));
    }
    for (const QString &unexpected : expectedNotContains) {
        QVERIFY2(!text.contains(unexpected),
                 qPrintable(QStringLiteral("Expected extracted text not to contain '%1', got '%2'")
                            .arg(unexpected, text)));
    }
}

void OfficeExtractorTest::testRejectsOversizedClx()
{
    static const int kFibClxOffset = 0x1a6;
    static const QByteArray kOversizedClx = QByteArray::fromHex("00000001");
    static const QList<QByteArray> kFibSignatures = {
        QByteArray::fromHex("9880"),
        QByteArray::fromHex("9980"),
        QByteArray::fromHex("dca5"),
        QByteArray::fromHex("eca5"),
        QByteArray::fromHex("97a6"),
        QByteArray::fromHex("99a6"),
    };

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = testFilePath(QStringLiteral("test"), QStringLiteral("doc"));
    const QString targetPath = tempDir.filePath(QStringLiteral("oversized-clx.doc"));
    QVERIFY2(QFile::copy(sourcePath, targetPath), qPrintable(sourcePath));

    QFile file(targetPath);
    QVERIFY(file.open(QIODevice::ReadWrite));

    const QByteArray content = file.readAll();
    QVERIFY(!content.isEmpty());

    int fibOffset = -1;
    for (const QByteArray &signature : kFibSignatures) {
        fibOffset = content.indexOf(signature);
        if (fibOffset >= 0) {
            break;
        }
    }

    QVERIFY2(fibOffset >= 0, "Failed to locate the WordDocument FIB header");
    QVERIFY(fibOffset + kFibClxOffset + kOversizedClx.size() <= content.size());

    file.seek(fibOffset + kFibClxOffset);
    QCOMPARE(file.write(kOversizedClx), qint64(kOversizedClx.size()));
    file.close();

    const QString helperPath = QCoreApplication::applicationDirPath()
        + QLatin1String("/OfficeExtractorHelper");
    QVERIFY2(QFileInfo::exists(helperPath), qPrintable(helperPath));

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("GCOV_PREFIX"), tempDir.filePath(QStringLiteral("gcov")));
    env.insert(QStringLiteral("GCOV_PREFIX_STRIP"), QStringLiteral("0"));
    process.setProcessEnvironment(env);
    process.start(helperPath, QStringList{targetPath});
    QVERIFY(process.waitForStarted());
    QVERIFY(process.waitForFinished(10000));

    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), 0);
}

void OfficeExtractorTest::testRejectsOversizedBbdCount()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString targetPath = tempDir.filePath(QStringLiteral("oversized-bbd-count.doc"));
    createPatchedOfficeFile(targetPath, 0x2c, littleEndian32(0x20000000u));

    // 该用例验证的是 BBD 数量的显式上限，不依赖 RLIMIT_AS；ASan 下不
    // 设置该限制，以免 shadow memory 的虚拟地址空间预约导致误失败。
    runOfficeHelperWithMemoryLimit(targetPath,
                                   1048576,
                                   nullptr,
                                   nullptr,
                                   true);
}

void OfficeExtractorTest::testRejectsForgedSbdLength()
{
#if OFFICE_TEST_WITH_ASAN
    QSKIP("This regression relies on RLIMIT_AS, which is incompatible with ASan shadow memory.");
#endif
    // OLE 头部常量：BIG_BLOCK_SIZE=512，根目录起始块号在头部 0x30 处；
    // root entry 内偏移 0x78 存的是 small-block 流的字节长度，解析时会被
    // 除以 SMALL_BLOCK_SIZE(64) 得到 tSBDLen，再用于 xcalloc(tSBDLen, 8)。
    //
    // 该字段是 32 位，伪造的最大值 0xFFFFFFFF 会让解析尝试分配
    // (0xFFFFFFFF / 64) * sizeof(ULONG) 字节。ULONG 在工程里定义为
    // unsigned long，因此 32 位和 64 位平台上的申请大小不同。
    // 为了让本用例真正锁住回归（而不是无论有无防护都通过），把 helper 的
    // 虚拟内存限制设为这次伪造申请量的 3/4：
    //   - 正常解析 test.doc 只需不到 100 MiB，不受影响；
    //   - 这次伪造的分配在不同字长平台上都会失败。
    // 若“字段一致性校验”和“分配判空”任一存在，解析会优雅返回；两者都被
    // 移除时，xcalloc 失败后的空指针访问会让进程 SIGSEGV，用例随即失败。
    static const qint64 kBigBlockSize = 512;
    static const qint64 kRootStartOffset = 0x30;
    static const qint64 kSbdSizeFieldOffset = 0x78;
    static const quint32 kForgedStreamBytes = 0xFFFFFFFFu;
    const quint64 forgedAllocationBytes =
        (static_cast<quint64>(kForgedStreamBytes) / 64) * sizeof(unsigned long);
    const int kMemoryLimitKb = static_cast<int>(forgedAllocationBytes * 3 / 4 / 1024);

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString sourcePath = testFilePath(QStringLiteral("test"), QStringLiteral("doc"));
    const QString targetPath = tempDir.filePath(QStringLiteral("forged-sbd-length.doc"));
    QVERIFY2(QFile::copy(sourcePath, targetPath), qPrintable(sourcePath));

    QFile file(targetPath);
    QVERIFY(file.open(QIODevice::ReadWrite));

    const quint32 rootStart = readLittleEndian32(file, kRootStartOffset);
    const qint64 sbdSizeOffset =
        (static_cast<qint64>(rootStart) + 1) * kBigBlockSize + kSbdSizeFieldOffset;
    QVERIFY2(sbdSizeOffset + 4 <= file.size(),
             "Patched SBD length field falls outside the sample file");

    QVERIFY(file.seek(sbdSizeOffset));
    QCOMPARE(file.write(littleEndian32(kForgedStreamBytes)), qint64(4));
    file.close();

    runOfficeHelperWithMemoryLimit(targetPath, kMemoryLimitKb);
}

void OfficeExtractorTest::testRejectsSparseOleWithInsufficientFat()
{
#if OFFICE_TEST_WITH_ASAN
    QSKIP("This regression relies on RLIMIT_AS, which is incompatible with ASan shadow memory.");
#endif
    static const qint64 kSparseOleSize = 5ll * 1024 * 1024 * 1024;
    static const int kMemoryLimitKb = 393216;

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString targetPath = tempDir.filePath(QStringLiteral("sparse-insufficient-fat.doc"));
    createResizedOfficeFile(targetPath, kSparseOleSize);

    QString stderrText;
    runOfficeHelperWithMemoryLimit(targetPath, kMemoryLimitKb, &stderrText);
    QVERIFY2(stderrText.contains(QStringLiteral("Rejecting inconsistent OLE FAT")),
             qPrintable(stderrText));
}

void OfficeExtractorTest::testRejectsOversizedOleDepotAllocation()
{
#if OFFICE_TEST_WITH_ASAN
    QSKIP("This regression relies on RLIMIT_AS, which is incompatible with ASan shadow memory.");
#endif
    if(sizeof(long) < 8) {
        QSKIP("This regression path needs long to represent a sparse OLE file larger than 4 GiB.");
    }

    static const qint64 kSparseOleSize = 5ll * 1024 * 1024 * 1024;
    static const qint64 kBigBlockSize = 512;
    static const qint64 kBbdCountOffset = 0x2c;
    static const int kMemoryLimitKb = 393216;

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString targetPath = tempDir.filePath(QStringLiteral("oversized-depot-allocation.doc"));
    createResizedOfficeFile(targetPath, kSparseOleSize);

    QFile file(targetPath);
    QVERIFY(file.open(QIODevice::ReadWrite));

    const quint64 bbdLen = static_cast<quint64>(kSparseOleSize / kBigBlockSize - 1);
    const quint32 bbdBlocks = static_cast<quint32>((bbdLen + 127) / 128);
    QVERIFY(file.seek(kBbdCountOffset));
    QCOMPARE(file.write(littleEndian32(bbdBlocks)), qint64(4));
    file.close();

    QString stderrText;
    runOfficeHelperWithMemoryLimit(targetPath, kMemoryLimitKb, &stderrText);
    QVERIFY2(stderrText.contains(QStringLiteral("Rejecting oversized OLE depot allocation")),
             qPrintable(stderrText));
}

void OfficeExtractorTest::testRejectsSstStringWithoutNullTerminator()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // test.xls stores its Workbook stream in the first big-block chain.  The
    // SST record starts at this physical file offset; patch only the flag and
    // three compressed characters, keeping the OLE structure intact.
    static const qsizetype kSstStringFlagOffset = 0x285e;
    const QString targetPath =
        tempDir.filePath(QStringLiteral("sst-no-null.xls"));
    createPatchedOfficeFile(targetPath,
                            kSstStringFlagOffset,
                            QByteArray::fromHex("00414243"),
                            QStringLiteral("xls"),
                            QByteArray::fromHex("01680077"));

    // The compressed BIFF string is deliberately not NUL terminated.  The
    // helper must still exit normally; the old QString(const char *) call
    // scans beyond the three-byte heap allocation under ASan.
    // fHighByte==0 只表示每字符 1 字节，不表示"不支持"：压缩字符串
    // 必须和未压缩字符串一样进入提取结果。这里期望正好是 "ABC"
    // （SimpleExtractionResult::append() 会再追加一个尾部空格）。
    verifyExtractedText(targetPath,
                        4,
                        QStringLiteral("c1663e290ddbef3bd6e1f7b153e4680d"
                                       "886956dec90bf8f2ca20c2da38b0328b"));
}

void OfficeExtractorTest::testRejectsOversizedSstRecord()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString targetPath =
        tempDir.filePath(QStringLiteral("sst-oversized-record.xls"));
    createPatchedOfficeFile(targetPath,
                            0x0a16,
                            QByteArray::fromHex(
                                "212001000000010000000b10"),
                            QStringLiteral("xls"),
                            QByteArray::fromHex(
                                "1f200100000001000000ffff"),
                            QStringLiteral("large-sst"));

    QString stderrText;
    runOfficeHelperWithMemoryLimit(targetPath,
                                   1048576,
                                   &stderrText,
                                   nullptr,
                                   true);
    QVERIFY2(stderrText.contains(
                 QStringLiteral("Invalid SST record stream")),
             qPrintable(stderrText));
}

void OfficeExtractorTest::testRejectsOversizedSstContinueRecord()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString targetPath =
        tempDir.filePath(QStringLiteral("sst-oversized-continue.xls"));
    createPatchedOfficeFile(targetPath,
                            0x0a20,
                            QByteArray::fromHex("1a20"),
                            QStringLiteral("xls"),
                            QByteArray::fromHex("ffff"),
                            QStringLiteral("large-sst"));
    patchOfficeFile(targetPath,
                    0x2a39,
                    QByteArray::fromHex("2120"),
                    QByteArray::fromHex("ff1f"));

    QString stderrText;
    runOfficeHelperWithMemoryLimit(targetPath,
                                   1048576,
                                   &stderrText,
                                   nullptr,
                                   true);
    QVERIFY2(stderrText.contains(
                 QStringLiteral("Invalid SST record stream")),
             qPrintable(stderrText));
}

void OfficeExtractorTest::testRejectsInvalidSstContinueOption()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // 第一个字符串的 rgRun 跨到 CONTINUE；续段首字节按 MS-XLS
    // 必须是 fHighByte（0 或 1），这里使用非法值 2 验证解析失败。
    const QString targetPath =
        tempDir.filePath(QStringLiteral("sst-invalid-continue-option.xls"));
    createPatchedOfficeFile(targetPath,
                            0x2850,
                            QByteArray::fromHex(
                                "fc00100002000000020000000200080100"
                                "4142003c0008000200010001000043"),
                            QStringLiteral("xls"),
                            QByteArray());

    QString stderrText;
    runOfficeHelperWithMemoryLimit(targetPath,
                                   1048576,
                                   &stderrText,
                                   nullptr,
                                   true);
    QVERIFY2(stderrText.contains(
                 QStringLiteral("Invalid SST record stream")),
             qPrintable(stderrText));
}

void OfficeExtractorTest::testRejectsTrailingSstPayload()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // test.xls 的 SST 正好在一个字符串后结束。把记录长度增加一字节，
    // 验证声明的字符串全部读完后，解析器不会接受尾随垃圾。
    const QString targetPath =
        tempDir.filePath(QStringLiteral("sst-trailing-payload.xls"));
    createPatchedOfficeFile(targetPath,
                            0x2852,
                            QByteArray::fromHex("1200"),
                            QStringLiteral("xls"),
                            QByteArray::fromHex("1100"));

    QString stderrText;
    runOfficeHelperWithMemoryLimit(targetPath,
                                   1048576,
                                   &stderrText,
                                   nullptr,
                                   true);
    QVERIFY2(stderrText.contains(
                 QStringLiteral("Invalid SST record stream")),
             qPrintable(stderrText));
}

void OfficeExtractorTest::testReadsLargeSstAcrossContinueRecords()
{
    const QString filePath =
        testFilePath(QStringLiteral("large-sst"),
                     QStringLiteral("xls"));

    verifyExtractedText(filePath,
                        65536,
                        QStringLiteral("82275f7c40534d0541c20e88c65b77df"
                                       "377a468e0188a31c252cd97e756d0fe3"));
}

void OfficeExtractorTest::testReadsSstTrailingDataAcrossContinue()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // 第一个字符串的 rich-text rgRun 在 SST 末尾只保留一个字节；其余
    // 三字节及第二个字符串位于 CONTINUE。按 MS-XLS，CONTINUE 首字节是
    // fHighByte，必须在跳过 rgRun 前消费。
    const QString targetPath = tempDir.filePath(QStringLiteral("sst-tail-continue.xls"));
    createPatchedOfficeFile(targetPath,
                            0x2850,
                            QByteArray::fromHex(
                                "fc00100002000000020000000200080100"
                                "4142003c0008000000000001000043"),
                            QStringLiteral("xls"),
                            QByteArray());

    verifyExtractedText(targetPath,
                        5,
                        QStringLiteral("3b160671464092076163d204ade1e187"
                                       "62698d133d66d8dac7365413281db9df"));
}

void OfficeExtractorTest::testReadsExtRstAcrossContinue()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // 第一个字符串的 ExtRst 在 SST 末尾只保留一个字节；CONTINUE 先给出
    // fHighByte，再保存其余七字节 ExtRst 和第二个完整字符串。
    const QString targetPath =
        tempDir.filePath(QStringLiteral("sst-extrst-continue.xls"));
    createPatchedOfficeFile(targetPath,
                            0x2850,
                            QByteArray::fromHex(
                                "fc001200020000000200000002000408000000"
                                "4142013c000c00000000000000000001000043"),
                            QStringLiteral("xls"),
                            QByteArray());

    verifyExtractedText(targetPath,
                        5,
                        QStringLiteral("3b160671464092076163d204ade1e187"
                                       "62698d133d66d8dac7365413281db9df"));
}

void OfficeExtractorTest::testReadsNextSstStringFromContinue()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // 第一个字符串 "AB" 恰好结束于 SST 记录末尾；CONTINUE 从下一完整
    // 字符串 "C" 的 cch 开始，因此其首字节不是 fHighByte。
    const QString targetPath = tempDir.filePath(QStringLiteral("sst-next-string-continue.xls"));
    createPatchedOfficeFile(targetPath,
                            0x2850,
                            QByteArray::fromHex(
                                "fc000d0002000000020000000200004142"
                                "3c00040001000043"),
                            QStringLiteral("xls"),
                            QByteArray());

    verifyExtractedText(targetPath,
                        5,
                        QStringLiteral("3b160671464092076163d204ade1e187"
                                       "62698d133d66d8dac7365413281db9df"));
}

QTEST_GUILESS_MAIN(OfficeExtractorTest)
