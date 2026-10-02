/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#ifndef TOPDFPREVIEWPAGE_H
#define TOPDFPREVIEWPAGE_H

#include <QDateTime>
#include <QProcess>
#include <QString>

#ifdef USE_OnlyOffice

#include "previewpage/pdf.h"
typedef PdfPreviewPage ToPdfBaseClass;

#else

#include "previewpage/pdf2.h"
typedef Pdf2PreviewPage ToPdfBaseClass;

#endif

class ToPdfPreviewPage : public ToPdfBaseClass
{
public:
    explicit ToPdfPreviewPage(QWidget *parent = nullptr);
    ~ToPdfPreviewPage();

private:
    QProcess *m_proc;
    QString m_pdf_path;
    void loadConvertedPdf(const QString &expectedPath, const QString &outDir,
                          const QDateTime &startedAt, int attemptsLeft);

    // BasePreviewPage interface
public:
    void cancel() override;

public Q_SLOTS:
    void updateInfo(PreviewHubFileInfo *info) override;
};

#endif // TOPDFPREVIEWPAGE_H
