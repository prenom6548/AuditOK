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
#ifndef SHOWIMAGEWIDGET_H
#define SHOWIMAGEWIDGET_H

#include <QWidget>
#include <QWidgetList>
#include <QStackedWidget>
#include <QLabel>
#include <QPainter>
#include <QImage>
#include <QMenu>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QImage>
#include <QFile>
#include <QPdfWriter>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QStack>
#include <QMatrix>
#include <QThread>
#include "imageBaseOP/loadimage.h"
#include <QMovie>
#include <QStackedLayout>

#include "globalsignal.h"
#include "saneobject.h"
#include "toolbarwidget.h"
#include "watermarkdialog.h"
#include "crop.h"
#include "rectify.h"
#include "include/common.h"
#include "imageOp/imageoperationbase.h"
#include "imageBaseOP/savefilebase.h"
#include <kborderlessbutton.h>
#include "navigator.h"

#define ShowImageWidgetMinimumSize QSize(387, 536)
#define CancelOkWidgetFixedSize QSize(92, 36)


class ShowImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShowImageWidget(QWidget *parent = nullptr);
    ~ShowImageWidget();
    friend class LoadImage;

    void rollBackOperation();

    void setupGui();
    void initConnect();

    int toUnicode(QString str);
    void setPdfSize(QPdfWriter *pdfWriter, QString size);
    void saveToPdf(QImage img, QString pathname);

    QImage m_navigationImage;          //导航栏背景
    int m_spaceWidth = 0;              //导航栏窗口与缩略图左边缘距离
    int m_spaceHeight = 0;             //导航栏窗口与缩略图上边缘距离
    QSize m_hightlightSize;            //高亮区域大小;
    QSize m_tmpSize;                   //按比例缩放后的图片大小
    QPoint m_clickBeforePosition = QPoint(0, 0);      //记录上次点击区域，用于提升体验

    QImage *imageSave(QString filename);
    QString setPixmapScaled(QImage img, QLabel *lab, double scale = 1.0);
    QString setCropPixmapScaled(QImage img, QLabel *lab, double scale = 1.0);
    QImage pictureDeepen(const QImage &img, const QSize &hightlightSize, const QPoint &point); //图片加深
    int boundaryJudg(int max, int point);
    int minNumIsZero(const int &num1, const int &num2);

    void setPixmapScaledByProportion(double scaledNumber);

    enum ZoomStatus {
        ZoomOut,
        ZoomIn
    };

    void getCropArea(int &x1, int &y1, int &x2, int &y2);

    double getCurrentPercentage();
    double getRealPercentage(ZoomStatus zoomType);
    void updatePercentageByZoom(double scale);

    const QString scannerImagePath;
    const int defaultImageLableWidth = 387;
    const int defaultImageLableHeight = 536;

    QSize defaultScanImageSize;
    QImage *m_stackImage = nullptr;
    QImage *m_editImage = nullptr;
    QImage *m_normalImage = nullptr;
    QList<QImage*> m_normalImageList;
    QPixmap m_normalPixmap;
    QStack<QImage> m_imageStack;
    QPixmap m_nowImage;

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
public slots:
    void beautyFinished();
    void rotateFinished();
    void cancelImageOperationSlot();
    void loadImageFinishedSlot(QString, double, QImage);

    void showNormalImageAfterScan(QStringList, QStringList);
    void showImageAfterClickedThumbnail(QString loadPath);

    void saveImage(QString filename);

    void cropSlot();
    void rotateSlot();
    void mirrorSlot();
    void watermarkSlot();
    void zoomoutNormalImage();
    void zoominNormalImage();
    void zoomCurrentNormalImage();

    void beautyStartSlot();
    void beautyStopSlot();
    void rectifyStartSlot();
    void rectifyStopSlot(bool isTrue);
    void ocrStartSlot();
    void ocrStopSlot();

    void saveCurrentPicture();
    void loadAfterBROPicture();

    void cropCancelSlot();
    void cropCompleteSlot();

    void naviChange();      //改变导航栏位置
    void resetNavigator();
    void createNavigation();
    void drawShadow();
    void clickNavigation(const QPoint &point = QPoint(-1, -1));

private:
    QString m_loadPathDelayUpdate;
    QString m_savePathDelayUpdate;
    bool showImageAfterClickedThumbnailFlag = false;
    RunningDialog *m_beautyRunningDialog = nullptr;
    void showBeautyRunningDialog(QString text);
    void imageNUll(bool res);

    bool rotationFlag = false;
    double proportion = 1.0;
    double proportionForPercentage = 1.0;

    QLabel *m_showImageLabel = nullptr;
    QLabel *m_waitingImageLabel = nullptr;
    QStackedLayout *m_stackedLayout = nullptr;
    QWidget *m_labelWidget = nullptr;

    CropLabel *m_cropLabel = nullptr;
    QStackedWidget *m_showImageAndCropWidget = nullptr;
    QHBoxLayout *m_showImageHLayout = nullptr;

    QWidget *m_cancelOkWidget = nullptr;
    QHBoxLayout *m_cancelOkLayout = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_okButton = nullptr;

    ToolBarWidget *m_toolbarWidget = nullptr;

    QVBoxLayout *m_mainVLayout = nullptr;
    ImageOperationBase *imageOp = nullptr;
    QThread *imageOpThread = nullptr;

    ImageOperationBase *imageOpOcr = nullptr;
    QThread *imageOpOcrThread = nullptr;


    LoadImage *loadImg = nullptr;
    QThread loadImgOP;
    QMovie *m_loadingMovie = nullptr;
    SaveFileBase m_saveFileBase;

    Navigator *m_navigator = nullptr;

    void setCancelWidgetPositon();
    QPixmap localAmplification(const QPixmap &orgPix, QSize showSize, QPoint local, QSize widSize);

signals:
    void beautySignal();
    void rectifySignal();
    void OCRSignal();
    void rotateSignal();
    void mirrorSignal();
    void showNavigation(QPixmap pix);                  //操作导航器
    void sendHightlightPos(QPoint startPos, QPoint endPos); //给前端发送导航器高亮区域范围

    void loadImgSignal(QString loadPath, QString savePath, QImage *img, QLabel *container,double scale, bool rotationFlag,QSize labSize);

};

#endif // SHOWIMAGEWIDGET_H
