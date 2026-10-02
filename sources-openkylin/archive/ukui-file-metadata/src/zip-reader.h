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

#ifndef UKUI_FILE_METADATA_ZIP_READER_H
#define UKUI_FILE_METADATA_ZIP_READER_H

#include <functional>

#include <QByteArray>
#include <QIODevice>
#include <QList>
#include <QScopedPointer>
#include <QString>
#include <QStringList>

#include "ukui-file-metadata_global.h"

namespace UkuiFileMetadata {

/**
 * Lightweight ZIP archive reader for metadata extractors and external callers.
 *
 * The reader opens archives lazily. Constructing the object or changing the
 * archive path does not open the archive immediately. Callers can use `open()`
 * proactively or rely on `entryNames()` / `readEntry()` to open the archive on
 * demand.
 */

class UKUIFILEMETADATA_EXPORT ZipReader
{
public:
    /**
     * Create a reader bound to `archivePath`.
     *
     * The path may be empty. In that case `open()`, `entryNames()`, and
     * `readEntry()` will fail until a non-empty path is set.
     */
    explicit ZipReader(QString archivePath = {});

    /**
     * Destroy the reader and close the archive if it is still open.
     */
    ~ZipReader();

    /**
     * Replace the archive path used by this reader.
     *
     * If the reader is currently open, the current archive is closed first and
     * any cached entry name list is discarded.
     */
    void setArchivePath(const QString &archivePath);

    /**
     * Return the archive path currently associated with the reader.
     */
    QString archivePath() const;

    /**
     * Return the fallback codec names used for non-UTF8 ZIP entry names.
     *
     * `ZipReader` always tries the system codec before this list. The default
     * fallback list contains only `CP437`, which is the ZIP specification
     * default when no Unicode metadata is present. Use
     * `setFallbackFileNameCodecs()` to provide additional legacy codecs when
     * the creating system's encoding is known.
     */
    QList<QByteArray> fallbackFileNameCodecs() const;

    /**
     * Replace the fallback codec chain used for non-UTF8 ZIP entry names.
     *
     * Changing the codec list closes the current archive and clears cached
     * entry names so subsequent lookups use the new decoding policy.
     */
    void setFallbackFileNameCodecs(const QList<QByteArray> &codecNames);

    /**
     * Open the archive referenced by `archivePath()`.
     *
     * Returns `true` if the archive is already open or if opening succeeds.
     * Returns `false` when the path is empty, the archive cannot be opened, or
     * the archive cannot be positioned on its first entry.
     */
    bool open();

    /**
     * Return whether the archive is currently open.
     */
    bool isOpen() const;

    /**
     * Close the current archive and clear any cached entry names.
     *
     * Calling `close()` on an already closed reader is safe.
     */
    void close();

    /**
     * Return all entry names in the archive.
     *
     * The list is cached after the first successful scan and rebuilt whenever
     * the archive path changes or the reader is closed. An empty list is
     * returned if the archive cannot be opened or scanned.
     */
    QStringList entryNames();

    /**
     * Read the uncompressed contents of `entryName` into `data`.
     *
     * `data` must be non-null. The archive is opened automatically if needed.
     * `caseSensitivity` controls how the entry lookup is performed. On success,
     * `data` is replaced with the full entry contents and the method returns
     * `true`. On failure, `data` is cleared and the method returns `false`.
     */
    bool readEntry(const QString &entryName, QByteArray *data,
                   Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);

    /**
     * Process the uncompressed contents of `entryName` through a sequential device.
     *
     * `processor` must be callable. The archive is opened automatically if needed.
     * `caseSensitivity` controls how the entry lookup is performed. The method
     * returns `true` only when the entry is found, the callback returns `true`,
     * no read error occurs, and the entry closes successfully. Returning
     * `false` from the callback is treated as a processing failure even if the
     * caller intentionally stops reading early; the entry is still closed
     * before returning.
     */
    bool processEntry(const QString &entryName,
                      const std::function<bool(QIODevice *device)> &processor,
                      Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive);

private:
    Q_DISABLE_COPY(ZipReader)

    class Private;
    QScopedPointer<Private> d;
};

} // namespace UkuiFileMetadata

#endif
