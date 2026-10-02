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

#include "copydisc.h"
#include "ui_copydisc.h"
#include "src/po/translation.h"
#include "src/frame/logger.h"
#include "src/frame/configuration.h"
#include "src/model/logicmain.h"
#include "src/model/logiccopydisc.h"
#include "src/model/menufactory.h"
#include "src/data/datacdrom.h"
#include "src/view/execdialog.h"
#include "src/view/mainwindow.h"
#include "src/frame/signals.h"

#include <QFileDialog>
#include <QFileSystemWatcher>

using namespace kdk;

CopyDisc *CopyDisc::m_oInstance = nullptr;
QMutex    CopyDisc::m_oMutex;

CopyDisc::CopyDisc(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CopyDisc)
{
    this->setObjectName("kylin-burner_CopyImage");
    ui->setupUi(this);
    setAccessibleName();

    ui->title->setFocusPolicy(Qt::NoFocus);
    ui->title->setText(_("CopyDisk"));

    ui->btnBurnerSetting->setText(_("SelectPath"));
    ui->labelISOTitle->setText(_("Please select disk which to be copied"));
    ui->labelBurnerTitle->setText(_("Please select the image/disk you want to write"));

    m_oShowISO = new QWidget(ui->isoWidget);
    m_oShowISO->setMinimumSize(ui->isoWidget->size());
    m_oShowISO->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_oIcon = new QLabel(m_oShowISO);
    m_oIcon->setFixedSize(25, 25);
    m_oDisplay = new QLabel(m_oShowISO);
    m_oDisplay->setFixedHeight(30);
    //m_oDisplay->setFixedWidth(ui->comboBoxISO->width() - 15);

    QHBoxLayout *showISOLay = new QHBoxLayout(m_oShowISO);
    showISOLay->setMargin(0);
    showISOLay->addWidget(m_oIcon);
    //showISOLay->addSpacing(5);
    showISOLay->addWidget(m_oDisplay);
    showISOLay->addStretch(15);
    m_oShowISO->setFocusPolicy(Qt::NoFocus);
    m_oShowISO->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    m_oShowBurner = new QWidget(ui->burnerWidget);
    m_oIconBurner = new QLabel(m_oShowBurner);
    m_oIconBurner->setFixedSize(25, 25);
    m_oDisplayBurner = new QLabel(m_oShowBurner);
    m_oDisplayBurner->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QHBoxLayout *showBurnerLay = new QHBoxLayout(m_oShowBurner);
    showBurnerLay->setMargin(0);
    showBurnerLay->addWidget(m_oIconBurner);
    showBurnerLay->addWidget(m_oDisplayBurner);
    showBurnerLay->addStretch(15);
    m_oShowBurner->setFocusPolicy(Qt::NoFocus);
    m_oShowBurner->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    connect(LogicCopyDisc::instance(), &LogicCopyDisc::sigAddDestination, this, &CopyDisc::slotsAddDestination);
    connect(LogicCopyDisc::instance(), &LogicCopyDisc::sigDelDestination, this, &CopyDisc::slotsDelDestination);
    connect(LogicCopyDisc::instance(), &LogicCopyDisc::sigAddSource, this, &CopyDisc::slotsAddSource);
    connect(LogicCopyDisc::instance(), &LogicCopyDisc::sigDelSource, this, &CopyDisc::slotsDelSource);
    connect(ui->comboBoxBurner, SIGNAL(currentIndexChanged(int)), LogicCopyDisc::instance(), SLOT(slotsDestinationChange(int)));
    connect(ui->comboBoxBurner, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsDestinationChange(int)));
    connect(ui->comboBoxISO, SIGNAL(currentIndexChanged(int)), LogicCopyDisc::instance(), SLOT(slotsSourceChange(int)));
    connect(ui->btnBurnerSetting, SIGNAL(clicked(bool)), this, SLOT(slotsButtonClick()));

    connect(g_user_signal, &GlobalUserSignal::sigChangeDiscDisplayInfo, this, [=](void *cdrom, QString displayInfo){

        int index = m_oSrc.indexOf(cdrom);
        if ( -1 == index)
            return;

        ui->comboBoxISO->setItemText(index, displayInfo);
    });

    ui->labelLogoISO->hide();
    ui->labelLogoBurner->hide();

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &CopyDisc::slotChangeUIFontSize);
}

CopyDisc *CopyDisc::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new CopyDisc();
        }
    }
    return m_oInstance;
}

void CopyDisc::paintEvent(QPaintEvent *event)
{
    int width = ui->burnerWidget->width();
    ui->isoWidget->setFixedWidth(width);
    QWidget::paintEvent(event);
}

void CopyDisc::resizeEvent(QResizeEvent *event)
{
    int width = ui->burnerWidget->width();
    ui->isoWidget->setFixedWidth(width);
    QWidget::resizeEvent(event);
}

void CopyDisc::setAccessibleName()
{
    ui->btnBurnerSetting->setAccessibleName("kylin-burner_QPushButton_CopyImage_btnBurnerSetting");
    ui->burnerSelection->setAccessibleName("kylin-burner_QWidget_CopyImage_burnerSelection");
    ui->burnerWidget->setAccessibleName("kylin-burner_QWidget_CopyImage_burnerWidget");
    ui->comboBoxBurner->setAccessibleName("kylin-burner_QComboBox_CopyImage_comboBoxBurner");
    ui->comboBoxISO->setAccessibleName("kylin-burner_QComboBox_CopyImage_comboBoxISO");
    ui->isoSelection->setAccessibleName("kylin-burner_QWidget_CopyImage_isoSelection");
    ui->isoWidget->setAccessibleName("kylin-burner_QWidget_CopyImage_isoWidget");
    ui->labelBurnerTip->setAccessibleName("kylin-burner_QLabel_CopyImage_labelBurnerTip");
    ui->labelBurnerTitle->setAccessibleName("kylin-burner_QLabel_CopyImage_labelBurnerTitle");
    ui->labelISOTip->setAccessibleName("kylin-burner_QLabel_CopyImage_labelISOTip");
    ui->labelISOTitle->setAccessibleName("kylin-burner_QLabel_CopyImage_labelISOTitle");
    ui->labelLogoBurner->setAccessibleName("kylin-burner_QLabel_CopyImage_labelLogoBurner");
    ui->labelLogoISO->setAccessibleName("kylin-burner_QLabel_CopyImage_labelLogoISO");
    ui->title->setAccessibleName("kylin-burner_QLabel_CopyImage_title");
}

void CopyDisc::check()
{
    if (ui->comboBoxISO->currentIndex() < 0 ||
            ui->comboBoxBurner->currentIndex() < 0)
    {
        busness(false);
        ui->labelISOTip->setText("");
        if (ui->comboBoxBurner->currentIndex() > 0){
            ui->btnBurnerSetting->setEnabled(false);
        }
        else
            ui->btnBurnerSetting->setEnabled(true);
        return;
    }
    if (LogicCopyDisc::instance()->capacity())
    {
        busness(true);
        ui->btnBurnerSetting->setEnabled(true);
        if (ui->comboBoxBurner->currentIndex() == 0)
        {
            busness(LogicMain::GetISO);
            ui->labelISOTip->setText("");
        }
        else
        {
            busness(LogicMain::CopyData);
            if (LogicCopyDisc::instance()->isDestinationDiscDL()) {
                updateLabelIsoTip(_("Note: Large-capacity data/image discs may lower copy success rate."));
            }
        }
        return;
    }

    busness(false);
    ui->labelISOTip->setText("");
    if (ui->comboBoxBurner->currentIndex() > 0)
        ui->btnBurnerSetting->setEnabled(false);
    else
        ui->btnBurnerSetting->setEnabled(true);
    if (ui->comboBoxBurner->currentIndex() == 0)
        busness(LogicMain::GetISO);
    else
        busness(LogicMain::CopyData);
}

void CopyDisc::updateLabelIsoTip(QString tipInfo)
{
    ui->labelISOTip->setForegroundRole(QPalette::WindowText);
    QPalette pal = ui->labelISOTip->palette();
    ui->labelISOTip->setPalette(pal);
    ui->labelISOTip->setText(tipInfo);
}

void CopyDisc::slotsAddDestination(void *d)
{
    DataCDROM *cdrom = static_cast<DataCDROM *>(d);
    int index = m_oDes.indexOf(d);
    QString newDisplay = cdrom->mDisplay();
    if((-1 != index)){														//当设备容量更新时，更新复选框内的文字信息
        if(newDisplay != ui->comboBoxBurner->itemText(index))
            ui->comboBoxBurner->setItemText(index, newDisplay);
        return;
    }

    m_oDes << d;
    ui->comboBoxBurner->addItem(cdrom->mediumIcon(), cdrom->mDisplay());
    check();
}

void CopyDisc::slotsDelDestination(int idx)
{
    ui->comboBoxBurner->removeItem(idx);
    m_oDes.takeAt(idx);
    check();
}

void CopyDisc::slotsAddSource(void *d)
{
    DataCDROM *cdrom = static_cast<DataCDROM *>(d);
    if(m_oSrc.indexOf(d) != -1) 
        return;
   
    ui->comboBoxISO->addItem(cdrom->mediumIcon(), cdrom->mDisplay());
    m_oSrc << d;
    check();
}

void CopyDisc::slotsDelSource(int idx)
{
    ui->comboBoxISO->removeItem(idx);
    m_oSrc.takeAt(idx);
    check();
}

void CopyDisc::slotsButtonClick()
{
#ifndef UNIT_TEST
    QString path;
    QFileInfo f;

    if (ui->comboBoxBurner->currentIndex() > 0)
    {
        MenuFactory::instance()->burnSetting(true);
        return;
    }

    QFileDialog fd(this);
    QList<QUrl> list = fd.sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
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
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
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


    fd.setFileMode(QFileDialog::DirectoryOnly);
    fd.setSidebarUrls(list + mntUrlList);
    fd.move(this->geometry().center()-fd.geometry().center());
    fd.setGeometry(fd.geometry());
    if (fd.exec() != QDialog::Accepted) return;
    path = fd.selectedFiles().at(0);

    if (path.isEmpty()) return;
    f.setFile(path);
    if (!(f.permissions() & QFileDevice::WriteUser))
    {
        EXEC->error(_("Browse ISO Path"), _("Path %1 owner have no write permission.").arg(path));
        return;
    }
    path.append("/");
    LogicCopyDisc::instance()->updateISOPath(path);
    ui->comboBoxBurner->setItemText(0, path);
    update();
#endif
}

void CopyDisc::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);

    QFont font = ui->title->font();
    font.setPixelSize(24 + scaleRatio);
    ui->title->setFont(font);

    font.setPixelSize(14 + scaleRatio);
    ui->labelISOTip->setFont(font);
    ui->labelBurnerTip->setFont(font);
    ui->comboBoxBurner->setFont(font);
    ui->comboBoxISO->setFont(font);
    ui->btnBurnerSetting->setFont(font);

    font.setWeight(60);
    ui->labelISOTitle->setFont(font);
    ui->labelBurnerTitle->setFont(font);

    font = m_oDisplay->font();
    font.setPixelSize(14 + scaleRatio);
    m_oDisplay->setFont(font);

    m_oDisplayBurner->setFont(font);
}

void CopyDisc::slotsDestinationChange(int idx)
{
    if (idx < 0)
        return;

    if (idx > 0) {
        ui->btnBurnerSetting->setText(_("Setting"));
        emit busness(LogicMain::CopyData);
    } else {
        ui->btnBurnerSetting->setText(_("SelectPath"));
        emit busness(LogicMain::GetISO);
    }
    check();
}

CopyDisc::~CopyDisc()
{
    delete ui;
}
