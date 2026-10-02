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
#include <QtTest/QTest>
#include <QStringList>
#include <QSignalSpy>
#include <QThread>
#include <memory>
#include <QBuffer>
#include <QPrintDialog>
#include <QWidget>
#include "coreinteraction.h"
#include "core.h"
#include "ocr.h"
#include "interaction.h"
#include "dbus.h"
#include "rotate-menu-plugin.h"
#include <QDebug>

class MyTest : public QObject
{
    Q_OBJECT
public:
    MyTest();
    ~MyTest();

private Q_SLOTS:
    static void initTestCase()
    {
        int argc = 0;
        char *argv[] = {};
        m_app = std::make_unique<QApplication>(argc, argv);
    }
    static void cleanupTestCase()
    {
        m_coreInteraction->close();
        m_app.reset();
    }

    //coreinteraction
    void tst_getInstance();
    void tst_creatCore();
    void tst_initUiFinish();
    void tst_openImage();
    void tst_coreOperateTooOften();
    void tst_nextImage();
    void tst_backImage();
    void tst_changeWidgetSize();
    void tst_getDefaultProportion();
    void tst_watchOriginalImage();
    void tst_watchAutoImage();
    void tst_rotate();
    void tst_flipH();
    void tst_flipV();
    void tst_resizeImage();
    void tst_deleteImage();
    void tst_setAsBackground();
    void tst_changeOpenIcon();
    void tst_getAlbumModel();
    void tst_reName();
    void tst_getText();
    //裁剪
    void tst_qmlStartCutDisplay();
    void tst_startChange();
    void tst_cutRegion();
    void tst_releasePos();
    void tst_releaseCutChange();
    void tst_needSaveCutImage();
    void tst_clickCutFullViableImage();
    void tst_exitCut();
    void tst_needSaveAs();
    void tst_reloadImage();
    void tst_startMark();
    void tst_toMarkImage();
    void tst_markUndo();
    void tst_markSave();
    void tst_markClear();
    void tst_qmlTiffImagePage();

    //core
    void tst_tiff();
    void tst_gif();
    void tst_changeImage();
    void tst_onsleep();
    void tst_initMainImageLoad();

    //plugin
    void tst_peony_rotate_menu_plugin();

    //dbus
    void tst_initDbus();
    void tst_processingApi();
    void tst_processingCommand();
    void tst_fullScreen();

    void tst_close();

private:
    static std::unique_ptr<QApplication> m_app;
    static CoreInteraction *m_coreInteraction;
    static Core *m_core;
    static Interaction *m_interaction;
    static Dbus *m_dbus;
    static Peony::RotateMenuPlugin *m_menu;
};

std::unique_ptr<QApplication> MyTest::m_app;
CoreInteraction *MyTest::m_coreInteraction = nullptr;
Core *MyTest::m_core = nullptr;
Interaction *MyTest::m_interaction = nullptr;
Dbus *MyTest::m_dbus = nullptr;
Peony::RotateMenuPlugin *MyTest::m_menu = nullptr;
MyTest::MyTest()
{
}

MyTest::~MyTest()
{
}

void MyTest::tst_getInstance()
{  
    if(m_coreInteraction == nullptr){
        m_coreInteraction = new CoreInteraction();
        m_interaction = m_coreInteraction->getInstance();
    }
    QVERIFY(m_coreInteraction!= nullptr);
    QVERIFY(m_interaction!= nullptr);
}

void MyTest::tst_creatCore()
{
    if(m_core == nullptr){
        QStringList list= {"kylin-photo-viewer"};
        m_coreInteraction->creatCore(list);
        m_core = m_coreInteraction->m_core;
    }

    QVERIFY(m_coreInteraction->m_isCoreInit);
    QVERIFY(m_core!= nullptr);
}

void MyTest::tst_initUiFinish()
{
    m_coreInteraction->m_needStartWithOpenImagePath = true;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::startWithOpenImage);
    m_coreInteraction->initUiFinish();
    QCOMPARE(spy.count(), 1);
}

void MyTest::tst_openImage()
{
    QString path;

    //图片为空
    QSignalSpy spy0(m_core, &Core::openFinish);
    path = "testimage/a.bmp";
    m_core->changeOpenIcon(Variable::g_themeStyle);
    m_core->openImage(path);
    QTRY_COMPARE(spy0.count(), 1);
    //图片不为空
    path = "testimage/1.bmp";
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreOpenImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_core->changeOpenIcon(Variable::g_themeStyle);
    m_coreInteraction->openImage(path);

    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
    QTRY_COMPARE(m_core->m_albumModel->rowCount(), 15);
    QTRY_VERIFY(m_core->m_albumThumbnailLists.length() > 0);
    QTRY_COMPARE(m_core->m_albumThumbnailLists.length(), 0);

}

void MyTest::tst_coreOperateTooOften()
{
    QVERIFY(!m_coreInteraction->coreOperateTooOften());
}

void MyTest::tst_nextImage()
{
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreChangeImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->nextImage();

    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(m_core->m_albumThumbnailLists.length(), 0);
}

void MyTest::tst_backImage()
{
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreChangeImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->backImage();

    QCOMPARE(spy1.count(), 1);
    QVERIFY2(spy2.wait(5000), " tst_backImage spy2 openFinish time out");
    QCOMPARE(spy2.count(), 1);
    QTRY_COMPARE(m_core->m_albumThumbnailLists.length(), 0);
}

void MyTest::tst_changeWidgetSize()
{
    QSize size(1080, 720);

    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreChangeWidgetSize);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->changeWidgetSize(size);

    QCOMPARE(spy.count(), 1);
    QVERIFY2(spy2.wait(5000), " tst_changeWidgetSize spy2 openFinish time out");
    QCOMPARE(spy2.count(), 1);
}

void MyTest::tst_getDefaultProportion()
{
    QSize size(1080, 720);

    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreGetDefaultProportion);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::updateDefaultProportion);
    m_coreInteraction->getDefaultProportion(size);

    QCOMPARE(spy1.count(), 1);
    QVERIFY2(spy2.wait(5000), " tst_getDefaultProportion spy2 updateDefaultProportion time out");
    QCOMPARE(spy2.count(), 1);
}

void MyTest::tst_watchOriginalImage()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreChangeImageShowSize);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->watchOriginalImage();

    QCOMPARE(spy.count(), 1);
    QVERIFY2(spy2.wait(5000), " tst_watchOriginalImage spy2 openFinish time out");
    QCOMPARE(spy2.count(), 1);
}

void MyTest::tst_watchAutoImage()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreChangeImageShowSize);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->watchAutoImage();

    QCOMPARE(spy.count(), 1);
    QVERIFY2(spy2.wait(5000), " tst_watchAutoImage spy2 openFinish time out");
    QCOMPARE(spy2.count(), 1);
}

void MyTest::tst_rotate()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreFlip);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::openFinish);
    m_coreInteraction->rotate(true);

    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

    QSignalSpy spy3(m_coreInteraction, &CoreInteraction::coreFlip);
    QSignalSpy spy4(m_coreInteraction, &CoreInteraction::openFinish);
    m_coreInteraction->rotate(false);

    QCOMPARE(spy3.count(), 1);
    QTRY_COMPARE(spy4.count(), 1);
}
void MyTest::tst_flipH()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreFlip);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::openFinish);
    m_coreInteraction->flipH();

    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
}
void MyTest::tst_flipV()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreFlip);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::openFinish);
    m_coreInteraction->flipV();

    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
}

void MyTest::tst_resizeImage()
{
    // 原大小 1920 1080
    int w = 1920;
    int h = 1080;

    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreResizeImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->resizeImage(w, h);
    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

     w = 0;
     h = 0;
     m_coreInteraction->resizeImage(w, h);
     QCOMPARE(spy.count(), 2);
}

void MyTest::tst_deleteImage()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreDeleteImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->deleteImage();

    QCOMPARE(spy.count(), 1);
    QVERIFY2(spy2.wait(5000), " tst_deleteImage spy2 openFinish time out");
    QCOMPARE(spy2.count(), 1);

    QTRY_COMPARE(m_core->m_albumModel->rowCount(), 16);
}
void MyTest::tst_setAsBackground()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreSetAsBackground);
    m_coreInteraction->setAsBackground();
    m_core->setAsBackground();
    QCOMPARE(spy.count(), 1);
}

void MyTest::tst_changeOpenIcon()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreChangeOpenIcon);
    m_coreInteraction->changeOpenIcon("ukui-dark");
    m_core->changeOpenIcon("ukui-dark");
    QCOMPARE(spy.count(), 1);
    m_coreInteraction->changeOpenIcon("ukui-light");
    m_core->changeOpenIcon("ukui-light");
    QCOMPARE(spy.count(), 2);
}

void MyTest::tst_getAlbumModel()
{
    QStandardItemModel * albumModel;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreGetAlbumModel);
    albumModel = m_coreInteraction->getAlbumModel();
    QCOMPARE(spy.count(), 1);
}

void MyTest::tst_reName()
{
    //原文件不存在
    QString oldPath = "testimage/renameaaaa.bmp";
    QString newPath = "renameaaaa11";
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreRename);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::sendRenameResult);
    m_coreInteraction->reName(oldPath,newPath);
    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

    oldPath = "testimage/rename.bmp";
    QFileInfo fileinfo(oldPath);
    oldPath = fileinfo.absoluteFilePath();

    QStringList newPaths;
    newPaths<<"rename";
    newPaths<<"rename1";

    for(auto newPath : newPaths){
        QSignalSpy spy11(m_coreInteraction, &CoreInteraction::coreRename);
        QSignalSpy spy22(m_coreInteraction, &CoreInteraction::sendRenameResult);
        m_coreInteraction->reName(oldPath,newPath);
        QCOMPARE(spy11.count(), 1);
        QTRY_COMPARE(spy22.count(), 1);
    }
}

void MyTest::tst_getText()
{
    //打开图片
    QString path = "testimage/ocr.png";
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreOpenImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    QSignalSpy spy3(m_coreInteraction, &CoreInteraction::coreGetText);
    QSignalSpy spy4(m_core, &Core::getTextResult);

    m_core->changeOpenIcon(Variable::g_themeStyle);
    m_coreInteraction->openImage(path);

    QCOMPARE(spy1.count(), 1);
    QVERIFY2(spy2.wait(5000), " tst_openImage spy2 openFinish time out");
    QCOMPARE(spy2.count(), 1);
    QTRY_COMPARE(m_core->m_albumModel->rowCount(), 16);
    QTRY_COMPARE(m_core->m_albumThumbnailLists.length(), 0);

    m_coreInteraction->getText();

    QCOMPARE(spy3.count(), 1);
    QTRY_COMPARE(spy4.count(), 1);
}

void MyTest::tst_qmlStartCutDisplay(){
    //打开图片
    QString path = "testimage/2.png";
    qDebug()<<"tst_qmlStartCutDisplay 执行kaishi";
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreOpenImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_core->changeOpenIcon(Variable::g_themeStyle);
    m_coreInteraction->openImage(path);

    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

    // 进入裁剪
    QPointF startPos =  QPointF(0,0);
    QSizeF displaySize =  QSizeF(1920, 1200);
    double imageScale =  0.5625;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreQmlStartCutDisplay);
//    QSignalSpy spy3(m_coreInteraction, &CoreInteraction::openFinish);
    m_coreInteraction->qmlStartCutDisplay(startPos, displaySize, imageScale);
    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(spy2.count(), 2);
}

void MyTest::tst_startChange()
{
    //鼠标点击
    QPoint point_start(527,327);

    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreStartChange);

    m_coreInteraction->startChange(point_start);

    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(m_core->m_cutBeforeStartPosition , point_start);
}

void MyTest::tst_cutRegion()
{
    //鼠标移动 1920.1200
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreCutRegion);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::openFinish);

    double proportion = m_core->m_proportion * 0.01;
    QPoint point(0, 0);
    m_coreInteraction->cutRegion(point);
    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

    QPoint point2(-5 * proportion, -5 * proportion);
    m_coreInteraction->cutRegion(point2);
    QCOMPARE(spy1.count(), 2);
    QTRY_COMPARE(spy2.count(), 2);

    QPoint point3(0, 1200 * proportion);
    m_coreInteraction->cutRegion(point3);
    QCOMPARE(spy1.count(), 3);
    QTRY_COMPARE(spy2.count(), 3);

    QPoint point4(-5 * proportion, 1205 * proportion);
    m_coreInteraction->cutRegion(point4);
    QCOMPARE(spy1.count(), 4);
    QTRY_COMPARE(spy2.count(), 4);

    QPoint point5(1920 * proportion, 0);
    m_coreInteraction->cutRegion(point5);
    QCOMPARE(spy1.count(), 5);
    QTRY_COMPARE(spy2.count(), 5);

    QPoint point6(1925 * proportion, -5 * proportion);
    m_coreInteraction->cutRegion(point6);
    QCOMPARE(spy1.count(), 6);
    QTRY_COMPARE(spy2.count(), 6);

    QPoint point7(1920 * proportion, 1200 * proportion);
    m_coreInteraction->cutRegion(point7);
    QCOMPARE(spy1.count(),7);
    QTRY_COMPARE(spy2.count(), 7);

    QPoint point8(1925 * proportion, 1205 * proportion);
    m_coreInteraction->cutRegion(point8);
    QCOMPARE(spy1.count(), 8);
    QTRY_COMPARE(spy2.count(), 8);

//    QPoint point9(827, 627);
//    m_coreInteraction->cutRegion(point9);
//    QCOMPARE(spy1.count(), 9);
//    QTRY_COMPARE(spy2.count(), 9);
}

void MyTest::tst_releasePos()
{
    //鼠标释放
    qDebug()<<"tst_releasePos 执行kaishi";
    double proportion = m_core->m_proportion;
    QPoint point(1925 * proportion, 1205 * proportion);
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreRealsePos);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::releaseRegion);
    m_coreInteraction->releasePos(point);
    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
    qDebug()<<"tst_releasePos 执行完成";
}

void MyTest::tst_releaseCutChange()
{
    //释放后改变
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreReleaseCutChange);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::openFinish);

    double proportion = m_core->m_proportion * 0.01;

    QPoint point(0, 0);
    m_coreInteraction->releaseCutChange(4, point);
    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

    QPoint point2(-5 * proportion, -5 * proportion);
    m_coreInteraction->releaseCutChange(4, point2);
    QCOMPARE(spy1.count(), 2);
    QTRY_COMPARE(spy2.count(), 2);

    QPoint point3(0, 1200 * proportion);
    m_coreInteraction->releaseCutChange(4, point3);
    QCOMPARE(spy1.count(), 3);
    QTRY_COMPARE(spy2.count(), 3);

    QPoint point4(-5 * proportion, 1205 * proportion);
    m_coreInteraction->releaseCutChange(4, point4);
    QCOMPARE(spy1.count(), 4);
    QTRY_COMPARE(spy2.count(), 4);

    QPoint point5(1920 * proportion, 0);
    m_coreInteraction->releaseCutChange(4, point5);
    QCOMPARE(spy1.count(), 5);
    QTRY_COMPARE(spy2.count(), 5);

    QPoint point6(1925 * proportion, -5 * proportion);
    m_coreInteraction->releaseCutChange(4, point6);
    QCOMPARE(spy1.count(), 6);
    QTRY_COMPARE(spy2.count(), 6);

    QPoint point7(1920 * proportion, 1200 * proportion);
    m_coreInteraction->releaseCutChange(4, point7);
    QCOMPARE(spy1.count(),7);
    QTRY_COMPARE(spy2.count(), 7);

    QPoint point8(1925 * proportion, 1205 * proportion);
    m_coreInteraction->releaseCutChange(4, point8);
    QCOMPARE(spy1.count(), 8);
    QTRY_COMPARE(spy2.count(), 8);

    qDebug()<<"tst_releaseCutChange 执行完成";

}



void MyTest::tst_needSaveCutImage()
{
    //保存裁剪图片
    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreNeedSaveCutImage);
    QSignalSpy spy2(m_coreInteraction, &CoreInteraction::cutSaveFinish);
    QSignalSpy spy3(m_core, &Core::openFinish);
    m_coreInteraction->needSaveCutImage(20);
    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
    QTRY_COMPARE(spy3.count(), 1);
//    QTRY_COMPARE(m_core->m_albumModel->rowCount(), 15);
//    QTRY_COMPARE(m_core->m_albumThumbnailLists.length(), 0);
//    QTRY_COMPARE(m_core->m_findImageThread, nullptr);
}


void MyTest::tst_clickCutFullViableImage()
{
    tst_qmlStartCutDisplay();
    tst_startChange();
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreCutFullViableImage);
    m_coreInteraction->clickCutFullViableImage();
    m_core->cutFullViableImage();
    QTRY_COMPARE(spy.count(), 1);
}

void MyTest::tst_exitCut()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreExitCut);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_coreInteraction->exitCut();
    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
}

void MyTest::tst_needSaveAs()
{
    //ras
    QString path = "testimage/7.ras";
    QString savepath = "testimage/save/7.ras";
    QSignalSpy spy11(m_coreInteraction, &CoreInteraction::coreNeedSaveas);
    QSignalSpy spy22(m_core, &Core::openFinish);
    m_core->openImage(path);
    QTRY_COMPARE(spy22.count(), 1);
    m_coreInteraction->needSaveAs(savepath);
    QCOMPARE(spy11.count(), 1);
    QTRY_COMPARE(spy22.count(), 2);

    //svg
    path = "testimage/5.svg";
    savepath = "testimage/save/5.svg";

    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreNeedSaveas);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_core->openImage(path);
    QTRY_COMPARE(spy2.count(), 1);
    m_coreInteraction->needSaveAs(savepath);
    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 2);

    //单张gif

    //png
    path = "testimage/2.png";
    savepath = "testimage/save/2.png";
    QSignalSpy spy111(m_coreInteraction, &CoreInteraction::coreNeedSaveas);
    QSignalSpy spy222(m_core, &Core::openFinish);
    m_core->openImage(path);
    QTRY_COMPARE(spy222.count(), 1);
    m_coreInteraction->needSaveAs(savepath);
    QCOMPARE(spy111.count(), 1);
    QTRY_COMPARE(spy222.count(), 1);
}

void MyTest::tst_reloadImage()
{
    int type = 7;
    MyStandardItem *item = dynamic_cast<MyStandardItem *>(m_core->m_albumModel->item(type));
    QModelIndex modelIndex = m_core->m_albumModel->indexFromItem(item);
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreReloadImage);
    m_coreInteraction->reloadImage(modelIndex);
    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(m_core->m_albumThumbnailLists.count(), 0);
}

void MyTest::tst_startMark()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreStartMark);
    m_coreInteraction->startMark();
    m_core->startMark();
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_toMarkImage()
{
    QVariant var;
    MarkPainterSet mark;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreMarkImage);
    m_coreInteraction->toMarkImage(var);
    for(int i = 0; i < 8; i++){
        mark.type = i;
        mark.recordStartPos = QPoint(5, 5);
        mark.recordEndPos = QPoint(8, 8);
        var.setValue(mark);
        m_core->markImage(var);
    }
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_markUndo()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreMarkUndo);
    m_coreInteraction->markUndo();
    m_core->markUndo();
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_markSave()
{
    QString path = "testimage/save/markSave.png";
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreMarkSave);
    m_coreInteraction->markSave(path);
    m_core->markSave(path);
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_markClear()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreMarkClear);
    m_coreInteraction->markClear();
    m_core->markClear();
    QCOMPARE(spy.count(), 1);
}
/*
void MyTest::tst_replaceBackground()
{
    uint b = 0; uint g = 0; uint r = 0; uint a = 0; bool isFirstEnter = true;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreReplaceBackground);
    m_coreInteraction->replaceBackground(b, g, r, a, isFirstEnter);
    QCOMPARE(spy.count(), 1);
}
//void MyTest::tst_picMatting()
//{
//    QImage pic("testimage/7.bmp");
//    QSignalSpy spy(m_coreInteraction, &CoreInteraction::corePicMatting);
//    m_coreInteraction->picMatting(pic);
//    QCOMPARE(spy.count(), 1);
//}
//void MyTest::tst_needSaveReplaceImage()
//{
//    QString path = "testimage/7.bmp";
//    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreNeedSaveReplaceImage);
//    m_coreInteraction->needSaveReplaceImage(path);
//    QCOMPARE(spy.count(), 1);
//}
void MyTest::tst_mattingRotate()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreMattingFlip);
    m_coreInteraction->mattingRotate(true);
    QCOMPARE(spy.count(), 1);
    m_coreInteraction->mattingRotate(false);
    QCOMPARE(spy.count(), 2);
}
void MyTest::tst_mattingFlipH()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreMattingFlip);
    m_coreInteraction->mattingFlipH();
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_mattingFlipV()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreMattingFlip);
    m_coreInteraction->mattingFlipV();
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_saveCutImage()
{
    int proportion = 50;
    QPoint beginPoint(10, 20);
    QPoint endPoint(30, 40);
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreSaveCutImage);
    m_coreInteraction->saveCutImage(proportion, beginPoint, endPoint);
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_scanImage()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreScanImage);
    m_coreInteraction->scanImage();
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_needSaveScanImage()
{
    QString path = "testimage/7.bmp";
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreNeedSaveScanImage);
    m_coreInteraction->needSaveScanImage(path);
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_printScanImage()
{
    QPrinter *printer;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::corePrintScanImage);
    m_coreInteraction->printScanImage(printer);
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_scannerSavePDF()
{
    QPrinter *printer;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreScannerSavePDF);
    m_coreInteraction->scannerSavePDF(printer);
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_scannerAddWatermark()
{
    bool isWatermark = true;
    QString vmContent = "test";
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreScannerAddWatermark);
    m_coreInteraction->scannerAddWatermark(isWatermark, vmContent);
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_scannerRotate()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreScannerFlip);
    m_coreInteraction->scannerRotate(true);
    //m_core->scannerFlip(Processing::clockwise);
    QCOMPARE(spy.count(), 1);
    m_coreInteraction->scannerRotate(false);
    //m_core->scannerFlip(Processing::counterclockwise);
    QCOMPARE(spy.count(), 2);
}
void MyTest::tst_scannerFlipH()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreScannerFlip);
    m_coreInteraction->scannerFlipH();
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_scannerFlipV()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreScannerFlip);
    m_coreInteraction->scannerFlipV();
    QCOMPARE(spy.count(), 1);
}
void MyTest::tst_scannerSaveCutImage()
{
    int proportion = 50;
    QPoint beginPoint(10, 20);
    QPoint endPoint(30, 40);
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreScannerSaveCutImage);
    m_coreInteraction->scannerSaveCutImage(proportion, beginPoint, endPoint);
    QCOMPARE(spy.count(), 1);
}*/


void MyTest::tst_qmlTiffImagePage()
{
    int pageNum = m_core->m_tiffPage;
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreQmlTiffImagePage);
    m_coreInteraction->qmlTiffImagePage(pageNum);
    m_core->qmlTiffImagePage(pageNum);
    QCOMPARE(spy.count(), 1);
}



void MyTest::tst_tiff()
{
    //打开
    QString path = "testimage/tiff/7.tiff";

    QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreOpenImage);
    QSignalSpy spy2(m_core, &Core::openFinish);
    m_core->changeOpenIcon(Variable::g_themeStyle);
    m_coreInteraction->openImage(path);

    QCOMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

    QTRY_COMPARE(m_core->m_albumThumbnailLists.length(), 0);

    //flip
    QSignalSpy spy3(m_core, &Core::openFinish);
    m_core->flipImage(Processing::clockwise);
    QTRY_COMPARE(spy3.count(), 1);
    m_core->flipImage(Processing::counterclockwise);
    QTRY_COMPARE(spy3.count(), 2);
    m_core->flipImage(Processing::horizontal);
    QTRY_COMPARE(spy3.count(), 3);
    m_core->flipImage(Processing::vertical);
    QTRY_COMPARE(spy3.count(), 4);

    //保存
    QSignalSpy spy4(m_core, &Core::openFinish);
    m_core->needSave();
    QTRY_COMPARE(spy4.count(), 1);
}

void MyTest::tst_gif()
{
    QStringList paths;
    paths<< "testimage/movie/9.gif";
    paths<< "testimage/movie/9-1.gif";

    for(auto path : paths){
        //打开
//        QString path = "testimage/movie/9.gif";
        KylinImageCodec::m_flagGifLoad = true;
        QSignalSpy spy1(m_coreInteraction, &CoreInteraction::coreOpenImage);
        QSignalSpy spy2(m_core, &Core::openFinish);
        QSignalSpy spy5(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::loadMovieFinish);
        m_core->changeOpenIcon(Variable::g_themeStyle);
        m_coreInteraction->openImage(path);
        QMetaObject::invokeMethod(m_core->m_playMovieTimer,"start",Qt::QueuedConnection, Q_ARG(int,m_core->m_fps));

        QCOMPARE(spy1.count(), 1);
        QTRY_COMPARE(spy2.count(), 1);

        QTRY_COMPARE(m_core->m_albumThumbnailLists.length(), 0);
        if(path == "testimage/movie/9.gif"){
            QTRY_COMPARE(spy5.count(), 4);
        }

        //flip
        m_core->flipImage(Processing::clockwise);
        QMetaObject::invokeMethod(m_core->m_playMovieTimer,"start",Qt::QueuedConnection, Q_ARG(int,m_core->m_fps));
        m_core->flipImage(Processing::counterclockwise);
        QMetaObject::invokeMethod(m_core->m_playMovieTimer,"start",Qt::QueuedConnection, Q_ARG(int,m_core->m_fps));
        if(path == "testimage/movie/9.gif"){
            m_core->flipImage(Processing::horizontal);
            QMetaObject::invokeMethod(m_core->m_playMovieTimer,"start",Qt::QueuedConnection, Q_ARG(int,m_core->m_fps));
            m_core->flipImage(Processing::vertical);
            QMetaObject::invokeMethod(m_core->m_playMovieTimer,"start",Qt::QueuedConnection, Q_ARG(int,m_core->m_fps));
        }

        //保存
        QSignalSpy spy6(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish);
        m_core->needSave();
        QTRY_COMPARE_WITH_TIMEOUT(spy6.count(), 1,20000);
    }
}

void MyTest::tst_changeImage()
{
    QSignalSpy spy(m_core, &Core::updateSideNum);
    m_core->changeImage(Enums::ChamgeImageType::ERROR_IMAGE);
    QCOMPARE(spy.count(), 1);

    m_core->changeImage(Enums::ChamgeImageType::NEXT_IMAGE);

    m_core->changeImage(Enums::ChamgeImageType::BACK_IMAGE);

    m_core->changeImage(1);
}

void MyTest::tst_onsleep()
{
    m_core->onSleep(false);
}

void MyTest::tst_initMainImageLoad()
{
    QString fullPath = "testimage/big/big.jpg";
    m_core->initMainImageLoad(fullPath);
    QTRY_COMPARE(m_core->m_bigImageCache.count(), 1);

    QSignalSpy spy0(m_core, &Core::openFinish);
    m_core->openImage(fullPath);
    QTRY_COMPARE(spy0.count(), 1);
}

void MyTest::tst_peony_rotate_menu_plugin()
{
    Peony::MenuPluginInterface::Type type = Peony::MenuPluginInterface::DesktopWindow;
    QString uri = "";
    QStringList selectionUris;
    QStringList selectionPath = { "testimage/tiff/7.tiff",
                                  "testimage/movie/9.gif",
                                  "testimage/1.bmp",
                                  "testimage/2.png",
                                  "testimage/3.jpg",
                                  "testimage/4.tiff",
                                };
    for(auto string :selectionPath ){
        QFileInfo info(string);
        QString filepath = info.absoluteFilePath();
        QUrl fileUrl = QUrl::fromLocalFile(filepath);
        if(fileUrl.isValid()){
            selectionUris<<fileUrl.toString();
        }
    }
    m_menu = new Peony::RotateMenuPlugin(this);
    KylinImageCodec::m_flagGifLoad = true;

    //向右旋转
    QList<QAction*> actions = m_menu->menuActions(type, uri, selectionUris);
    QWidget * testParentWidget = new QWidget();
    actions[0]->setParent(testParentWidget);

    QSignalSpy spy1(m_menu, &Peony::RotateMenuPlugin::sigImageRotate);
    QSignalSpy spy2(m_menu->rotateProcessor, &Rotation::allImageFinishSig);
    actions[0]->trigger();
    QTRY_COMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
    QTRY_COMPARE(m_menu->m_dialog, nullptr);

    //向左旋转
    QList<QAction*> actions2 = m_menu->menuActions(type, uri, selectionUris);
    actions2[1]->setParent(testParentWidget);

    QSignalSpy spy3(m_menu, &Peony::RotateMenuPlugin::sigImageRotate);
    QSignalSpy spy4(m_menu->rotateProcessor, &Rotation::allImageFinishSig);
    actions2[1]->trigger();

    QTRY_COMPARE(spy3.count(), 1);
    QTRY_COMPARE(spy4.count(), 1);
    QTRY_COMPARE(m_menu->m_dialog, nullptr);
}


void MyTest::tst_initDbus() //命令参数
{
    //1. help
    QStringList arguments = {"kylin-photo-viewer", "--help"};
    QString result = m_core->initDbus(arguments);
    QCOMPARE(result, "");

    //2. 文件地址存在  file:/// 支持的格式 参数两个
    QString path = "testimage/tiff/7.tiff";
    QFileInfo fileinfo(path);
    QUrl fileUrl = QUrl::fromLocalFile(fileinfo.absoluteFilePath());

    QStringList argumentsFile;
    argumentsFile<<"kylin-photo-viewer";
    argumentsFile<<fileUrl.toString();
    QString resultFile = m_core->initDbus(argumentsFile);
    qDebug()<<"[dbus] file:/// resultFile="<<resultFile;
    QCOMPARE(resultFile == "", false);

    //地址不存在
    path = "testimage/tiff/5.tiff";
    arguments.clear();
    arguments<<"kylin-photo-viewer";
    arguments<<path;
    resultFile = m_core->initDbus(arguments);
    qDebug()<<"[dbus] path resultFile="<<resultFile;
    QCOMPARE(resultFile == "", true);
    //不是支持的格式
    path = "testimage/tiff/tiff.txt";
    arguments.clear();
    arguments<<"kylin-photo-viewer";
    arguments<<path;
    resultFile = m_core->initDbus(arguments);
    qDebug()<<"[dbus] path resultFile="<<resultFile;
    QCOMPARE(resultFile == "", false);

    //path
    path = "testimage/2.png";
    arguments.clear();
    arguments<<"kylin-photo-viewer";
    arguments<<path;
    arguments<<"kylin-gallery";
    resultFile = m_core->initDbus(arguments);
    qDebug()<<"[dbus] path resultFile="<<resultFile;
    QCOMPARE(resultFile == "", false);
}

void MyTest::tst_processingApi()
{
    //3. api : cmd.length() < 5
//    QString path = "testimage/2.png";
    QStringList arguments;
    arguments.clear();
    arguments<<"kylin-photo-viewer";
    arguments<<"-api";
    QString resultApi = m_core->initDbus(arguments);
    qDebug()<<"[dbus] api resultApi="<<resultApi;
    QCOMPARE(resultApi == "", true);

    QStringList cmd;
    //flip cmd.length() < 5
    cmd.clear();
    cmd<<"kylin-photo-viewer";
    cmd<<"-api";
    cmd<<"-flip";
    resultApi = m_core->processingApi(cmd);
    qDebug()<<"[dbus] api resultApi="<<resultApi;

    //cmd[3]地址不存在
    cmd.clear();
    cmd<<"kylin-photo-viewer";
    cmd<<"-api";
    cmd<<"-flip";
    cmd<<"testimage/tiff/4.tiff";
    resultApi = m_core->processingApi(cmd);
    qDebug()<<"[dbus] api resultApi="<<resultApi;

    //cmd[3]不支持的格式
    cmd.clear();
    cmd<<"kylin-photo-viewer";
    cmd<<"-api";
    cmd<<"-flip";
    cmd<<"testimage/tiff/tiff.txt";
    resultApi = m_core->processingApi(cmd);
    qDebug()<<"[dbus] api resultApi="<<resultApi;

    //cmd[4] h v c
    cmd.clear();
    cmd<<"kylin-photo-viewer";
    cmd<<"-api";
    cmd<<"-flip";
    cmd<<"testimage/2.png";
    cmd<<"-h -v -c";
    resultApi = m_core->processingApi(cmd);
    qDebug()<<"[dbus] api resultApi="<<resultApi;

}
void MyTest::tst_processingCommand()
{
    //4. 命令
//    QString path = "testimage/2.png";
    QStringList arguments;
    arguments<<"kylin-photo-viewer";
    arguments<<"-next";
    QString resultCommand = m_core->initDbus(arguments);
    qDebug()<<"[dbus] path resultCommand="<<resultCommand;
    QCOMPARE(resultCommand == "", true);


    QStringList cmd;
    cmd << "-next";
    m_core->processingCommand(cmd);
    cmd.clear();
    cmd << "-back";
    m_core->processingCommand(cmd);
    cmd.clear();
    cmd << "-big";
    m_core->processingCommand(cmd);
    cmd.clear();
    cmd << "-small";
    m_core->processingCommand(cmd);
    cmd.clear();
    cmd << "-origin";
    m_core->processingCommand(cmd);
    cmd.clear();
    cmd << "-auto";
    m_core->processingCommand(cmd);
    cmd.clear();
    cmd << "-rotate";
    m_core->processingCommand(cmd);
    cmd.clear();
}
void MyTest::tst_fullScreen()
{
    m_dbus = m_core->m_dbus;
    QSignalSpy spy(m_dbus, &Dbus::processingFullScreen);
    QSignalSpy spy2(m_core, &Core::fullScreen);

    m_dbus->fullScreen();
    m_core->processingFullscreen();

    QTRY_COMPARE(spy.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);
}


/*
//void MyTest::tst_coreReplaceBackground()
//{
//    QString path = "testimage/7.bmp";
//    m_core->findAllImageFromDir(path);
//    QSignalSpy spy(m_core, &Core::returnReplaceAndOrigImage);
//    uint b = 0; uint g = 0; uint r = 0; uint a = 0; bool isFirstEnter = true;
//    m_core->replaceBackground(b, g, r, a, isFirstEnter);
//    QCOMPARE(spy.count(), 1);
//    QImage image("testimage/10.png");
//    m_core->picMatting(image);
//    QCOMPARE(spy.count(), 2);
//    QSignalSpy spy2(m_core, &Core::returnFlipAndOrigImage);
//    m_core->mattingFlip(Processing::horizontal);
//    QCOMPARE(spy2.count(), 1);
//    m_core->saveCutImage(43, QPoint(181, 0), QPoint(644, 463));
//    m_core->mattingFlip(Processing::horizontal);
//    QCOMPARE(spy2.count(), 2);
//}
//void MyTest::tst_coreScanImage()
//{
//    QString path = "testimage/7.bmp";
//    m_core->findAllImageFromDir(path);
//    QSignalSpy spy(m_core, &Core::returnScanAndOrigImage);
//    m_core->scanImage();
//    QCOMPARE(spy.count(), 1);
//    m_core->scannerAddWatermark(false, "");
//    QCOMPARE(spy.count(), 2);
//    m_core->scannerSaveCutImage(52, QPoint(204, 74), QPoint(720, 446));
//    m_core->scannerFlip(Processing::horizontal);
//    QPrinter printer(QPrinter::HighResolution);
//    m_core->printScanImage(&printer);
//    m_core->scannerSavePDF(&printer);

//    m_core->scannerAddWatermark(true, "test");
//    QCOMPARE(spy.count(), 4);
//    m_core->scannerSaveCutImage(52, QPoint(204, 74), QPoint(720, 446));
//    m_core->scannerFlip(Processing::horizontal);
//    m_core->printScanImage(&printer);
//    m_core->scannerSavePDF(&printer);
//}
*/

void MyTest::tst_close()
{
    QSignalSpy spy(m_coreInteraction, &CoreInteraction::coreClose);
    m_coreInteraction->close();
    m_core->close();
    QCOMPARE(spy.count(), 1);
}

QTEST_APPLESS_MAIN(MyTest)
#include "tst_mytest.moc"
