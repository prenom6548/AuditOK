/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#include "bioauthwidget.h"
#include "ui_bioauthwidget.h"
#include <QImageReader>
#include <QMovie>
#include "generic.h"
#include <unistd.h>
#include <pwd.h>
#include "giodbus.h"
#include "biodeviceswidget.h"

int BioAuthWidget::failedCount = 0;
BioAuthWidget::BioAuthWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BioAuthWidget),
    bioAuth(nullptr)
{
    nMaxFailCnt = GetFailedTimes();
    ui->setupUi(this);
    ui->btnRetry->setFlat(true);
    ui->btnRetry->setVisible(false);
    ui->btnRetry->setText(tr("Retry"));
    //ui->btnMore->setVisible(false);
}

BioAuthWidget::~BioAuthWidget()
{
    delete ui;
}

void BioAuthWidget::emitSwithToPassword()
{
    Q_EMIT switchToPassword();
}

void BioAuthWidget::on_btnPasswdAuth_clicked()
{
    stopAuth();
    //Q_EMIT switchToPassword();
    QTimer::singleShot(100,this,SLOT(emitSwithToPassword()));
}

void BioAuthWidget::on_btnMore_clicked()
{
    Q_EMIT selectDevice();
}

void BioAuthWidget::on_btnRetry_clicked()
{
    if(bioAuth && !bioAuth->isAuthenticating()) {
        setMovie();
        bioAuth->startAuth();
    }
}

void BioAuthWidget::onBioAuthNotify(const QString &notifyMsg)
{
    ui->lblBioNotify->setText(notifyMsg);
}

void BioAuthWidget::onBioAuthComplete(uid_t uid, bool ret, int retErrNo)
{
    if (!device)
        return ;
    if (ret == true)
    {
        BioAuthWidget::failedCount = 0;
    }else{
        BioAuthWidget::failedCount++;
    }
    
    setImage();
    dup_fd = -1;
    if(BioAuthWidget::failedCount >= nMaxFailCnt)
    {
        // ui->lblBioWarnNotify->setText(tr("Too many unsuccessful attempts,please enter password."));
        ui->lblBioWarnNotify->setText(tr("%1 too many unsuccessful attempts,please enter password.").arg(BioDevices::bioTypeToString_tr(device->biotype)));
    }
    else
    {
        ui->btnRetry->setVisible(true);
        // ui->lblBioWarnNotify->setText(tr("Authentication failure,there are still %1 remaining opportunities").arg(nMaxFailCnt - BioAuthWidget::failedCount));
        ui->lblBioWarnNotify->setText(tr("%1 authentication failure,there are still %2 remaining opportunities").arg(BioDevices::bioTypeToString_tr(device->biotype)).arg(nMaxFailCnt - BioAuthWidget::failedCount));
    }
    
    Q_EMIT authComplete(uid, ret);
}

void BioAuthWidget::setMovie()
{
    if(!device || device->biotype == BIOTYPE_FACE)
        return ;

    QString typeString = bioTypeToString(device->biotype);
    QString moviePath = QString("%1/images/%2.gif").arg(GET_STR(UKUI_BIOMETRIC)).arg(typeString);
    QMovie *movie = new QMovie(moviePath);
    movie->setScaledSize(QSize(ui->lblBioImage->width(), ui->lblBioImage->height()));

    ui->lblBioImage->setMovie(movie);
    movie->start();

    ui->btnRetry->setVisible(false);

    qDebug() << "set movie " << moviePath;
}

void BioAuthWidget::setImage()
{
    if(!device || device->biotype == BIOTYPE_FACE)
        return ;
    QString typeString = bioTypeToString(device->biotype);
    QString pixmapPath = QString("%1/images/%2.png").arg(GET_STR(UKUI_BIOMETRIC)).arg(typeString);
    QPixmap pixmap(pixmapPath);
    pixmap = pixmap.scaled(ui->lblBioImage->width(), ui->lblBioImage->height(),
                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ui->lblBioImage->setPixmap(pixmap);

    if(BioAuthWidget::failedCount >= nMaxFailCnt)
    {
        ui->btnRetry->setVisible(false);
    }
    else{
        ui->btnRetry->setVisible(true);
    }

    qDebug() << "set pixmap " << typeString;
}

void BioAuthWidget::onFrameWritten(int deviceId)
{
    if(dup_fd == -1){
        dup_fd = get_server_gvariant_stdout(deviceId);
    }

    lseek(dup_fd, 0, SEEK_SET);
    char base64_bufferData[1024*1024];
    int rc = read(dup_fd, base64_bufferData, 1024*1024);
    printf("rc = %d\n", rc);

    if (rc > 0) {
        QByteArray imageData(base64_bufferData, rc);
        QBuffer imgBuffer(&imageData);
        imgBuffer.open(QIODevice::ReadOnly);

        QImageReader imgReader(&imgBuffer);
        QImage image = imgReader.read();
        if (image.isNull()) {
            return;
        }
        image = image.convertToFormat(QImage::Format_RGB888);
        ui->lblBioImage->setPixmap(QPixmap::fromImage(image).scaled(ui->lblBioImage->size()));
        ui->btnRetry->setVisible(false);
    }
}

void BioAuthWidget::hidePasswdButton()
{
    //ui->btnPasswdAuth->hide();
}

bool BioAuthWidget::isAuthenticating()
{
    if(bioAuth){
    	return bioAuth->isAuthenticating();
    }
    return false;
}

void BioAuthWidget::stopAuth()
{
    if(bioAuth){
        bioAuth->stopAuth();
    }
}

void BioAuthWidget::startAuth(uid_t uid, const DeviceInfoPtr device)
{
    if (!device) {
        qDebug()<<"Device is invalid!!";
        return ;
    }
    this->uid = uid;
    this->device = device;

    //ui->lblBioDevice->setText(tr("Current Device: ") + device.device_shortname);

    if(bioAuth) {
        bioAuth->stopAuth();
        delete bioAuth;
        bioAuth = nullptr;
    }

    setMovie();

    if(BioAuthWidget::failedCount >= nMaxFailCnt)
    {
        // ui->lblBioWarnNotify->setText(tr("Too many unsuccessful attempts,please enter password."));
        ui->lblBioWarnNotify->setText(tr("%1 too many unsuccessful attempts,please enter password.").arg(BioDevices::bioTypeToString_tr(device->biotype)));
        return;
    }
    else
    {
        ui->lblBioWarnNotify->setText("");
    }

    bioAuth = new BioAuth(uid, device, this);
    connect(bioAuth, &BioAuth::notify, this, &BioAuthWidget::onBioAuthNotify);
    connect(bioAuth, &BioAuth::authComplete, this, &BioAuthWidget::onBioAuthComplete);
    connect(bioAuth, &BioAuth::frameWritten,this,&BioAuthWidget::onFrameWritten);

    if (device->biotype == REMOTE_QRCODE_TYPE) {
        onBioAuthNotify(tr("Please use wechat to scan the code"));
    }

    dup_fd = -1;
    fd = -1;
    bioAuth->startAuth();

}

void BioAuthWidget::setMoreDevices(bool hasMore)
{
    //ui->btnMore->setVisible(hasMore);
}
