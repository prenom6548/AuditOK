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

#include "previewpage/to-pdf.h"
#include "preview-file-type.h"
#include "libreoffice-runtime.h"

#include <QDir>
#include <QFileInfo>
#include <QTimer>

static QString libreOfficePdfOutDir()
{
    const QString d = QDir::tempPath() + QLatin1String("/ukui-previewhub-topdf");
    QDir().mkpath(d);
    return d;
}

static QString newestConvertedPdf(const QString &outDir, const QDateTime &startedAt)
{
    QDir dir(outDir);
    const QFileInfoList files = dir.entryInfoList(QStringList() << QLatin1String("*.pdf"),
                                                  QDir::Files, QDir::Time);
    for (const QFileInfo &file : files) {
        if (file.lastModified() >= startedAt.addSecs(-1))
            return file.absoluteFilePath();
    }
    return QString();
}

ToPdfPreviewPage::ToPdfPreviewPage(bool simplePreview, QWidget *parent):
    ToPdfBaseClass(simplePreview, parent),m_proc(nullptr)
{

}

ToPdfPreviewPage::~ToPdfPreviewPage()
{
    if (m_proc != nullptr) {
        QProcess *proc = m_proc;
        m_proc = nullptr;
        proc->disconnect(this);
        delete proc;
    }
}

void ToPdfPreviewPage::cancel() {
    ToPdfBaseClass::cancel();
    if (m_proc != nullptr) {
        QProcess *proc = m_proc;
        m_proc = nullptr;
        proc->disconnect(this);
        proc->deleteLater();
        QProcess::startDetached("rm", QStringList() << m_pdf_path);
    }
}

void ToPdfPreviewPage::updateInfo(PreviewHubFileInfo *info) {
    if (m_proc != nullptr) {
        QProcess *proc = m_proc;
        m_proc = nullptr;
        proc->disconnect(this);
        proc->deleteLater();
    }

    m_proc = new QProcess(this);
    QProcess *proc = m_proc;
    const QString out = libreOfficePdfOutDir();
    const QDateTime startedAt = QDateTime::currentDateTime();
    const QString displayName = info->displayName();
    const LibreOfficeRuntime::Command command = LibreOfficeRuntime::convertToPdfCommand(out, info->filePath());
    proc->setProgram(command.program);
    proc->setArguments(command.arguments);
    qDebug() << proc->arguments() << "\n" << info->filePath();
    proc->start();
    //m_proc->waitForFinished();
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          [=](int exitCode, QProcess::ExitStatus exitStatus) {
        const QString stdOut = QString::fromLocal8Bit(proc->readAllStandardOutput());
        const QString stdErr = QString::fromLocal8Bit(proc->readAllStandardError());
        if (!stdOut.isEmpty())
            qDebug() << "libreoffice stdout:" << stdOut;
        if (!stdErr.isEmpty())
            qDebug() << "libreoffice stderr:" << stdErr;

        if (m_proc == proc)
            m_proc = nullptr;
        proc->deleteLater();

        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            qDebug() << QString("libreoffice convert failed with exitCode %1").arg(exitCode);
            return;
        }

        QString file_name = displayName;
        file_name = file_name.left(file_name.lastIndexOf(".")) + ".pdf";
        const QString expectedPath = out + QLatin1Char('/') + file_name;
        loadConvertedPdf(expectedPath, out, startedAt, 20);
    });
}

void ToPdfPreviewPage::loadConvertedPdf(const QString &expectedPath, const QString &outDir,
                                        const QDateTime &startedAt, int attemptsLeft)
{
    QString pdfPath = expectedPath;
    if (!QFileInfo::exists(pdfPath))
        pdfPath = newestConvertedPdf(outDir, startedAt);

    if (!pdfPath.isEmpty() && QFileInfo::exists(pdfPath)) {
        m_pdf_path = pdfPath;
        qDebug() << "new file:" + m_pdf_path;
        ToPdfBaseClass::updateInfo(m_pdf_path);
        return;
    }

    if (attemptsLeft <= 0) {
        qDebug() << "libreoffice output pdf not found:" << expectedPath;
        return;
    }

    QTimer::singleShot(250, this, [=]() {
        loadConvertedPdf(expectedPath, outDir, startedAt, attemptsLeft - 1);
    });
}
