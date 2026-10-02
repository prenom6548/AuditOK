/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "diskcheck.h"
#include "mainwindow.h"
#include "ui_diskcheck.h"
#include "src/data/datacdrom.h"
#include "src/model/logicmain.h"
#include "src/po/translation.h"
#include "src/process/processmd5sum.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QFileSystemWatcher>

DiskCheck::DiskCheck(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiskCheck)
{
    this->setObjectName("kylin-burner_DiskCheck");
    ui->setupUi(this);
    ui->lineEdit->setReadOnly(true);
    ui->lineEdit->setCursor(Qt::IBeamCursor);

    ui->comboBox->setAccessibleName("kylin-burner_QComboBox_DiskCheck_comboBox");
    ui->diskTitle->setAccessibleName("kylin-burner_QLabel_DiskCheck_diskTitle");
    ui->fileTitle->setAccessibleName("kylin-burner_QLabel_DiskCheck_fileTitle");
    ui->lineEdit->setAccessibleName("kylin-burner_CustomLineEdit_DiskCheck_lineEdit");
    ui->pushButton->setAccessibleName("kylin-burner_QPushButton_DiskCheck_pushButton");
    ui->title->setAccessibleName("kylin-burner_QLabel_DiskCheck_title");

    ui->title->setText(_("Disk Check"));
    ui->diskTitle->setText(_("Select a disc"));
    ui->fileTitle->setText(_("Check the disc using the MD5 file"));
    ui->pushButton->setText(_("Browse"));
    ui->pushButton->setEnabled(false);

    connect(LogicMain::instance(), SIGNAL(appendableMedium(void*)), this, SLOT(slotsAddMedium(void*)));
    connect(LogicMain::instance(), SIGNAL(finishedMedium(void*)), this, SLOT(slotsAddMedium(void*)));
    connect(LogicMain::instance(), &LogicMain::mediumRmoved, this, &DiskCheck::slotsRemoveMedium);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsChangeMedium(int)));
    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(slotsBrowseFile()));
    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &DiskCheck::slotChangeUIFontSize);
    connect(g_user_signal, &GlobalUserSignal::sigChangeDiscDisplayInfo, this, &DiskCheck::slotChangeDiscDisplayInfo);
}

void DiskCheck::accept()
{
    int idx;
    ProcessMd5sum *md5Check;

    idx = ui->comboBox->currentIndex();
    if (idx < 0) return;

    md5Check = new ProcessMd5sum(m_oCDROMS.at(idx), m_oCheckFiles.at(idx), this);
    md5Check->exec();
    delete md5Check;
}

void DiskCheck::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);
    QFont font = ui->title->font();
    font.setPixelSize(16 + scaleRatio);
    font.setBold(true);
    ui->title->setFont(font);

    font = ui->diskTitle->font();
    font.setPixelSize(14 + scaleRatio);


    ui->fileTitle->setFont(font);

    ui->lineEdit->setFont(font);
    ui->comboBox->setFont(font);
    ui->pushButton->setFont(font);

    //font.setPixelSize(14);
    ui->diskTitle->setFont(font);


}

void DiskCheck::slotChangeDiscDisplayInfo(void *cdrom, QString discInfo)
{
    for(int i = 0; i < m_oCDROMS.size(); i++) {
        if (cdrom != m_oCDROMS.at(i)) {
            continue;
        } else {
            ui->comboBox->setItemText(i, discInfo);
        }
    }
}

bool DiskCheck::check()
{
    if (ui->lineEdit->text().isEmpty() || ui->lineEdit->text().contains(_("Non md5sum.txt in the current Disk;Verification is not supported"))) {
        return false;
    }
    return true;
}

void DiskCheck::slotsAddMedium(void *c)
{
    int idx;
    DataCDROM *cdrom;
    QFileInfo f;
    QFileInfo f_data; //数据校验文件

    idx = m_oCDROMS.indexOf(c);
    if (-1 == idx)
    {
        m_oCDROMS << c;
        cdrom = static_cast<DataCDROM *>(c);
        f.setFile(cdrom->mediumMountPoint().append("/").append("md5sum.txt"));
        f_data.setFile(cdrom->mediumMountPoint().append("/").append(".md5sum.txt"));

        if (f.exists()) {
            m_oCheckFiles << f.absoluteFilePath();
        } else if (f_data.exists()) {
            m_oCheckFiles << f_data.absoluteFilePath();
        } else
            m_oCheckFiles << _("Non md5sum.txt in the current Disk;Verification is not supported");

        ui->comboBox->addItem(cdrom->mediumIcon(), cdrom->mDisplay());
        ui->comboBox->setCurrentIndex(m_oCDROMS.indexOf(c));
        change(check());
    }
    if (m_oCDROMS.size() > 0) ui->pushButton->setEnabled(true);
}

void DiskCheck::slotsRemoveMedium(void *c)
{
    int idx;

    idx = m_oCDROMS.indexOf(c);

    if (-1 != idx)
    {
        m_oCDROMS.takeAt(idx);
        m_oCheckFiles.takeAt(idx);
        ui->comboBox->removeItem(idx);
        change(check());
    }

    if (m_oCDROMS.size() <= 0) {
        ui->pushButton->setEnabled(false);
        ui->lineEdit->clear();
    }
}

void DiskCheck::slotsChangeMedium(int idx)
{
    if (-1 == idx) return;
    if (idx >= m_oCDROMS.size()) return;
    if(m_oCheckFiles.at(idx).contains("Non md5sum.txt"))
        ui->lineEdit->setText(m_oCheckFiles.at(idx));
    else ui->lineEdit->setText(m_oCheckFiles.at(idx));
    ui->lineEdit->setCursorPosition(0);
    ui->lineEdit->setToolTip(m_oCheckFiles.at(idx));
    change((check()));
}

void DiskCheck::slotsBrowseFile()
{
#ifndef UNIT_TEST
    QString file;
    QFileDialog fd;
    m_fileDialog = &fd;
    QList<QUrl> list = fd.sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files | QDir::NoSymLinks);
    mntDir.setNameFilters(QStringList() << "*.txt");

    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);

        if (fi.fileName() == "2691-6AB8") /* 990 9a0 */
                 continue;

        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(&fd);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path) {
        QDir wmntDir(path);

        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files | QDir::NoSymLinks);
        wmntDir.setNameFilters(QStringList() << "*.txt");

        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
            QFileInfo fi = wfilist.at(i);

            if (fi.fileName() == "2691-6AB8") /* 990 9a0 */
                     continue;

            mntUrlList << QUrl("file://" + fi.filePath());
        }
        fd.setSidebarUrls(list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd]() {
        fd.setSidebarUrls(list);
    });


    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setNameFilter("文本文件(*.txt)");
    fd.setSidebarUrls(list + mntUrlList);
    fd.move(this->geometry().center()-fd.geometry().center());
    fd.setGeometry(fd.geometry());
    if (fd.exec() != QDialog::Accepted) return;
    file = fd.selectedFiles().at(0);

    //file = QFileDialog::getOpenFileName(nullptr, _("Burner") + " - " + _("Browse"));
    ui->lineEdit->setText(file);
    ui->lineEdit->setCursorPosition(0);
    ui->lineEdit->setToolTip(file);
    m_oCheckFiles.replace(ui->comboBox->currentIndex(), file);
    change(check());
#endif
}

DiskCheck::~DiskCheck()
{
    delete ui;
}
