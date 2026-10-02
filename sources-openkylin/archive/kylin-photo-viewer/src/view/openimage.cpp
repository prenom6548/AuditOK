/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
*/
#include "openimage.h"

OpenImage::OpenImage(QWidget *parent) : KDragWidget(parent)
{
    this->setObjectName("Kylin-photo-viewer_KDragWidget_openImage");
    this->setAccessibleName("Kylin-photo-viewer_KDragWidget_openImage");
    this->setAccessibleDescription("open the image drag widget");
    this->iconButton()->setFixedSize(70, 70);
    this->iconButton()->setObjectName("Kylin-photo-viewer_QPushButton_dragButton");
    this->iconButton()->setAccessibleName("Kylin-photo-viewer_QPushButton_dragButton");
    this->textLabel()->setText(tr("Click or drag to load picture"));
    this->textLabel()->setObjectName("Kylin-photo-viewer_QLabel_dragText");
    this->textLabel()->setAccessibleName("Kylin-photo-viewer_QLabel_dragText");
    initOpenImage();
}

void OpenImage::initOpenImage()
{
    this->fileDialog()->update();
    QList<QUrl> list = this->fileDialog()->sidebarUrls();
    int sidebarNum = 8; //最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
            continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(this->fileDialog());
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, this,
            [this, &sidebarNum, &mntUrlList, &list](const QString path) {
                QDir wmntDir(path);
                wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
                QFileInfoList wfilist = wmntDir.entryInfoList();
                mntUrlList.clear();
                for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                    QFileInfo fi = wfilist.at(i);
                    //华为990、9a0需要屏蔽最小系统挂载的目录
                    if (fi.fileName() == "2691-6AB8")
                        continue;
                    mntUrlList << QUrl("file://" + fi.filePath());
                }
                this->fileDialog()->setSidebarUrls(list + mntUrlList);
                this->fileDialog()->update();
            });

    connect(this->fileDialog(), &QFileDialog::finished, this, [this, &list]() {
        this->fileDialog()->setSidebarUrls(list);
    });

    QString allFormats;
    allFormats = Variable::SUPPORT_FORMATS.join(" *.");
    QString setFilterAll = tr("Image Files") + QString(" (*.%1)").arg(allFormats);
    QStringList formatList = Variable::SUPPORT_FORMATS_CLASSIFY.split(";;", QString::SkipEmptyParts);
    QStringList categorizedFilters;
    for (const QString& formatStr: formatList) {
        categorizedFilters << QString("%1 %2").arg(formatStr).arg(formatStr);
    }
    QStringList filters;
    filters << setFilterAll << categorizedFilters;
    this->fileDialog()->setNameFilters(filters);

    //这行要添加，设置左侧导航
    this->fileDialog()->setSidebarUrls(list + mntUrlList);
    //设置视图模式
    this->fileDialog()->setViewMode(QFileDialog::Detail);
    this->fileDialog()->setWindowModality(Qt::WindowModal);
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    this->fileDialog()->setFileMode(QFileDialog::ExistingFiles);
    this->fileDialog()->setOption(QFileDialog::HideNameFilterDetails);

    QString openFile = tr("Open Image");
    //打开文件夹中的图片文件
    this->fileDialog()->setWindowTitle(openFile);
    this->fileDialog()->setDirectory(Variable::getSettings("imagePath").toString());
}

void OpenImage::setIsSupportOpen(bool isSupportOpen)
{
    m_isSupportOpen = isSupportOpen;
}

void OpenImage::dragEnterEvent(QDragEnterEvent* event)
{
    if (m_isSupportOpen) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
