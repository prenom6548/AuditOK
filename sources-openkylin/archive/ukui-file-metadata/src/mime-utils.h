/*
    This file is part of UkuiFileMetadata
    SPDX-FileCopyrightText: 2019 Stefan Brüns <stefan.bruens@rwth-aachen.de>
    SPDX-FileCopyrightText: 2019 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef UKUIFILEMETADATA_MIMEUTILS
#define UKUIEMETADATA_MIMEUTILS

#include <QMimeDatabase>
#include "ukui-file-metadata_global.h"


namespace UkuiFileMetadata
{
namespace MimeUtils
{

/**
 * Returns the mimetype for a file
 *
 * The function uses both content and filename to determine the
 * \c QMimeType. In case the extension mimetype is more specific
 * than the content mimetype, and the first inherits the latter,
 * the extension mimetype is preferred.
 * If the extension does not match the content, the content has
 * higher priority.
 * The file must exist and be readable.
 *
 * @since 5.57
 *
 * \sa QMimeDatabase::mimeTypesForFileName
 * \sa QMimeType::inherits
 */
UKUIFILEMETADATA_EXPORT
QMimeType strictMimeType(const QString& filePath, const QMimeDatabase& db);


} // namespace MimeUtils
} // namespace KFileMetaData

#endif // UKUIEMETADATA_MIMEUTILS
