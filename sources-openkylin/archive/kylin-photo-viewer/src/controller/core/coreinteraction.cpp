/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "coreinteraction.h"

Interaction *Interaction::m_interaction(nullptr);

Interaction *Interaction::getInstance()
{
    if (m_interaction == nullptr) {
        m_interaction = new CoreInteraction;
    }
    return m_interaction;
}


CoreInteraction::CoreInteraction()
{
    m_canProcess = new QTimer(this);
    m_canProcess->setSingleShot(true);
}

CoreInteraction::~CoreInteraction()
{
    if(m_core)
        m_core->deleteLater();
    if(m_thread)
        m_thread->quit();
}

void CoreInteraction::creatCore(const QStringList &list)
{
    if (m_isCoreInit) {
        return; //防止多次初始化核心
    }
    //开辟线程
    m_core = new Core();
    m_needStartWithOpenImagePath = m_core->initDbus(list);
    m_thread = new QThread();
    //绑定信号和槽
    initConnect();
    //放到新线程中
    m_core->moveToThread(m_thread);
    m_thread->start();
    if (m_needStartWithOpenImagePath != nullptr && m_needStartWithOpenImagePath != Variable::API_TYPE) {
        m_core->initMainImageLoad(m_needStartWithOpenImagePath);
    }
    m_isCoreInit = true; //防止多次初始化核心
}

void CoreInteraction::initConnect()
{
    connect(this, &CoreInteraction::coreOpenImage, m_core, &Core::findAllImageFromDir); //生成缩略图列表
    connect(this, &CoreInteraction::coreGetAlbumModel, m_core, &Core::getAlbumModel,
            Qt::BlockingQueuedConnection);                                  //获取相册model指针
    connect(m_core, &Core::openFinish, this, &CoreInteraction::openFinish); //图片打开完成，将数据返回给UI层
    connect(this, &CoreInteraction::coreChangeImage, m_core, &Core::changeImage);           //切换图片
    connect(this, &CoreInteraction::coreChangeWidgetSize, m_core, &Core::changeWidgetSize); //改变窗口大小
    connect(this, &CoreInteraction::coreGetDefaultProportion, m_core, &Core::getDefaultProportion);
    connect(m_core, &Core::updateDefaultProportion, this, &CoreInteraction::updateDefaultProportion);
    connect(this, &CoreInteraction::coreChangeImageShowSize, m_core,
            &Core::changeImageShowSize); //图片显示状态（放大缩小）
    connect(this, &CoreInteraction::coreFlip, m_core, &Core::flipImage);                  //翻转
    connect(this, &CoreInteraction::coreDeleteImage, m_core, &Core::deleteImage);         //删除图片
    connect(this, &CoreInteraction::coreSetAsBackground, m_core, &Core::setAsBackground); //设置为背景图
    #ifndef UNUI
    connect(this, &CoreInteraction::coreOpenInfile, m_core, &Core::openInfile);           //文件夹中打开
    #endif
    connect(this, &CoreInteraction::coreChangeOpenIcon, m_core, &Core::changeOpenIcon); //更改主题，切换相册打开按钮图标
    connect(this, &CoreInteraction::coreClose, m_core, &Core::close);                   //关闭事件
    connect(m_core, &Core::coreProgremExit, this, &CoreInteraction::progremExit); //发送信号让主界面结束进程
    #ifndef UNUI
    connect(this, &CoreInteraction::changeImageFromClick, m_core, &Core::changeImageFromClick); //点击相册事件
    #endif
    connect(m_core, &Core::changeAlbumHighLight, this,
            &CoreInteraction::changeAlbumHighLight);                      //在相册中选中到所切换的图片
    connect(m_core, &Core::delayShow, this, &CoreInteraction::delayShow); //图片加载过慢，发信号给前端
    connect(m_core, &Core::openFromAlbum, this, &CoreInteraction::openFromAlbum);    //点击相册按钮打开图片
    connect(this, &CoreInteraction::coreRename, m_core, &Core::toCoreChangeName);    //重命名
    connect(m_core, &Core::renameResult, this, &CoreInteraction::sendRenameResult);  //返回重命名结果
    connect(this, &CoreInteraction::corePrint, m_core, &Core::toPrintImage);         //打印
    connect(m_core, &Core::canDelImage, this, &CoreInteraction::changeDelState);     //检查是否可被删除
    connect(m_core, &Core::updateSideNum, this, &CoreInteraction::updateSideLength); //更新相册长度
    connect(m_core, &Core::fullScreen, this, &CoreInteraction::fullScreen);
    #ifndef UNUI
    connect(this, &CoreInteraction::coreCut, m_core, &Core::cutImage); //开始裁剪，处理需要显示的图片
    #endif
    connect(this, &CoreInteraction::coreCutRegion, m_core, &Core::cutRegion);     //裁剪区域
    connect(this, &CoreInteraction::coreStartChange, m_core, &Core::startChange); //起始点-裁剪
    connect(this, &CoreInteraction::coreRealsePos, m_core, &Core::realsePos);     //释放时-重绘裁剪框边界
    connect(m_core, &Core::releaseRegion, this, &CoreInteraction::releaseRegion); //释放时-裁剪框的起点和终点
    connect(this, &CoreInteraction::coreReleaseCutChange, m_core,
            &Core::releaseCutChange); //释放后-裁剪框需要再次改变尺寸的情况
    connect(this, &CoreInteraction::coreExitCut, m_core, &Core::exitCut);                   //退出裁剪
    connect(this, &CoreInteraction::coreNeedSaveCutImage, m_core, &Core::needSaveCutImage); //保存裁剪图片
    connect(m_core, &Core::saveCutImageFaild, this, &CoreInteraction::saveCutImageFaild);
    connect(m_core, &Core::cutSaveFinish, this, &CoreInteraction::cutSaveFinish);           //裁剪结束
    connect(this, &CoreInteraction::coreNeedSaveas, m_core, &Core::needSaveAsImage);        //图片另存为
    connect(m_core, &Core::saveAsFaild, this, &CoreInteraction::isSaveAsFaild);             //检查另存为失败
    connect(m_core, &Core::returnMouseType, this, &CoreInteraction::returnMouseShape);      //回传鼠标形状
    connect(m_core, &Core::isCutNotSave, this, &CoreInteraction::isCutNotSave);   //正在裁剪还未保存就要关闭
    connect(this, &CoreInteraction::coreReloadImage, m_core, &Core::reloadImage); //将当前页的item传给后端
    connect(this, &CoreInteraction::coreGetText, m_core, &Core::getTextFromImage);        //获取图片中文字
    connect(m_core, &Core::getTextResult, this, &CoreInteraction::getTextFinish);         //图片中文字识别结果
    connect(m_core, &Core::exitOcrModeSign, this, &CoreInteraction::exitOcrModeSign);    //退出ocr模式

    // qml-------------------------------------
    connect(this, &CoreInteraction::coreQmlStartCutDisplay, m_core, &Core::qmlCutImageDisplay);
    connect(this, &CoreInteraction::coreQmlTiffImagePage, m_core, &Core::qmlTiffImagePage);
    //裁剪-显示区域全图
    connect(this, &CoreInteraction::coreCutFullViableImage, m_core, &Core::cutFullViableImage);
    //标注
    connect(this, &CoreInteraction::coreMarkImage, m_core, &Core::markImage); //按下标注
    connect(this, &CoreInteraction::coreStartMark, m_core, &Core::startMark); //进入标注
    connect(this, &CoreInteraction::coreMarkClear, m_core, &Core::markClear); //清除
    connect(this, &CoreInteraction::coreMarkSave, m_core, &Core::markSave);   //保存
    connect(this, &CoreInteraction::coreMarkUndo, m_core, &Core::markUndo);   //撤销

    connect(m_core, &Core::showLoadingImageWid, this, &CoreInteraction::showLoadingImageWid);

    // 调整图像尺寸
    connect(this, &CoreInteraction::coreResizeImage, m_core, &Core::resizeImage);

#ifdef USE_ONNXRUNTIME
    //背景替换
    connect(this, &CoreInteraction::corePicMatting, m_core, &Core::picMatting);
    connect(this, &CoreInteraction::coreReplaceBackground, m_core, &Core::replaceBackground);
    connect(m_core, &Core::returnReplaceAndOrigImage, this, &CoreInteraction::returnReplaceAndOrigImage);
    connect(this, &CoreInteraction::coreNeedSaveReplaceImage, m_core, &Core::needSaveReplaceImage);
    connect(this, &CoreInteraction::coreMattingFlip, m_core, &Core::mattingFlip);
    connect(m_core, &Core::returnFlipAndOrigImage, this, &CoreInteraction::returnFlipAndOrigImage);
    connect(this, &CoreInteraction::coreSaveCutImage, m_core, &Core::saveCutImage);
#endif

    //扫描黑白件
#ifdef SUPPORTSCANNER
    connect(this, &CoreInteraction::coreScanImage, m_core, &Core::scanImage);
    connect(m_core, &Core::returnScanAndOrigImage, this, &CoreInteraction::returnScanAndOrigImage);
    connect(m_core, &Core::returnScannerFlipAndOrigImage, this, &CoreInteraction::returnScannerFlipAndOrigImage);
    connect(m_core, &Core::returnScannerSaveResult, this, &CoreInteraction::returnScannerSaveResult);
    connect(this, &CoreInteraction::coreNeedSaveScanImage, m_core, &Core::needSaveScanImage);
    connect(this, &CoreInteraction::corePrintScanImage, m_core, &Core::printScanImage);
    connect(this, &CoreInteraction::coreScannerSavePDF, m_core, &Core::scannerSavePDF);
    connect(this, &CoreInteraction::coreScannerAddWatermark, m_core, &Core::scannerAddWatermark);
    connect(this, &CoreInteraction::coreScannerFlip, m_core, &Core::scannerFlip);
    connect(this, &CoreInteraction::coreScannerSaveCutImage, m_core, &Core::scannerSaveCutImage);
#endif
}

bool CoreInteraction::coreOperateTooOften()
{
    if (m_canProcess->isActive()) {
        return true;
    }
    m_canProcess->start(Variable::REFRESH_RATE); //刷新间隔
    return false;
}

void CoreInteraction::initUiFinish()
{
    Q_EMIT startWithOpenImage(m_needStartWithOpenImagePath);
}

void CoreInteraction::openImage(const QString &path)
{
    Q_EMIT coreOpenImage(path);
}

#ifndef UNUI
void CoreInteraction::changeImage(const int &type)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreChangeImage(type);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"ChangeImage", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}
#endif

void CoreInteraction::nextImage()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif

    //    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    Q_EMIT coreChangeImage(Enums::NEXT_IMAGE);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"ChangeImage", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::backImage()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreChangeImage(Enums::BACK_IMAGE);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"ChangeImage", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::changeWidgetSize(const QSize &size)
{
    Q_EMIT coreChangeWidgetSize(size);
}

void CoreInteraction::getDefaultProportion(const QSize &size)
{
    Q_EMIT coreGetDefaultProportion(size);
}

#ifndef UNUI
void CoreInteraction::watchBigImage(QPoint focus)

{
    Q_EMIT coreChangeImageShowSize(ImageShowStatus::BIG, focus);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"ZoomImage", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::watchSmallImage(QPoint focus)
{

    Q_EMIT coreChangeImageShowSize(ImageShowStatus::SMALL, focus);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"ZoomImage", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}
#endif

void CoreInteraction::watchOriginalImage()
{
    Q_EMIT coreChangeImageShowSize(ImageShowStatus::ORIGIN, QPoint());
}

void CoreInteraction::watchAutoImage()
{
    Q_EMIT coreChangeImageShowSize(ImageShowStatus::AUTO, QPoint());
}

void CoreInteraction::rotate(const bool &clockwise)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    if (clockwise) {
        Q_EMIT coreFlip(Processing::clockwise);
    } else {
        Q_EMIT coreFlip(Processing::counterclockwise);
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Rotate", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::flipH()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreFlip(Processing::horizontal);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Flip", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::flipV()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreFlip(Processing::vertical);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Flip", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}
#ifndef UNUI
void CoreInteraction::picMatting(QImage pic)
{
    Q_EMIT corePicMatting(pic);
}

void CoreInteraction::replaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter)
{
    Q_EMIT coreReplaceBackground(b, g, r, a, isFirstEnter);
}

void CoreInteraction::needSaveReplaceImage(QString path)
{
    Q_EMIT coreNeedSaveReplaceImage(path);
}

void CoreInteraction::mattingRotate(const bool &clockwise)
{
    if (clockwise) {
        Q_EMIT coreMattingFlip(Processing::clockwise);
    } else {
        Q_EMIT coreMattingFlip(Processing::counterclockwise);
    }
}
void CoreInteraction::mattingFlipH()
{
    Q_EMIT coreMattingFlip(Processing::horizontal);
}
void CoreInteraction::mattingFlipV()
{
    Q_EMIT coreMattingFlip(Processing::vertical);
}

void CoreInteraction::scanImage()
{
    Q_EMIT coreScanImage();
}

void CoreInteraction::needSaveScanImage(QString savePath)
{
    Q_EMIT coreNeedSaveScanImage(savePath);
}

void CoreInteraction::printScanImage(QPrinter *printer)
{
    Q_EMIT corePrintScanImage(printer);
}

void CoreInteraction::scannerSavePDF(QPrinter *printer)
{
    Q_EMIT coreScannerSavePDF(printer);
}

void CoreInteraction::scannerAddWatermark(bool isWatermark, QString vmContent)
{
    Q_EMIT coreScannerAddWatermark(isWatermark, vmContent);
}

void CoreInteraction::scannerRotate(const bool &clockwise)
{
    if (clockwise) {
        Q_EMIT coreScannerFlip(Processing::clockwise);
    } else {
        Q_EMIT coreScannerFlip(Processing::counterclockwise);
    }
}

void CoreInteraction::scannerFlipH()
{
    Q_EMIT coreScannerFlip(Processing::horizontal);
}

void CoreInteraction::scannerFlipV()
{
    Q_EMIT coreScannerFlip(Processing::vertical);
}

void CoreInteraction::scannerSaveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif

    Q_EMIT coreScannerSaveCutImage(proportion, beginPoint, endPoint);
}
#endif
void CoreInteraction::resizeImage(int w, int h)
{
    Q_EMIT coreResizeImage(w, h);
}

void CoreInteraction::deleteImage()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreDeleteImage();
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Delete", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::setAsBackground()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreSetAsBackground();
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"SetAsBackground", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}
#ifndef UNUI
void CoreInteraction::openImageInfile()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreOpenInfile();
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"OpenImageInfile", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}
#endif
void CoreInteraction::changeOpenIcon(QString theme)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreChangeOpenIcon(theme);
}

void CoreInteraction::close()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreClose();
}

QStandardItemModel *CoreInteraction::getAlbumModel()
{
    return coreGetAlbumModel();
}

void CoreInteraction::reName(QString oldPath, QString newPath)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreRename(oldPath, newPath);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Rename", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}
#ifndef UNUI
void CoreInteraction::printImage(QPrinter *printer)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT corePrint(printer);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"PringImage", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::cutImage()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreCut();
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Cutting", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}
#endif
void CoreInteraction::getText()
{
    Q_EMIT coreGetText();
}

void CoreInteraction::cutRegion(const QPoint &point)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreCutRegion(point);
}

void CoreInteraction::startChange(const QPoint &point)
{
    Q_EMIT coreStartChange(point);
}

void CoreInteraction::releasePos(const QPoint &point)
{
    Q_EMIT coreRealsePos(point);
}

void CoreInteraction::releaseCutChange(int type, const QPoint &point)
{
    Q_EMIT coreReleaseCutChange(type, point);
}

void CoreInteraction::exitCut()
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreExitCut();
}

void CoreInteraction::needSaveCutImage(int proportion)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreNeedSaveCutImage(proportion);
}
#ifndef UNUI
void CoreInteraction::saveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreSaveCutImage(proportion, beginPoint, endPoint);
}
#endif
void CoreInteraction::needSaveAs(QString path)
{
#ifndef UNUI
    if (coreOperateTooOften()) {
        return;
    }
#endif
    Q_EMIT coreNeedSaveas(path);
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"SaveAs", (char *)"coreinteraction");
    kdk_dia_data_free(node);
}

void CoreInteraction::reloadImage(QModelIndex modelIndex)
{
    Q_EMIT coreReloadImage(modelIndex);
}

void CoreInteraction::clickCutFullViableImage()
{
    Q_EMIT coreCutFullViableImage();
}

void CoreInteraction::startMark()
{
    Q_EMIT coreStartMark();
}

void CoreInteraction::toMarkImage(QVariant var)
{
    Q_EMIT coreMarkImage(var);
}

void CoreInteraction::markUndo()
{
    Q_EMIT coreMarkUndo();
}

void CoreInteraction::markSave(QString savePath)
{
    Q_EMIT coreMarkSave(savePath);
}

void CoreInteraction::markClear()
{
    Q_EMIT coreMarkClear();
}

void CoreInteraction::qmlStartCutDisplay(QPointF startPos, QSizeF displaySize, double imageScale)
{
    Q_EMIT coreQmlStartCutDisplay(startPos, displaySize, imageScale);
}

void CoreInteraction::qmlTiffImagePage(int pageNum)
{
    Q_EMIT coreQmlTiffImagePage(pageNum);
}
