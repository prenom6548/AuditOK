/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "attributewindow.h"
#include "ui_attributewindow.h"
#include <QPixmap>
#include "mainwindow.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);





AttributeWindow::AttributeWindow(DeviceInfo *deviceinfo, QWidget *parent) :
    QDialog(parent),
    deviceInfo(deviceinfo),
    ui(new Ui::AttributeWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setupInit();
    QString verifyType = transferVerifyType(deviceInfo->vertype);
    QString busType = transferBusType(deviceInfo->bustype);
    QString storageType = transferStorageType(deviceInfo->stotype);
    QString identifyType = transferIdentifyType(deviceInfo->idtype);

    //qDebug()<<verifyType<<busType<<storageType<<identifyType;
    ui->bioBusTypeLbl->setText(busType);
    ui->bioVerifyTypeLbl->setText(verifyType);
    ui->bioStorageTypeLbl->setText(storageType);
    ui->bioIdentificationTypeLbl->setText(identifyType);

}

AttributeWindow::~AttributeWindow()
{
    delete ui;
}

QString AttributeWindow::transferBioType(int type)
{
    switch(type) {
    case BIOTYPE_FINGERPRINT:
        return tr("FingerPrint");
    case BIOTYPE_FINGERVEIN:
        return tr("Fingervein");
    case BIOTYPE_IRIS:
        return tr("Iris");
    case BIOTYPE_FACE:
        return tr("Face");
    case BIOTYPE_VOICEPRINT:
        return tr("VoicePrint");
    }
    return QString();
}

QString AttributeWindow::transferVerifyType(int type)
{
    switch(type) {
    case VERIFY_HARDWARE:
        return tr("Hardware Verification");
    case VERIFY_SOFTWARE:
        return tr("Software Verification");
    case VERIFY_MIX:
        return tr("Mix Verification");
    case VERIFY_OTHER:
        return tr("Other Verification");
    }
    return QString();
}
QString AttributeWindow::transferStorageType(int type)
{
    switch(type) {
    case STORAGE_DEVICE:
        return tr("Device Storage");
    case STORAGE_OS:
        return tr("OS Storage");
    case STORAGE_MIX:
        return tr("Mix Storage");
    }
    return QString();
}
QString AttributeWindow::transferBusType(int type)
{
    switch(type) {
    case BUS_SERIAL:
        return tr("Serial");
    case BUS_USB:
        return tr("USB");
    case BUS_PCIE:
        return tr("PCIE");
    case BUS_ANY:
        return tr("Any");
    case BUS_OTHER:
        return tr("Other");
    }
    return QString();
}
QString AttributeWindow::transferIdentifyType(int type)
{
    switch(type) {
    case VERIFY_HARDWARE:
        return tr("Hardware Identification");
    case VERIFY_SOFTWARE:
        return tr("Software Identification");
    case VERIFY_MIX:
        return tr("Mix Identification");
    case VERIFY_OTHER:
        return tr("Other Identification");
    }
    return QString();
}



void AttributeWindow::setupInit()
{
    setWindowTitle(tr(""));
    setAttribute(Qt::WA_TranslucentBackground);
    //setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::Dialog);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

}

void AttributeWindow::paintEvent(QPaintEvent * event){
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 6, 6);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();
}
