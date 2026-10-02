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

#include "previewpage/onlyoffice.h"

DocPreviewPage::DocPreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    m_web_view = new QWebEngineView(this);
    m_web_channel = new QWebChannel(this);
    m_web_page = new QWebEnginePage(this);
    m_web_page->load(QUrl("qrc:/onlyoffice/index.html"));
    m_web_page->setWebChannel(m_web_channel);
    m_web_view->setPage(m_web_page);
    m_web_view->setContentsMargins(0, 0, 0, 0);

    base_layout->addWidget(m_web_view);
}

void DocPreviewPage::cancel() {
    qDebug() << "DocPreviewPage::cancel: ";
    // qDebug() << m_proc->processId();
    if (m_proc_pid > 0) {
        QProcess::execute(QLatin1String("kill"), QStringList() << QString::number(m_proc_pid));
        // QProcess::execute(QLatin1String("killall"), QStringList()<<"python3");
        m_proc_pid = -1;
    }
    m_web_page->runJavaScript("refresh()");
}

void DocPreviewPage::updateInfo(PreviewHubFileInfo *info) {
    qDebug() << "in doc updateInfo";

    QString path = "/", filename, suffixname;
    for (int i = info->filePath().size()-1; i >= 0; i--) {
        if (info->filePath().at(i) == '.' && suffixname.isEmpty())
            suffixname = info->filePath().right(info->filePath().size()-i-1);
        if (info->filePath().at(i) == '/') {
            path = info->filePath().left(i);
            filename = info->filePath().right(info->filePath().size()-i-1);
            break;
        }
    }
    qDebug() << path << " " << filename;

    QProcess m_proc;
    m_proc.setProgram("python3");
    m_proc.setArguments(QStringList() << "-m" << "http.server"
                       << "-d" << path
                       << "-b" << "localhost" << "9124");

    m_proc.startDetached(&m_proc_pid);
    /**
     * \brief update(documentType, fileType, fileName)
     * \details
     *  documentType : fileType
     *  word : docx doc odt txt
     *  cell : xlsx xls ods csv
     *  slide : pptx ppt odp
     */
    QStringList wordTypes, cellTypes, slideTypes;
    wordTypes << "docx" << "doc" << "odt" << "txt";
    cellTypes << "xlsx" << "xls" << "ods" << "csv";
    slideTypes << "pptx" << "ppt" << "odp";

    QString method;
    if (wordTypes.contains(suffixname))
        method = QString("update(\"%1\",\"%2\",\"%3\")").arg("word").arg(suffixname).arg(filename);
    else if (cellTypes.contains(suffixname))
        method = QString("update(\"%1\",\"%2\",\"%3\")").arg("cell").arg(suffixname).arg(filename);
    else if (slideTypes.contains(suffixname))
        method = QString("update(\"%1\",\"%2\",\"%3\")").arg("slide").arg(suffixname).arg(filename);
    else
        method = QString("show(\"%1\")").arg("<b>Cann not preview this file.</b>");

    m_web_page->runJavaScript(method);

    qDebug() << "out doc updateInfo";
}
