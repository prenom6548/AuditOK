/*
 * SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
 * SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef UKUIFILEMETADATA_H
#define UKUIFILEMETADATA_H

#include "ukui-file-metadata_global.h"
#include <memory>
#include "extractor.h"

namespace UkuiFileMetadata {
class ExtractorManagerPrivate;
/**
 * @brief The ExtractorPluginManager class is a helper class which loads all the extractor plugins.
 *It can be used to fetch plugins based on a given mimetype.
 *\author iaom <zhangpengfei@kylinos.cn>
 */
class UKUIFILEMETADATA_EXPORT ExtractorManager
{
public:
    ExtractorManager();
    virtual ~ExtractorManager();

    /**
     * Fetch the extractors which can be used to extract
     * data for the respective file with the given mimetype.
     *
     * If no match is found then the best matching plugins
     * are returned, determined by mimetype inheritance.
     *
     * \sa QMimeType::allAncestors
     */
    QList<Extractor*> fetchExtractors(const QString& mimetype) const;

private:

    QList<Extractor*> allExtractors();
    const std::unique_ptr<ExtractorManagerPrivate> d;
};
}

#endif // UKUIFILEMETADATA_H
