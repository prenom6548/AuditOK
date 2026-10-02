/*
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#include "zip-reader.h"

#include <QFile>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif
#include <algorithm>
#include <limits>
#include <utility>

#include <minizip/unzip.h>
#include <zlib.h>

namespace UkuiFileMetadata {

namespace {

constexpr int kReadBufferSize = 8192;
constexpr uLong kUtf8NameFlag = 1u << 11;
constexpr quint16 kUnicodePathExtraFieldId = 0x7075;
constexpr quint8 kUnicodePathExtraFieldVersion = 1;

struct EntryRecord
{
    QByteArray rawName;
    QString decodedName;
    unz64_file_pos position {};
    quint64 uncompressedSize = 0;
    uLong flags = 0;
};

void appendUniqueCodecName(QList<QByteArray> *codecNames, const QByteArray &codecName)
{
    if (codecNames == nullptr) {
        return;
    }

    const QByteArray trimmedName = codecName.trimmed();
    if (trimmedName.isEmpty()) {
        return;
    }

    if (!codecNames->contains(trimmedName)) {
        codecNames->append(trimmedName);
    }
}

QList<QByteArray> normalizeCodecNames(const QList<QByteArray> &codecNames)
{
    QList<QByteArray> normalizedNames;
    for (const QByteArray &codecName : codecNames) {
        appendUniqueCodecName(&normalizedNames, codecName);
    }
    return normalizedNames;
}

QByteArray systemCodecName()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const char *codecName = QStringConverter::nameForEncoding(QStringConverter::System);
    return codecName ? QByteArray(codecName) : QByteArray();
#else
    const QTextCodec *localeCodec = QTextCodec::codecForLocale();
    return localeCodec ? localeCodec->name() : QByteArray();
#endif
}

QList<QByteArray> defaultFallbackCodecNames()
{
    QList<QByteArray> codecNames;
    appendUniqueCodecName(&codecNames, QByteArrayLiteral("CP437"));
    return codecNames;
}

quint16 readLittleEndian16(const uchar *data)
{
    return static_cast<quint16>(data[0])
           | (static_cast<quint16>(data[1]) << 8);
}

quint32 readLittleEndian32(const uchar *data)
{
    return static_cast<quint32>(data[0])
           | (static_cast<quint32>(data[1]) << 8)
           | (static_cast<quint32>(data[2]) << 16)
           | (static_cast<quint32>(data[3]) << 24);
}

quint32 rawNameCrc32(const QByteArray &rawName)
{
    const auto *bytes = reinterpret_cast<const Bytef *>(rawName.constData());
    return crc32(0L, bytes, static_cast<uInt>(rawName.size()));
}

bool tryDecodeWithCodec(const QByteArray &rawName, const char *codecName, QString *decodedName)
{
    if (codecName == nullptr || decodedName == nullptr) {
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder(codecName);
    if (!decoder.isValid()) {
        return false;
    }

    const QString text = decoder(rawName);
    if (decoder.hasError()) {
        return false;
    }

    QStringEncoder encoder(codecName);
    if (!encoder.isValid()) {
        return false;
    }

    const QByteArray roundTrip = encoder(text);
    if (roundTrip != rawName) {
        return false;
    }
#else
    QTextCodec *codec = QTextCodec::codecForName(codecName);
    if (codec == nullptr) {
        return false;
    }

    QTextCodec::ConverterState decodeState;
    const QString text = codec->toUnicode(rawName.constData(), rawName.size(), &decodeState);
    if (decodeState.invalidChars != 0) {
        return false;
    }

    if (codec->fromUnicode(text) != rawName) {
        return false;
    }
#endif

    *decodedName = text;
    return true;
}

bool tryDecodeUnicodePathExtraField(const QByteArray &rawName,
                                    const QByteArray &extraField,
                                    QString *decodedName)
{
    if (decodedName == nullptr || extraField.isEmpty()) {
        return false;
    }

    const auto *extraBytes = reinterpret_cast<const uchar *>(extraField.constData());
    int offset = 0;
    while (offset + 4 <= extraField.size()) {
        const quint16 headerId = readLittleEndian16(extraBytes + offset);
        const quint16 dataSize = readLittleEndian16(extraBytes + offset + 2);
        offset += 4;

        if (offset + dataSize > extraField.size()) {
            break;
        }

        if (headerId == kUnicodePathExtraFieldId) {
            if (dataSize < 5 || extraBytes[offset] != kUnicodePathExtraFieldVersion) {
                offset += dataSize;
                continue;
            }

            const quint32 storedNameCrc = readLittleEndian32(extraBytes + offset + 1);
            if (storedNameCrc != rawNameCrc32(rawName)) {
                offset += dataSize;
                continue;
            }

            const QByteArray utf8Name(reinterpret_cast<const char *>(extraBytes + offset + 5),
                                      dataSize - 5);
            return tryDecodeWithCodec(utf8Name, "UTF-8", decodedName);
        }

        offset += dataSize;
    }

    return false;
}

class EntryDevice final : public QIODevice
{
public:
    explicit EntryDevice(unzFile archiveHandle, quint64 uncompressedSizeValue)
        : archive(archiveHandle)
        , uncompressedSize(uncompressedSizeValue)
    {
    }

    bool isSequential() const override
    {
        return true;
    }

    qint64 size() const override
    {
        if (uncompressedSize > static_cast<quint64>(std::numeric_limits<qint64>::max())) {
            return -1;
        }
        return static_cast<qint64>(uncompressedSize);
    }

    bool reachedEnd() const
    {
        return endReached;
    }

    bool hasReadError() const
    {
        return readError;
    }

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        if (archive == nullptr || maxSize <= 0) {
            return 0;
        }

        const auto chunkSize = static_cast<unsigned>(std::min<qint64>(
            maxSize, static_cast<qint64>(std::numeric_limits<unsigned>::max())));
        const int bytesRead = unzReadCurrentFile(archive, data, chunkSize);
        if (bytesRead < 0) {
            readError = true;
            setErrorString(QStringLiteral("Failed to read ZIP entry data"));
            return -1;
        }

        if (bytesRead == 0) {
            endReached = true;
        }

        return bytesRead;
    }

    qint64 writeData(const char *, qint64) override
    {
        return -1;
    }

private:
    unzFile archive = nullptr;
    quint64 uncompressedSize = 0;
    bool endReached = false;
    bool readError = false;
};

}

class ZipReader::Private
{
public:
    explicit Private(QString archivePathValue)
        : archivePath(std::move(archivePathValue))
        , fallbackCodecs(defaultFallbackCodecNames())
    {
    }

    ~Private()
    {
        close();
    }

    void setArchivePath(const QString &archivePathValue)
    {
        if (archivePath == archivePathValue) {
            return;
        }

        close();
        archivePath = archivePathValue;
    }

    QList<QByteArray> fallbackFileNameCodecs() const
    {
        return fallbackCodecs;
    }

    void setFallbackFileNameCodecs(const QList<QByteArray> &codecNames)
    {
        const QList<QByteArray> normalizedCodecs = normalizeCodecNames(codecNames);
        if (fallbackCodecs == normalizedCodecs) {
            return;
        }

        close();
        fallbackCodecs = normalizedCodecs;
    }

    bool open()
    {
        if (archive != nullptr) {
            return true;
        }

        if (archivePath.isEmpty()) {
            return false;
        }

        const QByteArray encodedPath = QFile::encodeName(archivePath);
        auto *encodedArchivePath = const_cast<char *>(encodedPath.constData());
        archive = unzOpen64(encodedArchivePath);
        if (archive == nullptr) {
            archive = unzOpen(encodedArchivePath);
        }
        if (archive == nullptr) {
            return false;
        }

        entryRecordsCached = false;
        entryRecords.clear();

        const int result = unzGoToFirstFile(archive);
        if (result != UNZ_OK && result != UNZ_END_OF_LIST_OF_FILE) {
            close();
            return false;
        }

        return true;
    }

    bool isOpen() const
    {
        return archive != nullptr;
    }

    void close()
    {
        if (archive != nullptr) {
            unzClose(archive);
            archive = nullptr;
        }

        entryRecordsCached = false;
        entryRecords.clear();
    }

    QStringList entryNamesList()
    {
        if (!rebuildEntryRecords()) {
            return {};
        }

        QStringList entryNames;
        entryNames.reserve(entryRecords.size());
        for (const EntryRecord &entryRecord : entryRecords) {
            entryNames.append(entryRecord.decodedName);
        }
        return entryNames;
    }

    bool readEntry(const QString &entryName, QByteArray *data,
                   Qt::CaseSensitivity caseSensitivity)
    {
        if (data == nullptr) {
            return false;
        }

        data->clear();
        return processEntry(entryName,
                            [data](QIODevice *device) {
                                const qint64 entrySize = device->size();
                                if (entrySize > 0
                                    && entrySize <= static_cast<qint64>(std::numeric_limits<int>::max())) {
                                    data->reserve(static_cast<int>(entrySize));
                                }

                                while (true) {
                                    const QByteArray chunk = device->read(kReadBufferSize);
                                    if (chunk.isEmpty()) {
                                        break;
                                    }
                                    data->append(chunk);
                                }
                                return true;
                            }, caseSensitivity);
    }

    bool processEntry(const QString &entryName, const std::function<bool(QIODevice *device)> &processor,
                      Qt::CaseSensitivity caseSensitivity)
    {
        unz_file_info64 fileInfo {};
        if (!processor || !openEntry(entryName, caseSensitivity, &fileInfo)) {
            return false;
        }

        EntryDevice device(archive, fileInfo.uncompressed_size);
        device.open(QIODevice::ReadOnly);

        const bool processed = processor(&device) && !device.hasReadError();

        device.close();
        const bool closed = closeCurrentEntry(device.reachedEnd());
        return processed && closed;
    }

    QString decodeEntryName(const QByteArray &rawName,
                            const QByteArray &extraField,
                            uLong flags) const
    {
        QString decodedName;
        if (tryDecodeUnicodePathExtraField(rawName, extraField, &decodedName)) {
            return decodedName;
        }

        if ((flags & kUtf8NameFlag) != 0
            && tryDecodeWithCodec(rawName, "UTF-8", &decodedName)) {
            return decodedName;
        }

        const QByteArray localeCodecName = systemCodecName();
        if (!localeCodecName.isEmpty() && tryDecodeWithCodec(rawName, localeCodecName.constData(), &decodedName)) {
            return decodedName;
        }

        for (const QByteArray &codecName : fallbackCodecs) {
            if (tryDecodeWithCodec(rawName, codecName.constData(), &decodedName)) {
                return decodedName;
            }
        }

        return QString::fromLatin1(rawName.toHex());
    }

    const EntryRecord *findEntryRecord(const QString &entryName,
                                       Qt::CaseSensitivity caseSensitivity)
    {
        if (!rebuildEntryRecords()) {
            return nullptr;
        }

        const auto it = std::find_if(entryRecords.cbegin(), entryRecords.cend(),
                                     [&entryName, caseSensitivity](const EntryRecord &entryRecord) {
                                         return QString::compare(entryRecord.decodedName, entryName,
                                                                 caseSensitivity) == 0;
                                     });
        if (it == entryRecords.cend()) {
            return nullptr;
        }

        return &(*it);
    }

    bool openEntry(const QString &entryName, Qt::CaseSensitivity caseSensitivity, unz_file_info64 *fileInfo)
    {
        if (fileInfo == nullptr || !open()) {
            return false;
        }

        const EntryRecord *entryRecord = findEntryRecord(entryName, caseSensitivity);
        if (entryRecord == nullptr) {
            return false;
        }

        if (unzGoToFilePos64(archive, &entryRecord->position) != UNZ_OK) {
            return false;
        }

        if (unzGetCurrentFileInfo64(archive, fileInfo, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK) {
            return false;
        }

        return unzOpenCurrentFile(archive) == UNZ_OK;
    }

    bool closeCurrentEntry(bool requireCompleteRead = true)
    {
        const int closeResult = unzCloseCurrentFile(archive);
        return !requireCompleteRead || closeResult == UNZ_OK;
    }

    bool rebuildEntryRecords()
    {
        if (entryRecordsCached) {
            return true;
        }

        if (!open()) {
            return false;
        }

        unz64_file_pos savedPosition {};
        const bool hasSavedPosition = unzGetFilePos64(archive, &savedPosition) == UNZ_OK;

        QList<EntryRecord> refreshedEntryRecords;
        int result = unzGoToFirstFile(archive);
        while (result == UNZ_OK) {
            EntryRecord entryRecord;
            if (!currentEntryRecord(&entryRecord)) {
                return false;
            }
            refreshedEntryRecords.append(entryRecord);
            result = unzGoToNextFile(archive);
        }

        if (result != UNZ_END_OF_LIST_OF_FILE) {
            return false;
        }

        if (hasSavedPosition) {
            unzGoToFilePos64(archive, &savedPosition);
        } else {
            unzGoToFirstFile(archive);
        }

        entryRecords = std::move(refreshedEntryRecords);
        entryRecordsCached = true;
        return true;
    }

    bool currentEntryRecord(EntryRecord *entryRecord) const
    {
        if (archive == nullptr || entryRecord == nullptr) {
            return false;
        }

        unz_file_info64 fileInfo {};
        if (unzGetCurrentFileInfo64(archive, &fileInfo, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK) {
            return false;
        }

        QByteArray rawName(static_cast<int>(fileInfo.size_filename) + 1, '\0');
        QByteArray extraField(static_cast<int>(fileInfo.size_file_extra), '\0');
        if (unzGetCurrentFileInfo64(archive,
                                    nullptr,
                                    rawName.data(),
                                    rawName.size(),
                                    extraField.isEmpty() ? nullptr : extraField.data(),
                                    extraField.size(),
                                    nullptr,
                                    0)
            != UNZ_OK) {
            return false;
        }

        if (unzGetFilePos64(archive, &entryRecord->position) != UNZ_OK) {
            return false;
        }

        rawName.chop(1);
        entryRecord->rawName = rawName;
        entryRecord->decodedName = decodeEntryName(entryRecord->rawName, extraField, fileInfo.flag);
        entryRecord->uncompressedSize = fileInfo.uncompressed_size;
        entryRecord->flags = fileInfo.flag;
        return true;
    }

    QString archivePath;
    unzFile archive = nullptr;
    QList<EntryRecord> entryRecords;
    bool entryRecordsCached = false;
    QList<QByteArray> fallbackCodecs;
};

ZipReader::ZipReader(QString archivePath)
    : d(new Private(std::move(archivePath)))
{
}

ZipReader::~ZipReader()
{
}

void ZipReader::setArchivePath(const QString &archivePath)
{
    d->setArchivePath(archivePath);
}

QString ZipReader::archivePath() const
{
    return d->archivePath;
}

QList<QByteArray> ZipReader::fallbackFileNameCodecs() const
{
    return d->fallbackFileNameCodecs();
}

void ZipReader::setFallbackFileNameCodecs(const QList<QByteArray> &codecNames)
{
    d->setFallbackFileNameCodecs(codecNames);
}

bool ZipReader::open()
{
    return d->open();
}

bool ZipReader::isOpen() const
{
    return d->isOpen();
}

void ZipReader::close()
{
    d->close();
}

QStringList ZipReader::entryNames()
{
    return d->entryNamesList();
}

bool ZipReader::readEntry(const QString &entryName, QByteArray *data,
                          Qt::CaseSensitivity caseSensitivity)
{
    return d->readEntry(entryName, data, caseSensitivity);
}

bool ZipReader::processEntry(const QString &entryName,
                             const std::function<bool(QIODevice *device)> &processor,
                             Qt::CaseSensitivity caseSensitivity)
{
    return d->processEntry(entryName, processor, caseSensitivity);
}

} // namespace UkuiFileMetadata
