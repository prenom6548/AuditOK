/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */

#include "space-view.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <glib.h>
#include <QDragEnterEvent>
#include <QMimeData>
#include "file-utils.h"
Spaceview::Spaceview(QWidget *parent,QString uri,QString Spacename)
    :QWidget(parent),m_uri(uri),m_spaceName(Spacename)
{
    auto layout = new QVBoxLayout(this);
    setAcceptDrops(true);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setObjectName("_spaceview_layout");
    setProperty("highlightMode", true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setMouseTracking(true); // track mouse

    // Create a widget to hold the image and text
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Create a layout for the content widget
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(10); // Adjust spacing as needed

    // Create image label and center the image
    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(":/data/nodata.svg"); // Use resource file image
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter); // Center the image
    contentLayout->addWidget(imageLabel);

    // Create first line of text
    QLabel *textLabel1 = new QLabel(tr("When there are files in the system that meet the space conditions, they will be automatically displayed in the space"));
    textLabel1->setAlignment(Qt::AlignCenter); // Center the text
    contentLayout->addWidget(textLabel1);


    // Create second line of text
    QLabel *textLabel2 = new QLabel(tr("<html><head/><body><p>You can manually <a href='#addfiles' style='text-decoration:none;'>add files</a> or <a href='#modifyspace' style='text-decoration:none;'>modify the space conditions.</a></p></body></html>"), this);
    textLabel2->setOpenExternalLinks(false);
    textLabel2->setAlignment(Qt::AlignCenter); // Center the text
    contentLayout->addWidget(textLabel2);
    connect(textLabel2, &QLabel::linkActivated, this, &Spaceview::handleLinkActivated);
    // Add stretchable spacers to center the content
    layout->addStretch();
    layout->addWidget(contentWidget);
    layout->addStretch();
    this->setLayout(layout);
    m_signalTransInterface = new QDBusInterface("com.peony.idm.service",
                                                "/org/peony/idm/space/service",
                                                "org.peony.idm.modelcontroller");
}
void Spaceview::setDirectoryUri(QString uri)
{
    QString pre = "idm:///";
    m_uri = uri;
    m_spaceName = Peony::FileUtils::urlDecode(m_uri).remove(pre);
}

void Spaceview::handleLinkActivated(const QString &link) {
    if (link == "#addfiles") {
        QFileDialog fileDialog;
        // fixme: 居中窗口
        //fileDialog.windowHandle()->setTransientParent(qApp->topLevelWindows().first());
        fileDialog.setWindowTitle(tr("Choose file to add into space"));
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::AnyFile);
        QDir DocumentDir(g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS));
        fileDialog.setDirectory(DocumentDir);
        if (fileDialog.exec()) {
            auto urls = fileDialog.selectedUrls();
            if (!urls.isEmpty()) {
                //userAddFilesToSpace
                QStringList filePaths;
                for (auto url : urls) {
                    filePaths.append(url.path());
                }
                userAddFilesToSpaceBySpaceplugin(filePaths, m_spaceName,true);
            }
        }
    } else if (link == "#modifyspace") {
        //Q_EMIT IDMDBusManager::getInstance()->editSpaceConditionSignal(m_uri);
        //onModifySpaceConditions();
        editSpaceConditionBySpaceplugin(m_uri);
    }
}

void Spaceview::userAddFilesToSpaceBySpaceplugin(const QStringList &filePath, const QString &spaceName, bool isUserAdd)
{
    QDBusReply<int> reply = m_signalTransInterface->call("userAddFilesToSpaceBySpaceplugin",filePath,spaceName,isUserAdd);
}

void Spaceview::editSpaceConditionBySpaceplugin(const QString &uri)
{
    QDBusReply<int> reply = m_signalTransInterface->call("editSpaceConditionBySpaceplugin",uri);
}

void Spaceview::dragEnterEvent(QDragEnterEvent *event) {
    auto action = Qt::CopyAction;
    if (event->mimeData()->hasUrls()) {
        event->setDropAction(action);
        event->accept();
    }
}

void Spaceview::dragMoveEvent(QDragMoveEvent *event) {
    auto action = Qt::CopyAction;
    if (event->mimeData()->hasUrls()) {
        event->setDropAction(action);
        event->accept();
    }
    return QWidget::dragMoveEvent(event);
}

void Spaceview::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urlList = event->mimeData()->urls();
        QStringList filePaths;
        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            filePaths<<filePath;
        }
        userAddFilesToSpaceBySpaceplugin(filePaths, m_spaceName,true);
    }
}
