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

#include "burnimage.h"
#include "ui_burnimage.h"
#include "src/po/translation.h"
#include "src/frame/logger.h"
#include "src/frame/signals.h"
#include "src/frame/configuration.h"
#include "src/model/logicburnimage.h"
#include "src/model/menufactory.h"
#include "src/model/logicmain.h"
#include "src/view/mainwindow.h"
#include "src/data/datacdrom.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QLineEdit>

#include <qmessagebox.h>
#include <kballontip.h>
#include <QDebug>

BurnImage *BurnImage::m_oInstance = nullptr;
QMutex     BurnImage::m_oMutex;

BurnImage::BurnImage(QWidget *parent) :
    QWidget(parent),
    m_iBurnerIndex(0),
    ui(new Ui::BurnImage)
{
    this->setObjectName("kylin-burner_BurnImage");
    ui->setupUi(this);
    setAccessibleName();

    ui->title->setFocusPolicy(Qt::NoFocus);
    ui->title->setText(_("BurnImage"));

    ui->btnBrowseISO->setText(_("Browse"));
    ui->btnBurnerSetting->setText(_("Setting"));

    ui->labelISOTitle->setText(_("Please select ISO file which will to be burned"));
    ui->labelBurnerTitle->setText(_("Please select burner which will burned to"));

    m_burner_errorinfo_label = new AutoEllipseLabel(ui->comboBoxBurner);
    m_burner_errorinfo_label->setFixedWidth(ui->comboBoxBurner->width() + 10);
    m_burner_errorinfo_label->setFixedHeight(ui->comboBoxBurner->height() + 4);
    m_burner_errorinfo_label->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);

    this->installEventFilter(ui->comboBoxBurner);

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(),&LogicMain::systemFontSizeChanged,this,&BurnImage::slotChangeUIFontSize);
    connect(ui->btnBrowseISO, SIGNAL(clicked(bool)), this, SLOT(slotsBrowseISO()));
    connect(ui->comboBoxISO, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsISOChange(int)));
    connect(ui->comboBoxBurner, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsBurnerChange(int)));
    connect(this, SIGNAL(newISOPath(QString)), LogicBurnImage::instance(), SLOT(onNewISOPath(QString)));
    connect(LogicBurnImage::instance(), SIGNAL(enableBurner(bool)), ui->comboBoxBurner, SLOT(setEnabled(bool)));
    connect(LogicBurnImage::instance(), SIGNAL(enableBurner(bool)), ui->btnBurnerSetting, SLOT(setEnabled(bool)));
    connect(LogicBurnImage::instance(), SIGNAL(enableBusness(bool)), this, SIGNAL(busness(bool)));
    connect(LogicBurnImage::instance(), SIGNAL(isoError(QString)), this, SLOT(slotsISOError(QString)));
    connect(LogicBurnImage::instance(), &LogicBurnImage::burnerError, this, &BurnImage::slotsBurnerError);
    connect(LogicBurnImage::instance(), SIGNAL(ready(QString)), this, SLOT(slotsReady(QString)));
    connect(LogicBurnImage::instance(), &LogicBurnImage::newDeviceArrieved, this, &BurnImage::slotsNewBurner);
    connect(LogicBurnImage::instance(),&LogicBurnImage::deleteBurner, this,&BurnImage::slotsDeleteBurner);
    connect(this, &BurnImage::currentBurnerChanged, LogicBurnImage::instance(), &LogicBurnImage::setCurrentBurner);
    connect(ui->btnBurnerSetting, SIGNAL(clicked(bool)), MenuFactory::instance(), SIGNAL(burnSetting(bool)));
    connect(g_user_signal, &GlobalUserSignal::sigChangeDiscDisplayInfo, this, &BurnImage::slotsUpdateToolTipText);
    LogicBurnImage::instance()->init();

    // hide the icon, add icon to combobox
    ui->labelLogoBurner->hide();
    ui->labelLogoISO->hide();
}

BurnImage *BurnImage::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new BurnImage();
        }
    }
    return m_oInstance;
}

void BurnImage::setAccessibleName()
{
    ui->btnBrowseISO->setAccessibleName("kylin-burner_QPushButton_BurnImage_btnBrowseISO");
    ui->btnBurnerSetting->setAccessibleName("kylin-burner_QPushButton_BurnImage_btnBurnerSetting");
    ui->burnerSelection->setAccessibleName("kylin-burner_QWidget_BurnImage_burnerSelection");
    ui->burnerWidget->setAccessibleName("kylin-burner_QWidget_BurnImage_burnerWidget");
    ui->comboBoxBurner->setAccessibleName("kylin-burner_QComboBox_BurnImage_comboBoxBurner");
    ui->comboBoxISO->setAccessibleName("kylin-burner_QComboBox_BurnImage_comboBoxISO");

    ui->isoSelection->setAccessibleName("kylin-burner_QWidget_BurnImage_isoSelection");
    ui->isoWidget->setAccessibleName("kylin-burner_QWidget_BurnImage_isoWidget");
    ui->labelBurnerTip->setAccessibleName("kylin-burner_QLabel_BurnImage_labelBurnerTip");
    ui->labelBurnerTitle->setAccessibleName("kylin-burner_QLabel_BurnImage_labelBurnerTitle");
    ui->labelISOTip->setAccessibleName("kylin-burner_QLabel_BurnImage_labelISOTip");
    ui->labelISOTitle->setAccessibleName("kylin-burner_QLabel_BurnImage_labelISOTitle");
    ui->labelLogoBurner->setAccessibleName("kylin-burner_QLabel_BurnImage_labelLogoBurner");
    ui->labelLogoISO->setAccessibleName("kylin-burner_QLabel_BurnImage_labelLogoISO");
    ui->title->setAccessibleName("kylin-burner_QLabel_BurnImage_title");
}

void BurnImage::update()
{
    BURNIMAGE->init();
}

void BurnImage::resizeEvent(QResizeEvent *event)
{
    updataComboxIsoTextShow();
    
    m_burner_errorinfo_label->setFixedWidth(ui->comboBoxBurner->width() + 10);
    m_burner_errorinfo_label->setFixedHeight(ui->comboBoxBurner->height() + 4);
    QTimer::singleShot(50,[&]() {
        // 设置label位置使其在父控件中居中
        int y = (ui->comboBoxBurner->height() - m_burner_errorinfo_label->height()) / 2;
        m_burner_errorinfo_label->move(0, y);
    });
    QWidget::resizeEvent(event);
}

void BurnImage::slotsBrowseISO()
{
#ifndef UNIT_TEST
    QString path;
    QString isoPath;
    QFileDialog fd;
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

    if (m_oISOPaths.size() && ui->comboBoxISO->currentIndex() >= 0 &&
            ui->comboBoxISO->currentIndex() < m_oISOPaths.size())
        path = m_oISOPaths.at(ui->comboBoxISO->currentIndex());

    fd.setFileMode(QFileDialog::ExistingFiles);
    fd.setNameFilter("ISO(*.iso)");
    fd.setSidebarUrls(list + mntUrlList);
    fd.move(this->geometry().center()-fd.geometry().center());
    fd.setGeometry(fd.geometry());
    if (fd.exec() != QDialog::Accepted) return;
    isoPath = fd.selectedFiles().at(0);
    if (isoPath.isEmpty()) return;
    if (-1 != m_oISOPaths.indexOf(isoPath)){
        ui->comboBoxISO->setCurrentIndex(m_oISOPaths.indexOf(isoPath));
    }
    else
        slotsNewISOPath(isoPath);
#endif
}

void BurnImage::slotsISOChange(int idx)
{
    int u;
    qreal size;
    QIcon icon;
    QFileInfo f;
    QString display;

    ui->labelISOTip->setText("");

    if (idx < 0)
    {
        ui->labelLogoISO->setPixmap(icon.pixmap(ui->labelLogoISO->size()));
        return;
    }
    if (!m_oISOPaths.at(idx).isEmpty())
    {
        emit newISOPath(m_oISOPaths.at(idx));
        icon = QIcon::fromTheme("application-x-cd-image");
    }
    ui->labelLogoISO->setPixmap(icon.pixmap(ui->labelLogoISO->size()));
    f.setFile(m_oISOPaths.at(idx));
    display = f.fileName();
    display.append("  ");
    size = f.size() * 1.0;
    u = -1;
    while (size > 1024.00)
    {
        ++u; size /= 1024.00;
    }
    display.append(QString("%1").arg(size, 0, 'f', 1));
    display.append(" ");
    switch (u)
    {
    case 0:
        display.append(_("KB"));
        break;
    case 1:
        display.append(_("MB"));
        break;
    case 2:
        display.append(_("GB"));
        break;
    default:
        display.append(_("B"));
        break;
    }
    ui->comboBoxISO->setCurrentText(display);
}

void BurnImage::slotsNewISOPath(QString path)
{
    m_oISOPaths << path;

    QFileInfo fileInfo(path);
    QString fileName = fileInfo.fileName();

    ui->comboBoxISO->addItem(QIcon::fromTheme("application-x-cd-image"),path,fileName);

    ui->comboBoxISO->setCurrentIndex(m_oISOPaths.size() - 1);
    //emit newISOPath(path);
    //slotsISOChange(m_oISOPaths.size() - 1);
}

void BurnImage::slotsISOError(QString err)
{
    updateLabelIsoTip(err);
}

void BurnImage::slotsBurnerError(QString err)
{

    ui->btnBurnerSetting->setEnabled(false);
    err = "   " + err;
    m_burner_errorinfo_label->setText(err);
    m_burner_errorinfo_label->setToolTip(err);

    ui->labelBurnerTip->setText("");
    ui->comboBoxBurner->setEnabled(false);
    ui->comboBoxBurner->setCurrentText("");

    if(ui->comboBoxBurner->count() > 0){
        ui->comboBoxBurner->blockSignals(true);
        ui->comboBoxBurner->setCurrentIndex(-1);
        ui->comboBoxBurner->blockSignals(false);
    }
}

void BurnImage::slotsReady(QString str)
{
    QIcon icon;
    QString display;

    bool isDLDisc = LogicBurnImage::instance()->isDLDisc();
    QString currentUuid = LogicBurnImage::instance()->currentDeviceUuid();
    if(currentUuid != ui->comboBoxBurner->currentData().toString()){
        int index = ui->comboBoxBurner->findData(currentUuid);
        if(index >=0){
            ui->comboBoxBurner->setCurrentIndex(index);
        }
    }

    int idx = ui->comboBoxBurner->currentData().toInt();
    ui->labelBurnerTip->setText("");
    icon = BURNIMAGE->mediumIcon(idx);
    display = BURNIMAGE->display(idx);
    ui->labelLogoBurner->setPixmap(icon.pixmap(ui->labelLogoBurner->size()));

    if(isDLDisc) {
        updateLabelIsoTip(_("Note: Large-capacity images may reduce burning success rate; please verify the disc after burning."));
    } else {
        ui->labelISOTip->setText("");
    }
    ui->labelBurnerTip->setText(str);
    ui->btnBurnerSetting->setEnabled(true);
}

void BurnImage::slotsNewBurner(QIcon miniIcon,QString display, QString tooltip, QString uuid)
{
    ui->comboBoxBurner->addItem(miniIcon, display, uuid);
    ui->comboBoxBurner->setCurrentIndex(ui->comboBoxBurner->count() - 1);
    updateComboboxTooltip();
}

void BurnImage::slotsDeleteBurner(QString uuid)
{
    int to_delete_index =  ui->comboBoxBurner->findData(uuid);
    ui->comboBoxBurner->removeItem(to_delete_index);
    updateComboboxTooltip();
    update();
}

void BurnImage::slotsBurnerChange(int idx)
{
    QString uuid = ui->comboBoxBurner->itemData(idx).toString();
    emit currentBurnerChanged(uuid);
    updateComboboxTooltip();
}

void BurnImage::slotsUpdateToolTipText(void *cdrom, QString toolTip)
{
    DataCDROM * inputcdrom = (DataCDROM *)cdrom;
    QString currentUuid = inputcdrom->getUnique_uuid();
    int currentIndex = ui->comboBoxBurner->findData(currentUuid);
    if(currentIndex >= 0){
        ui->comboBoxBurner->setItemText(currentIndex, inputcdrom->mDisplay());
    }
}

void BurnImage::updataComboxIsoTextShow()
{
    int isoIndex = ui->comboBoxISO->currentIndex();
    if (isoIndex < 0)
        return;

    slotsISOChange(isoIndex);
}

void BurnImage::updateComboboxTooltip()
{
    m_burner_errorinfo_label->clear();
    m_burner_errorinfo_label->setToolTip("");
    if(ui->comboBoxBurner->count() > 0){
        if(!ui->comboBoxBurner->isEnabled()){
            ui->comboBoxBurner->setEnabled(true);
        }
    }
}

void BurnImage::updateLabelIsoTip(QString tipInfo)
{
    if (!tipInfo.isEmpty()) {
        QColor textColor(0, 0, 0, 89);
        ui->labelISOTip->setForegroundRole(QPalette::WindowText);
        QPalette pal = ui->labelISOTip->palette();
        pal.setColor(QPalette::WindowText, textColor);
        ui->labelISOTip->setPalette(pal);
    }

    ui->labelISOTip->setText(tipInfo);
}

bool BurnImage::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize){
        if(watched == ui->comboBoxBurner){
            m_burner_errorinfo_label->setGeometry(ui->comboBoxBurner->lineEdit()->rect());
        }
    }
}

void BurnImage::slotChangeUIFontSize(int fontSize)
{
    int scaleRatio = qAbs(fontSize - 10.0);
    QFont font = ui->title->font();
    font.setPixelSize(24 + scaleRatio);
    ui->title->setFont(font);

    font = ui->labelISOTitle->font();
    font.setPixelSize(14 + scaleRatio);

    ui->labelISOTip->setFont(font);
    ui->labelBurnerTip->setFont(font);
    ui->comboBoxBurner->setFont(font);
    ui->comboBoxISO->setFont(font);
    ui->btnBrowseISO->setFont(font);
    ui->btnBurnerSetting->setFont(font);


    font.setWeight(60);
    ui->labelISOTitle->setFont(font);
    ui->labelBurnerTitle->setFont(font);
    m_burner_errorinfo_label->setFont(font);

    font.setPixelSize(14 + scaleRatio);

    updataComboxIsoTextShow();

}

BurnImage::~BurnImage()
{
    delete ui;
}

void BurnImage::on_comboBoxBurner_activated(int index)
{
#ifndef UNIT_TEST
    if (LogicBurnImage::instance()->checkBurner()) {
        return;
    }
    else {
        kdk::KBallonTip *validBurnerWarnTip = new kdk::KBallonTip(_("This disc is not empty, please add an empty disc!"), kdk::TipType::Warning, this);

        validBurnerWarnTip->setTipTime(1500);
        validBurnerWarnTip->setWindowFlag(Qt::FramelessWindowHint);
        validBurnerWarnTip->setAttribute(Qt::WA_TranslucentBackground);
        validBurnerWarnTip->showInfo();
        validBurnerWarnTip->move(MainWindow::Instance()->frameGeometry().center() - validBurnerWarnTip->rect().center());
    }
#endif
}

void AutoEllipseLabel::setText(const QString &text)
{
    QString labelContentShowText = text;
    m_rawString = text;
    QFontMetrics metrics(font());
    if (metrics.width(labelContentShowText) > width()) {
        labelContentShowText = QFontMetrics(font()).elidedText(text, Qt::ElideRight, width());
    }
    QLabel::setText(labelContentShowText);
}

void AutoEllipseLabel::setFont(const QFont &font)
{
    QLabel::setFont(font);
    setText(m_rawString);
}
