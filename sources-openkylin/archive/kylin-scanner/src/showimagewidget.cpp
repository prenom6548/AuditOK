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
#include "showimagewidget.h"
#include <QDebug>
#include <KWindowEffects>
#include <kysdk/applications/gsettingmonitor.h>

#include "imageOp/imageoperationbeauty.h"
#include "imageOp/imageoperationmirror.h"
#include "imageOp/imageoperationocr.h"
#include "imageOp/imageoperationrectify.h"

ShowImageWidget::ShowImageWidget(QWidget *parent) : QWidget(parent)
  , m_stackImage(new QImage())
  , m_editImage(new QImage())
  , m_normalImage(new QImage())
  , m_showImageLabel(new QLabel())
  , m_showImageAndCropWidget (new QStackedWidget(this))
  , m_cropLabel(new CropLabel())
  , m_cancelOkWidget(new QWidget(this))
  , m_cancelOkLayout(new QHBoxLayout())
  , m_cancelButton(new QPushButton())
  , m_okButton(new QPushButton())
  , m_showImageHLayout(new QHBoxLayout())
  , m_toolbarWidget(new ToolBarWidget())
  , m_mainVLayout(new QVBoxLayout())
  , scannerImagePath(g_config_signal->m_scannerImagePath)
  , m_navigator(new Navigator(this))
{
    loadImg = new LoadImage;
    loadImg->moveToThread(&loadImgOP);
    loadImgOP.start();
    connect(this, &ShowImageWidget::loadImgSignal, loadImg, &LoadImage::loadImageToWidget, Qt::QueuedConnection);
    connect(loadImg, &LoadImage::finished, this, &ShowImageWidget::loadImageFinishedSlot);
    setupGui();
    initConnect();
}

ShowImageWidget::~ShowImageWidget()
{
    if (loadImg) {
        if (loadImgOP.isRunning()) {
            QMetaObject::invokeMethod(loadImg, "deleteLater", Qt::QueuedConnection);
            loadImgOP.quit();
            if (!loadImgOP.wait(3000)) {
                loadImgOP.terminate();
                loadImgOP.wait();
            }
        } else {
            delete loadImg;
        }
        loadImg = nullptr;
    }

    if (imageOpThread) {
        imageOpThread->quit();
        if (!imageOpThread->wait(3000)) {
            imageOpThread->terminate();
            imageOpThread->wait();
        }
        delete imageOpThread;
        imageOpThread = nullptr;
    }

    if (imageOpOcrThread) {
        imageOpOcrThread->quit();
        if (!imageOpOcrThread->wait(3000)) {
            imageOpOcrThread->terminate();
            imageOpOcrThread->wait();
        }
        delete imageOpOcrThread;
        imageOpOcrThread = nullptr;
    }
}

void ShowImageWidget::setupGui()
{
    this->setMinimumSize(QSize(ShowImageWidgetWidth, ShowImageWidgetHeight));
    this->setFocusPolicy(Qt::StrongFocus);

    m_cropLabel->setMinimumSize(QSize(defaultImageLableWidth*0.05, defaultImageLableHeight*0.05));

    m_cancelOkWidget->setFixedSize(CancelOkWidgetFixedSize);
    m_cancelButton->setFixedSize(QSize(20, 20));
    m_cancelButton->setIconSize(QSize(20, 20));
    m_cancelButton->setToolTip(tr("Cancel"));
    m_cancelButton->setFocusPolicy(Qt::NoFocus);
    m_cancelButton->setStyleSheet("QPushButton{border:0px; border-radius: 6px; background:transparent; border-image:url(:/cancel-button.png);}"
                                  "QPushButton::hover{border:0px; border-radius: 6px; background:transparent; border-image:url(:/cancel-button-hover.png);}"
                                  "QPushButton::pressed{border:0px; border-radius: 6px; background:transparent; border-image:url(:/cancel-button-click.png);}");

    m_okButton->setFixedSize(QSize(20, 20));
    m_okButton->setIconSize(QSize(20, 20));
    m_okButton->setToolTip(tr("Ok"));
    m_okButton->setFocusPolicy(Qt::NoFocus);
    m_okButton->setStyleSheet("QPushButton{border:0px; border-radius: 6px; background:transparent; border-image:url(:/ok-button.png);}"
                              "QPushButton::hover{border:0px; border-radius: 6px; background:transparent; border-image:url(:/ok-button-hover.png);}"
                              "QPushButton::pressed{border:0px; border-radius: 6px; background:transparent; border-image:url(:/ok-button-click.png);}");
    m_cancelOkLayout->addWidget(m_cancelButton);
    m_cancelOkLayout->addSpacing(12);
    m_cancelOkLayout->addWidget(m_okButton);
    m_cancelOkWidget->setLayout(m_cancelOkLayout);
    m_cancelOkWidget->hide();

    m_showImageLabel->setMinimumSize(QSize(387*0.05, 536*0.05));
    m_showImageLabel->installEventFilter(this);

    m_waitingImageLabel = new QLabel(this);
    m_waitingImageLabel->setFixedSize(128,128);
    m_waitingImageLabel->move(int((this->width() - m_waitingImageLabel->width())/2),int((this->height() - m_waitingImageLabel->height())/2));

    m_labelWidget = new QWidget(m_showImageAndCropWidget);
    m_labelWidget->setMinimumSize(QSize(defaultImageLableWidth*0.05, defaultImageLableHeight*0.05));
    m_stackedLayout = new QStackedLayout(m_labelWidget);
    m_stackedLayout->addWidget(m_showImageLabel);
    m_stackedLayout->addWidget(m_waitingImageLabel);
    m_stackedLayout->setCurrentWidget(m_showImageLabel);
    m_labelWidget->setLayout(m_stackedLayout);
    m_stackedLayout->setAlignment(m_waitingImageLabel,Qt::AlignHCenter);

    m_showImageAndCropWidget->addWidget(m_cropLabel);
    m_showImageAndCropWidget->addWidget(m_labelWidget);
    m_showImageAndCropWidget->setCurrentWidget(m_labelWidget);

    m_showImageHLayout->setSpacing(0);
    m_showImageHLayout->addSpacing(128);
    m_showImageHLayout->addStretch();
    m_showImageHLayout->addWidget(m_showImageAndCropWidget, 0, Qt::AlignCenter);
    m_showImageHLayout->addStretch();
    m_showImageHLayout->addSpacing(128);
    m_showImageHLayout->setContentsMargins(0, 0, 0, 0);

    m_mainVLayout->setSpacing(0);
    m_mainVLayout->addSpacing(24);
    m_mainVLayout->addStretch();
    m_mainVLayout->addLayout(m_showImageHLayout);
    m_mainVLayout->addSpacing(24);
    m_mainVLayout->addStretch();
    m_mainVLayout->addWidget(m_toolbarWidget, 0, Qt::AlignCenter);
    m_mainVLayout->addSpacing(16);
    m_mainVLayout->setContentsMargins(0, 0, 0, 0);

    m_loadingMovie = new QMovie(":/loadgif.gif");

    this->setLayout(m_mainVLayout);
    m_navigator->move(this->width() - 7 - m_navigator->width(), this->height() - 52 - m_navigator->height());
    m_navigator->hide();
}

void ShowImageWidget::initConnect()
{
    connect(g_user_signal, &GlobalUserSignal::scanThreadFinishedImageLoadSignal, this, &ShowImageWidget::showNormalImageAfterScan, Qt::QueuedConnection);

    connect(g_user_signal, &GlobalUserSignal::showImageAfterClickedThumbnailSignal, this, &ShowImageWidget::showImageAfterClickedThumbnail);

    connect(g_user_signal, &GlobalUserSignal::saveAsButtonClickedSignal, this, &ShowImageWidget::saveImage);

    connect(g_user_signal, &GlobalUserSignal::toolbarCropOperationSignal, this, &ShowImageWidget::cropSlot);
    connect(g_user_signal, &GlobalUserSignal::toolbarRotateOperationSignal, this, &ShowImageWidget::rotateSlot);
    connect(g_user_signal, &GlobalUserSignal::toolbarMirrorOperationSignal, this, &ShowImageWidget::mirrorSlot);
    connect(g_user_signal, &GlobalUserSignal::toolbarWatermarkOperationSignal, this, &ShowImageWidget::watermarkSlot);

    connect(g_user_signal, &GlobalUserSignal::toolbarZoomoutOperationSignal, this, &ShowImageWidget::zoomoutNormalImage);
    connect(g_user_signal, &GlobalUserSignal::toolbarZoominOperationSignal, this, &ShowImageWidget::zoominNormalImage);

    connect(g_user_signal, &GlobalUserSignal::toolbarBeautyOperationStartSignal, this, &ShowImageWidget::beautyStartSlot);
    connect(g_user_signal, &GlobalUserSignal::doBeautyOperationFinishedSignal, this, &ShowImageWidget::beautyStopSlot);

    connect(g_user_signal, &GlobalUserSignal::toolbarRectifyOperationStartSignal, this, &ShowImageWidget::rectifyStartSlot);
    connect(g_user_signal, &GlobalUserSignal::doRectifyOperationFinishedSignal, this, &ShowImageWidget::rectifyStopSlot);

    connect(g_user_signal, &GlobalUserSignal::toolbarOcrOperationStartSignal, this, &ShowImageWidget::ocrStartSlot);

    connect(g_user_signal, &GlobalUserSignal::cancleCorpSignal, m_cancelButton, &QPushButton::click);
    connect(m_cancelButton, &QPushButton::clicked, this, &ShowImageWidget::cropCancelSlot);
    connect(m_okButton, &QPushButton::clicked, this, &ShowImageWidget::cropCompleteSlot);

    connect(m_cropLabel, &CropLabel::paintCompleteSignal, this, &ShowImageWidget::setCancelWidgetPositon);
    connect(m_cropLabel, &CropLabel::hideCancelOkWidget, this, [=](){m_cancelOkWidget->hide();});

    connect(this, &ShowImageWidget::showNavigation, m_navigator, &Navigator::showNavigation);
    connect(m_navigator, &Navigator::naviChange, this, &ShowImageWidget::naviChange);
    connect(this, &ShowImageWidget::sendHightlightPos, m_navigator, &Navigator::getHighLightRegion);
    connect(g_user_signal, &GlobalUserSignal::posChange, this, &ShowImageWidget::clickNavigation);
    connect(g_user_signal, &GlobalUserSignal::resetNavigatorsignal, this, &ShowImageWidget::resetNavigator);

}

constexpr inline int U(const char *str)
{
    return str[0] + (str[1] ? U(str + 1) : 0);
}

int ShowImageWidget::toUnicode(QString str)
{
    char  *ch;
    QByteArray ba = str.toLatin1();
    ch = ba.data();
    return ch[0] + (ch[1] ? toUnicode(ch + 1) : 0);

}

void ShowImageWidget::setPdfSize(QPdfWriter *pdfWriter, QString size)
{
    switch (toUnicode(size)) {
    case U("A0"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A0));
        break;
    case U("A1"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A1));
        break;
    case U("A2"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A2));
        break;
    case U("A3"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A3));
        break;
    case U("A4"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A4));
        break;
    case U("A5"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A5));
        break;
    case U("A6"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A6));
        break;
    case U("A7"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A7));
        break;
    case U("A8"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A8));
        break;
    case U("A9"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A9));
        break;
    default:
        pdfWriter->setPageSize(QPageSize(QPageSize::A4));
        break;
    }

}

void ShowImageWidget::saveToPdf(QImage img, QString pathname)
{
    QFile pdfFile(pathname);
    pdfFile.open(QIODevice::WriteOnly);
    QPdfWriter *pdfWriter = new QPdfWriter(&pdfFile);
    setPdfSize(pdfWriter, g_sane_object->userInfo.size);

    int resolution = g_sane_object->resolutionValue;
    pdfWriter->setResolution(resolution);

    int pageMargin = 0;
    pdfWriter->setPageMargins(QMarginsF(pageMargin, pageMargin, pageMargin, pageMargin));

    QPainter *pdfPainter = new QPainter(pdfWriter);

    int yCurrentP = 0;
    int xCurrentP = 0;

    QPixmap pixmap = QPixmap::fromImage(img);
    pdfPainter->drawPixmap(xCurrentP, yCurrentP, pixmap.width(), pixmap.height(), pixmap);

    pdfPainter->end();
    delete pdfPainter;
    delete pdfWriter;
    pdfFile.close();
}

QImage *ShowImageWidget::imageSave(QString filename)
{
    qDebug() << "save image: " << filename;
    if (g_sane_object->ocrFlag == 0) {
        /*****************mark***************************/
        *m_editImage = m_normalImage->copy();
        /*****************mark***************************/
        if (filename.endsWith(".pdf"))
            return m_editImage;
        if (filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".tiff"))
            m_editImage->save(filename);
    } else {
        if (!filename.endsWith(".txt"))
            filename += ".txt";
        QFile file(filename);
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        QByteArray str = g_sane_object->ocrOutputText.toUtf8();
        file.write(str);
        file.close();
    }
    return NULL;
}

QString ShowImageWidget::setPixmapScaled(QImage img, QLabel *lab, double scale)
{
    if (img.isNull()) {
        qWarning() << "image is null.";
        return "";
    }
    double labWidth = this->width() - 2*ShowImageWidgetSpacing - AddWidthForLargeFontSize;
    double labHeight = this->height() - 24 -24 - 36 -16;

    double imgWidth = img.width();
    double imgHeight = img.height();


    qDebug() << "label size: " << lab->size()
             << "this size: " << this->size()
             << "this labsize: " << labWidth << labHeight
             << "defaultImage size: " << defaultScanImageSize
             << "scale: " << scale
             << "image size: " << img.size();

    if (! qFuzzyCompare(scale, 1.0)) {
        proportion = scale;
    } else {
        if ((labWidth / imgWidth) <= (labHeight / imgHeight)) {
            proportion = labWidth / imgWidth;
        } else {
            proportion = labHeight / imgHeight;
        }

    }

    proportionForPercentage = proportion;

    QPixmap pixmap = QPixmap::fromImage(img);
    m_nowImage = pixmap;
    QPixmap fitpixmap = pixmap.scaled(img.size()*proportion, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if(fitpixmap.width() > labWidth || fitpixmap.height() > labHeight){
        createNavigation();

        int proportionInt = qRound(proportionForPercentage * 100);
        QString proportionString = QString("%1").arg(proportionInt) + "%";

        return proportionString;
    }else{
        lab->setPixmap(fitpixmap);
        lab->setFixedSize(QSize(fitpixmap.size()));
        lab->setScaledContents(true);
        lab->setAlignment(Qt::AlignCenter | Qt::AlignCenter | Qt::AlignCenter);
        m_navigator->hide();

        qDebug() << "proportion: " << proportion << "proportionForPercentage: " << proportionForPercentage;

        int proportionInt = qRound(proportionForPercentage * 100);

        qDebug() << "proportionInt: " << proportionInt;
        QString proportionString = QString("%1").arg(proportionInt) + "%";

        qDebug() << "proportionString: " << proportionString;
        g_sane_object->percentage = proportionString;

        g_user_signal->toolbarPercentageChanged();

        return proportionString;
    }
}

QString ShowImageWidget::setCropPixmapScaled(QImage img, QLabel *lab, double scale)
{
    if (img.isNull()) {
            qWarning() << "image is null.";
            return "";
        }
        double labWidth = this->width() - 2*ShowImageWidgetSpacing - AddWidthForLargeFontSize;
        double labHeight = this->height() - 24 -24 - 36 -16;

        double imgWidth = defaultScanImageSize.width();
        double imgHeight = defaultScanImageSize.height();

        if(rotationFlag){
            std::swap(imgWidth,imgHeight);
        }

        qDebug() << "label size: " << lab->size()
                 << "this size: " << this->size()
                 << "this labsize: " << labWidth << labHeight
                 << "defaultImage size: " << defaultScanImageSize
                 << "scale: " << scale
                 << "image size: " << img.size();

        if (! qFuzzyCompare(scale, 1.0)) {
            proportion = scale;
        } else {
            if ((labWidth / imgWidth) <= (labHeight / imgHeight)) {
                proportion = labWidth / imgWidth;
            } else {
                proportion = labHeight / imgHeight;
            }

        }

        proportionForPercentage = proportion;

        QPixmap pixmap = QPixmap::fromImage(img);
        QPixmap fitpixmap = pixmap.scaled(img.size()*proportion, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        lab->setPixmap(fitpixmap);
        lab->setFixedSize(QSize(fitpixmap.size()));
        lab->setScaledContents(true);
        lab->setAlignment(Qt::AlignCenter | Qt::AlignVCenter | Qt::AlignHCenter);

        qDebug() << "proportion: " << proportion
                 << "proportionForPercentage: " << proportionForPercentage;

        int proportionInt = qRound(proportionForPercentage * 100);

        qDebug() << "proportionInt: " << proportionInt;
        QString proportionString = QString("%1").arg(proportionInt) + "%";

        qDebug() << "proportionString: " << proportionString;
        g_sane_object->percentage = proportionString;

        g_user_signal->toolbarPercentageChanged();

        return proportionString;

}

QImage ShowImageWidget::pictureDeepen(const QImage &img, const QSize &hightlightSize, const QPoint &point)
{
    QImage image = img.copy();
    int key = 50;
    int left = point.x();
    int right = point.x() + hightlightSize.width();
    int top = point.y();
    //边界值限定
    int bottom = point.y() + hightlightSize.height();
    right = boundaryJudg(img.width(), right);
    bottom = boundaryJudg(img.height(), bottom);
    //如果有透明通道
    bool hasAlpha = false;
    if (image.format() == QImage::Format_ARGB32 || image.format() == QImage::Format_ARGB32_Premultiplied) {
        hasAlpha = true;
    }

    for (int j = 0; j < image.height(); ++j) {
        for (int i = 0; i < image.width(); ++i) {
            if (i >= left && i < right && j >= top && j < bottom) {
                continue; //高亮区域不处理
            }
            QColor color(image.pixel(i, j));
            if (hasAlpha && color.red() + color.green() + color.blue() == 0) { //透明区域40%透明度的黑色
                color.setAlphaF(0.4);
                image.setPixel(i, j, color.rgba());
                continue;
            }
            color.setRed(minNumIsZero(color.red(), key));
            color.setGreen(minNumIsZero(color.green(), key));
            color.setBlue(minNumIsZero(color.blue(), key));
            image.setPixel(i, j, color.rgb());
        }
    }
    //绘制边框-画内边界
    for (int i = left; i < right; ++i) {
        i = boundaryJudg(right, i);
        QColor colorDown(img.pixel(i, top));
        colorDown.setRed(255);
        colorDown.setGreen(255);
        colorDown.setBlue(255);
        image.setPixel(i, top, colorDown.rgb());
        QColor colorTop(img.pixel(i, bottom - 1));
        colorTop.setRed(255);
        colorTop.setGreen(255);
        colorTop.setBlue(255);
        image.setPixel(i, bottom - 1, colorTop.rgb());
    }
    for (int k = top; k < bottom; ++k) {
        k = boundaryJudg(bottom, k);
        QColor colorLeft(img.pixel(left, k));
        colorLeft.setRed(255);
        colorLeft.setGreen(255);
        colorLeft.setBlue(255);
        image.setPixel(left, k, colorLeft.rgb());
        QColor colorRight(img.pixel(right - 1, k));
        colorRight.setRed(255);
        colorRight.setGreen(255);
        colorRight.setBlue(255);
        image.setPixel(right - 1, k, colorRight.rgb());
    }
    return image;
}

int ShowImageWidget::boundaryJudg(int max, int point)
{
    if (point < 0) {
        point = 0;
    }
    if (point > max) {
        point = max;
    }
    return point;
}

int ShowImageWidget::minNumIsZero(const int &num1, const int &num2)
{
    int num = num1 - num2;
    if (num < 0) {
        return 0;
    }
    return num;
}

void ShowImageWidget::setPixmapScaledByProportion(double scaledNumber)
{
    int scaledWidth = static_cast<int>(m_showImageLabel->width() * scaledNumber);
    int scaledHeight = static_cast<int>(m_showImageLabel->height() * scaledNumber);

    qDebug() << "scaledNumber = " << scaledNumber
             << "scaleWidth = " << scaledWidth
             << "scaleHeight = " << scaledHeight;

    *m_editImage = m_normalImage->copy();

    m_editImage->scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    *m_normalImage = m_editImage->copy();
    m_showImageLabel->setPixmap(QPixmap::fromImage(*m_normalImage));
    m_showImageLabel->setScaledContents(true);
}

void ShowImageWidget::getCropArea(int &x1, int &y1, int &x2, int &y2)
{
    if (m_cropLabel->getBeginX() <= m_cropLabel->getEndX()) {
        x1 = m_cropLabel->getBeginX();
        x2 = m_cropLabel->getEndX();
    } else {
        x1 = m_cropLabel->getEndX();
        x2 = m_cropLabel->getBeginX();
    }

    if (m_cropLabel->getBeginY() <= m_cropLabel->getEndY()) {
        y1 = m_cropLabel->getBeginY();
        y2 = m_cropLabel->getEndY();
    } else {
        y1 = m_cropLabel->getEndY();
        y2 = m_cropLabel->getBeginY();
    }

}

double ShowImageWidget::getCurrentPercentage()
{
    QString proportionNoPercentage = g_sane_object->percentage.replace("%", "");

    double percentageDouble;

    percentageDouble = proportionNoPercentage.toDouble() / 100;
    qDebug() << "percentageDouble = " << percentageDouble
             << "proportionNoPercentage = " << proportionNoPercentage;

    qDebug() << "percentageDouble = " << percentageDouble;


    return percentageDouble;

}

double ShowImageWidget::getRealPercentage(ShowImageWidget::ZoomStatus zoomType)
{
    QString proportionNoPercentage = g_sane_object->percentage.replace("%", "");

    double percentageDouble;

    if (zoomType == ZoomStatus::ZoomOut) {
        percentageDouble = proportionNoPercentage.toDouble() / 100 - 0.05;
    } else {
        percentageDouble = proportionNoPercentage.toDouble() / 100 + 0.05;
    }
    qDebug() << "percentageDouble = " << percentageDouble
             << "proportionNoPercentage = " << proportionNoPercentage;

    qDebug() << "percentageDouble = " << percentageDouble;

    if (zoomType == ZoomStatus::ZoomOut) {
        if (percentageDouble < 0.05) {
            percentageDouble = 0.05;
        }
    qDebug() << "percentageDouble = " << percentageDouble;
    } else if (zoomType == ZoomStatus::ZoomIn) {
        if (percentageDouble > 2) {
            percentageDouble = 2;
        }
    qDebug() << "percentageDouble = " << percentageDouble;
    }

    return percentageDouble;
}

void ShowImageWidget::updatePercentageByZoom(double scale)
{
    int percentageInt = qRound(scale * 100.0);

    QString percentageString = QString::number(percentageInt) + "%";
    qDebug() << "percentage: " << percentageString;

    g_sane_object->percentage = percentageString;

    g_user_signal->toolbarPercentageChanged();

}

void ShowImageWidget::resizeEvent(QResizeEvent *event)
{
    setPixmapScaled(*m_normalImage, m_showImageLabel);

    if(!m_cancelOkWidget->isHidden()){
        setCancelWidgetPositon();
    }

    QWidget::resizeEvent(event);
}
void ShowImageWidget::rollBackOperation(){
    if (! m_imageStack.isEmpty()) {

        if (m_showImageAndCropWidget->currentWidget() == m_cropLabel) {
            *m_editImage = m_imageStack.pop();
            setCropPixmapScaled(*m_editImage, m_cropLabel);

            *m_normalImage = m_editImage->copy();
            setCropPixmapScaled(*m_normalImage, m_showImageLabel);

            m_showImageAndCropWidget->setCurrentWidget(m_labelWidget);
            g_sane_object->cropFlag = 0;
        } else {
            *m_editImage = m_imageStack.pop();
            setPixmapScaled(*m_editImage, m_showImageLabel);

            *m_normalImage = m_editImage->copy();
            setPixmapScaled(*m_normalImage, m_showImageLabel);

            m_showImageAndCropWidget->setCurrentWidget(m_showImageLabel);
            g_sane_object->cropFlag = 0;
        }
    }
    if(m_imageStack.isEmpty()){
        g_user_signal->exitWindowWithSaveFlag = false;
    }
}

void ShowImageWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:

        if (m_showImageAndCropWidget->currentWidget() == m_cropLabel) {
            qDebug() << "pressed key(Enter): " << event->key();

            int x1, y1, x2, y2;
            getCropArea(x1, y1, x2, y2);

            QImage cropImage = m_normalImage->copy(x1/proportion, y1/proportion, (x2-x1)/proportion, (y2-y1)/proportion);

            *m_editImage = cropImage;
            setPixmapScaled(*m_editImage, m_cropLabel, proportion);

            *m_normalImage = m_editImage->copy();
            setPixmapScaled(*m_normalImage, m_showImageLabel, proportion);

            m_cropLabel->initCropSettings();


            m_showImageAndCropWidget->setCurrentWidget(m_labelWidget);

            g_sane_object->cropFlag = 0;
            m_cancelOkWidget->hide();
        }
        break;

    case Qt::Key_Z:
        if (event->modifiers() == Qt::ControlModifier) {
            qDebug() << "pressed key(ctrl+z): " << event->key();
            rollBackOperation();
            m_cancelOkWidget->hide();
        }
        break;

    default:
        qDebug() << "pressed key: " << event->key();
        break;
    }
}

bool ShowImageWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_showImageLabel) {
        if (event->type()==QEvent::Wheel) {
            QWheelEvent *wheelEvent=static_cast<QWheelEvent *>(event);
            if (g_sane_object->cropFlag == 1) {
                return QObject::eventFilter(obj,event);
            }
            if (wheelEvent->delta()>0) {
                zoominNormalImage();
            } else {
                zoomoutNormalImage();
            }
        }
    }

    return QObject::eventFilter(obj,event);
}

void ShowImageWidget::showNormalImageAfterScan(QStringList loadFileName, QStringList saveFileName)
{
    m_imageStack.clear();
    proportionForPercentage = 1.0;

    m_showImageAndCropWidget->setCurrentWidget(m_labelWidget);

    imageNUll(true);

    double labWidth = this->width() - 2*ShowImageWidgetSpacing - AddWidthForLargeFontSize;
    double labHeight = this->height() - 24 -24 - 36 -16;
    QSize labSize(labWidth,labHeight);

    for(int i = 0; i < loadFileName.length(); i++){
        m_loadPathDelayUpdate = loadFileName[i];
        m_savePathDelayUpdate = saveFileName[i];
        m_normalImageList.append(m_normalImage);
        emit loadImgSignal(m_loadPathDelayUpdate, m_savePathDelayUpdate, m_normalImage, m_showImageLabel, 1, false, labSize);
    }
}

void ShowImageWidget::imageNUll(bool res)
{
    if (res) {
        m_stackedLayout->setCurrentWidget(m_waitingImageLabel);
        m_waitingImageLabel->setMovie(m_loadingMovie);
        m_loadingMovie->start();
        m_waitingImageLabel->move(int((m_labelWidget->width() - m_waitingImageLabel->width())/2),int((m_labelWidget->height() - m_waitingImageLabel->height())/2));
    } else {
        m_stackedLayout->setCurrentWidget(m_showImageLabel);
        if (m_loadingMovie->state() != QMovie::NotRunning) {
            m_loadingMovie->stop();
        }
    }
}

void ShowImageWidget::setCancelWidgetPositon()
{
    m_cancelOkWidget->show();
    qDebug()  << "this height: " << this->height();
    qDebug()  << "m_showImageLabel height: " << m_showImageLabel->height();

    m_cancelOkWidget->move(int(this->width() / 2 - ThumbnailWidgetMinimumWidth + 16), (this->height()-150));
}

void ShowImageWidget::drawShadow()
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0); //设置向哪个方向产生阴影效果(dx,dy)，特别地，(0,0)代表向四周发散
    effect->setColor(QColor(220,220,220));       //设置阴影颜色，也可以setColor()
    effect->setBlurRadius(8); //设定阴影的模糊半径，数值越大越模糊
    m_cancelOkWidget->setGraphicsEffect(effect);
}

void ShowImageWidget::createNavigation()
{
    // 缩略图大小
    QSize navigationSize = QSize(130, 133);
    m_navigationImage = m_nowImage.scaled(navigationSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage();

    //记录空白区域
    m_spaceWidth = (navigationSize.width() - m_navigationImage.width()) / 2;
    m_spaceHeight = (navigationSize.height() - m_navigationImage.height()) / 2;

    //待显示图
    m_tmpSize = m_nowImage.size() * qRound(proportionForPercentage * 100) / 100;

    double labWidth = this->width() - 2*ShowImageWidgetSpacing - AddWidthForLargeFontSize;
    double labHeight = this->height() - 24 -24 - 36 -16;

    //高亮区域大小
    m_hightlightSize.setWidth(m_navigationImage.width() * labWidth / m_tmpSize.width());
    m_hightlightSize.setHeight(m_navigationImage.height() * labHeight / m_tmpSize.height());
    if (m_hightlightSize.width() > m_navigationImage.width()) {
        m_hightlightSize.setWidth(m_navigationImage.width());
    }
    if (m_hightlightSize.height() > m_navigationImage.height()) {
        m_hightlightSize.setHeight(m_navigationImage.height());
    }

    clickNavigation(m_clickBeforePosition);
}

void ShowImageWidget::clickNavigation(const QPoint &point)
{
    bool hasArg = true;
    //无参输入则使用上次的位置
    if (point == QPoint(-1, -1)) {
        hasArg = false;
    }
    //有参则记录，无参使用上一次的点
    if (hasArg) {
        m_clickBeforePosition = point;
    }

    QPoint startPoint(m_clickBeforePosition.x() - m_hightlightSize.width() / 2 - m_spaceWidth,
                      m_clickBeforePosition.y() - m_hightlightSize.height() / 2 - m_spaceHeight);
    int right = m_navigationImage.width() - m_hightlightSize.width();       //右侧边缘
    int bottom = m_navigationImage.height() - m_hightlightSize.height();    //下侧边缘

    //过滤无效区域
    if (startPoint.x() < 0) {
        startPoint.setX(0);
    }
    if (startPoint.y() < 0) {
        startPoint.setY(0);
    }
    if (startPoint.x() > right) {
        startPoint.setX(right);
    }
    if (startPoint.y() > bottom) {
        startPoint.setY(bottom);
    }

    QImage deepenImg = pictureDeepen(m_navigationImage, m_hightlightSize, startPoint);

    // 发送到导航器
    Q_EMIT showNavigation(QPixmap::fromImage(deepenImg));
    // 发送到导航器设置
    Q_EMIT sendHightlightPos(QPoint(startPoint.x() + m_spaceWidth, startPoint.y() + m_spaceHeight),
                             QPoint(startPoint.x() + m_hightlightSize.width() + m_spaceWidth,
                                    startPoint.y() + m_hightlightSize.height() + m_spaceHeight));

    double labWidth = this->width() - 2 * ShowImageWidgetSpacing - AddWidthForLargeFontSize;
    double labHeight = this->height() - 24 - 24 - 36 -16;

    QSize m_size(labWidth, labHeight);

    QPoint startShowPoint = startPoint * m_tmpSize.width() / m_navigationImage.width();
    QPixmap result = localAmplification(m_nowImage, m_tmpSize, startShowPoint, m_size);
    m_showImageLabel->setPixmap(result);
    m_showImageLabel->setFixedSize(QSize(result.size()));
    m_showImageLabel->setScaledContents(true);
    m_showImageLabel->setAlignment(Qt::AlignCenter | Qt::AlignCenter | Qt::AlignCenter);

    qDebug() << "proportion: " << proportion << "proportionForPercentage: " << proportionForPercentage;

    int proportionInt = qRound(proportionForPercentage * 100);

    qDebug() << "proportionInt: " << proportionInt;
    QString proportionString = QString("%1").arg(proportionInt) + "%";

    qDebug() << "proportionString: " << proportionString;
    g_sane_object->percentage = proportionString;
}

QPixmap ShowImageWidget::localAmplification(const QPixmap &orgPix, QSize showSize, QPoint local, QSize widSize)
{
    // orgPix——原图   showSize——目标大小   local——基于showSize的起始坐标   widSize——窗口尺寸
    double proportion = double(showSize.width()) / double(orgPix.width());
    QPoint locaPoint(local.rx() / proportion, local.ry() / proportion);
    QSize locaSize(widSize.width() / proportion, widSize.height() / proportion);
    QPixmap localPix = orgPix.copy(locaPoint.rx(), locaPoint.ry(), locaSize.width(), locaSize.height());

    return localPix.scaled(widSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ShowImageWidget::showImageAfterClickedThumbnail(QString loadPath)
{
    m_imageStack.clear();
    proportionForPercentage = 1.0;

    qDebug() << "loadPath = " << loadPath;

    rotationFlag = false;
    imageNUll(true);
    showImageAfterClickedThumbnailFlag = true;
    double labWidth = this->width() - 2*ShowImageWidgetSpacing - AddWidthForLargeFontSize;
    double labHeight = this->height() - 24 -24 - 36 -16;
    QSize labSize(labWidth,labHeight);

    emit loadImgSignal(loadPath, NULL, m_normalImage, m_showImageLabel, 1, false, labSize);

    g_sane_object->normalImagePath = loadPath;

    if (g_sane_object->ocrFlag != 0) {
        g_user_signal->toolbarOcrOperationStart();
    }
}

void ShowImageWidget::loadImageFinishedSlot(QString save_path, double proportion, QImage image){
    if(showImageAfterClickedThumbnailFlag){
        showImageAfterClickedThumbnailFlag = false;
    }else{
        if(save_path != NULL){
            QString savePath = save_path;
            qDebug() << "savePath = " << savePath;
            g_sane_object->normalImagePath = savePath;
            m_saveFileBase.saveFileOP(savePath, &image);
        }else{
            QString savePath = m_savePathDelayUpdate;
            qDebug() << "savePath = " << savePath;
            g_sane_object->normalImagePath = savePath;
            m_saveFileBase.saveFileOP(savePath, m_normalImage);
        }
    }
    proportionForPercentage = proportion;
    int proportionInt = qRound(proportionForPercentage * 100);

    qDebug() << "proportionInt: " << proportionInt;
    QString proportionString = QString("%1").arg(proportionInt) + "%";

    qDebug() << "proportionString: " << proportionString;
    g_sane_object->percentage = proportionString;

    g_user_signal->toolbarPercentageChanged();

    defaultScanImageSize = m_normalImage->size();
    imageNUll(false);
}
void ShowImageWidget::saveImage(QString filename)
{
    qDebug() << "另存为" << filename;
    QImage tmp = m_normalImage->copy();
    QFileInfo a(filename);
    if(a.suffix() != "pdf"  && a.suffix() != "ofd"){
        if(a.suffix() != "txt"){
            tmp.save(filename);
        }else{
            QFile file(filename);
            file.open(QIODevice::ReadWrite | QIODevice::Text);
            QByteArray str = g_sane_object->ocrOutputText.toUtf8();
            file.write(str);
            file.close();
        }
    }else if(a.suffix() == "ofd"){
        m_saveFileBase.saveFileOP(filename, &tmp);
    }else{
        //        m_saveFileBase.saveFileOP(filename, &tmp);
        saveToPdf(tmp, filename);
    }
}

void ShowImageWidget::cropSlot()
{
    qDebug() << "Crop begin";
    if (g_sane_object->cropFlag == 0) {
        g_sane_object->cropFlag = 1;
        m_toolbarWidget->cropOtherIconState(true);
        m_cropLabel->initCropSettings(m_showImageLabel->width(), m_showImageLabel->height());
        m_showImageAndCropWidget->setCurrentWidget(m_cropLabel);
        *m_editImage = m_normalImage->copy();
        *m_stackImage = m_normalImage->copy();
        m_imageStack.push(*m_stackImage);
        g_user_signal->exitWindowWithSaveFlag = true;
        setCropPixmapScaled(*m_editImage, m_cropLabel);
    }else{
        m_toolbarWidget->cropOtherIconState(false);
        rollBackOperation();
        m_cancelOkWidget->hide();
    }
}

void ShowImageWidget::rotateSlot()
{
    qDebug() << "Rotate begin";
    *m_stackImage = m_normalImage->copy();
    m_imageStack.push(*m_stackImage);
    QMatrix matrix;
    matrix.rotate(270);
    *m_normalImage = m_normalImage->transformed(matrix);
    rotationFlag = !rotationFlag;
    setPixmapScaled(*m_normalImage, m_showImageLabel);
}
void ShowImageWidget::rotateFinished(){
    m_normalImage->load(ScanningPicture);
    rotationFlag = !rotationFlag;
    setPixmapScaled(*m_normalImage, m_showImageLabel);
}
void ShowImageWidget::mirrorSlot()
{
    *m_stackImage = m_normalImage->copy();
    m_imageStack.push(*m_stackImage);

    *m_editImage = m_normalImage->mirrored(true, false);

    *m_normalImage = m_editImage->copy();
    setPixmapScaled(*m_normalImage, m_showImageLabel);

}

void ShowImageWidget::watermarkSlot()
{
    qDebug() << "Watermark begin";

    WatermarkDialog *dialog = new WatermarkDialog();

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - dialog->width()/2;
        int y = rect.y() + rect.height()/2 - dialog->height()/2;
        dialog->move(x,y);
    }

    int ret = dialog->exec();
    qDebug() << "ret = " << ret;

    if (ret == QDialog::Accepted) {
        QString text = dialog->getLineEditText();
        qDebug() << text;

        *m_stackImage= m_normalImage->copy();
        m_imageStack.push(*m_stackImage);

        *m_editImage = m_normalImage->copy();

        QPixmap pix;
        pix = QPixmap::fromImage(*m_editImage);

        qDebug() << "pix size: " << pix.size();


        int spacing = 5;
        int fontSize = pix.width()/30;

        QFont font;
        font.setFamily("NotoSansCJKsc-Regular, NotoSansCJKsc;");
        font.setPointSize(fontSize);
        font.setWeight(400);
        font.setLetterSpacing(QFont::AbsoluteSpacing, spacing);

        QColor color;
        color.setRed(0);
        color.setGreen(0);
        color.setBlue(0);
        color.setAlphaF(0.2);

        QPainter painter(&pix);
        painter.setFont(font);
        painter.setPen(color);
        painter.rotate(15);

        int squareEdgeSize = m_editImage->width() * sin(45) + m_editImage->height() * sin( 45);
        int hCount = squareEdgeSize / ((fontSize + spacing) * (text.size() + 1)) + 1;
        int x = squareEdgeSize / hCount + (fontSize + spacing) * 3;
        int y = x / 2;

        qDebug() << "hcount = " << hCount
                 << "squareEdgeSize = " << squareEdgeSize
                 << "(x,y) = " << x << y;

        for (int i = 0; i < hCount; ++i) {
            for (int j = 0; j < hCount * 2; ++j) {
                painter.drawText(x * i, y * j, text);
//                qDebug() << "drawtext: " << text;
            }
        }

        *m_editImage = pix.toImage();


        setPixmapScaled(*m_editImage, m_showImageLabel);

        *m_normalImage = m_editImage->copy();
        setPixmapScaled(*m_normalImage, m_showImageLabel);
    }

    delete dialog;
}

void ShowImageWidget::zoomoutNormalImage()
{
    /*setPixmapScaled中的scale=1一方面是初始化数值，
     * 表示将图片恢复为默认尺寸，但是在zoom in/out时，
     * scale=1，表示的则是放到到相对尺寸的100%（100% ！= 默认尺寸）
     * 所以缩放时遇到刚好scale == 1的情况，则采取避开的措施（不会影响显示精度），防止歧义
    */
    double scale = getRealPercentage(ZoomOut);
    if (qFuzzyCompare(scale, 1.0)) {
        scale = 0.999;
    }

    setPixmapScaled(*m_normalImage, m_showImageLabel, scale);

    updatePercentageByZoom(scale);
}

void ShowImageWidget::zoominNormalImage()
{
    /*setPixmapScaled中的scale=1一方面是初始化数值，
     * 表示将图片恢复为默认尺寸，但是在zoom in/out时，
     * scale=1，表示的则是放到到相对尺寸的100%（100% ！= 默认尺寸）
     * 所以缩放时遇到刚好scale == 1的情况，则采取避开的措施（不会影响显示精度），防止歧义
    */
    double scale = getRealPercentage(ZoomIn);
    if (qFuzzyCompare(scale, 1.0)) {
        scale = 1.001;
    }

    setPixmapScaled(*m_normalImage, m_showImageLabel, scale);

    updatePercentageByZoom(scale);

}

void ShowImageWidget::zoomCurrentNormalImage()
{
    double scale = getCurrentPercentage();

    setCropPixmapScaled(*m_normalImage, m_showImageLabel, scale);
    setCropPixmapScaled(*m_normalImage, m_cropLabel, scale);

    updatePercentageByZoom(scale);
}
void ShowImageWidget::showBeautyRunningDialog(QString text)
{
    m_beautyRunningDialog = new RunningDialog(this, text);

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - m_beautyRunningDialog->width()/2;
        int y = rect.y() + rect.height()/2 - m_beautyRunningDialog->height()/2;
        m_beautyRunningDialog->move(x,y);
    }

    m_beautyRunningDialog->disconnectCancelButton();
    m_beautyRunningDialog->m_closeButtondisconnect();

    m_beautyRunningDialog->show();

    connect(m_beautyRunningDialog->btnCancel, &QPushButton::clicked, this, &ShowImageWidget::cancelImageOperationSlot);
    connect(m_beautyRunningDialog->m_closeButton, &QPushButton::clicked, this,[=](){
        m_beautyRunningDialog->btnCancel->click();
    });
    connect(m_beautyRunningDialog->btnCancel, &QPushButton::clicked, this, [=](){
        m_beautyRunningDialog->hideCancelButton();
        m_beautyRunningDialog->setWaitText(tr("Canceling...Please waiting..."));
    });
    connect(g_user_signal, &GlobalUserSignal::exitOCR, this, &ShowImageWidget::ocrStopSlot);
}
void ShowImageWidget::cancelImageOperationSlot(){
    if(imageOpThread->isRunning()){
        imageOp->interuptOp();
    }
}
void ShowImageWidget::beautyStartSlot()
{
    showBeautyRunningDialog(tr("Running beauty ..."));

    imageOp = new ImageOperationBeauty(m_normalImage, &m_imageStack);
    imageOpThread = new QThread;
    imageOp->moveToThread(imageOpThread);
    imageOpThread->start();
    connect(this,&ShowImageWidget::beautySignal,imageOp,&ImageOperationBase::ImageOP);
    connect(imageOp,&ImageOperationBase::finished,this,&ShowImageWidget::beautyFinished);
    emit beautySignal();
}
void ShowImageWidget::beautyFinished(){
    delete m_beautyRunningDialog;
    m_beautyRunningDialog = nullptr;
    if(imageOp->getIsInterrupt()){
        qDebug() << "interupt operation! recover last step image";
        QKeyEvent keyPress(QEvent::KeyPress,Qt::Key_Z,Qt::ControlModifier,QString(""));
        QApplication::sendEvent(this, &keyPress);
    }else{
        loadAfterBROPicture();
    }
    delete imageOp;
    imageOp = nullptr;
    imageOpThread->quit();
    qDebug() << "image operation complete!";
}
void ShowImageWidget::beautyStopSlot()
{
    loadAfterBROPicture();
}

void ShowImageWidget::rectifyStartSlot()
{
    qDebug() << "[1]get rectify signal!";
    showBeautyRunningDialog(tr("Running rectify ..."));
    imageOp = new ImageOperationRectify(m_normalImage, &m_imageStack);
    imageOpThread = new QThread;
    imageOp->moveToThread(imageOpThread);
    imageOpThread->start();
    connect(this,&ShowImageWidget::rectifySignal,imageOp,&ImageOperationBase::ImageOP);
    connect(imageOp,&ImageOperationBase::finished,this,&ShowImageWidget::beautyFinished);
    emit rectifySignal();
}

void ShowImageWidget::rectifyStopSlot(bool isTrue)
{
    qDebug() << "rectify: " << isTrue;

    if (isTrue) {
        loadAfterBROPicture();
    }
}

void ShowImageWidget::ocrStartSlot()
{
    QString realLoadPathDelayUpdate = QFileInfo(m_loadPathDelayUpdate).absolutePath() + "/" +  QFileInfo(m_loadPathDelayUpdate).baseName() + "_temp" + ".jpg";
    m_normalImage->save(realLoadPathDelayUpdate);
    emit g_user_signal->showScanWidgetSignal(realLoadPathDelayUpdate);
    imageOp = new ImageOperationOCR(m_normalImage);
    imageOpThread = new QThread();
    imageOp->moveToThread(imageOpThread);
    imageOpThread->start();
    connect(this,&ShowImageWidget::OCRSignal,imageOp,&ImageOperationBase::ImageOP);
    connect(imageOp,&ImageOperationBase::finished,this,&ShowImageWidget::ocrStopSlot);
    emit OCRSignal();
}

void ShowImageWidget::ocrStopSlot()
{
//    disconnect(imageOpOcr,&ImageOperationBase::finished,this,&ShowImageWidget::ocrStopSlot);
//    imageOpOcrThread->terminate();
    imageOpOcr = nullptr;
    imageOpOcrThread = nullptr;
}

void ShowImageWidget::saveCurrentPicture()
{
    m_normalImage->save(ScanningPicture);
}

void ShowImageWidget::loadAfterBROPicture()
{
    m_normalImage->load(ScanningPicture);
    setPixmapScaled(*m_normalImage, m_showImageLabel);
}

void ShowImageWidget::cropCancelSlot()
{
    qDebug() << "pressed cancel: " ;
    rollBackOperation();
    m_cancelOkWidget->hide();
    m_toolbarWidget->cropOtherIconState(false);
}

void ShowImageWidget::cropCompleteSlot()
{
    qDebug() << "pressed OK: ";
    m_toolbarWidget->cropOtherIconState(false);

    int x1, y1, x2, y2;
    getCropArea(x1, y1, x2, y2);

    QImage cropImage = m_normalImage->copy(x1/proportion, y1/proportion, (x2-x1)/proportion, (y2-y1)/proportion);

    *m_editImage = cropImage;
    setPixmapScaled(*m_editImage, m_cropLabel, proportion);

    *m_normalImage = m_editImage->copy();
    setPixmapScaled(*m_normalImage, m_showImageLabel, proportion);

    m_cropLabel->initCropSettings();


    m_showImageAndCropWidget->setCurrentWidget(m_labelWidget);

    g_sane_object->cropFlag = 0;

    m_cancelOkWidget->hide();

}

void ShowImageWidget::naviChange()
{
    if (m_navigator->isHidden()) {
        return;
    }
    m_navigator->move(this->width() - 7 - m_navigator->width(), this->height() - 52 - m_navigator->height());
}

void ShowImageWidget::resetNavigator()
{
    m_navigator->hide();
    m_clickBeforePosition = QPoint(0, 0);
}
