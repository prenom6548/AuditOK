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
#ifndef CoreINTERACTIONCoreH
#define CoreINTERACTIONCoreH

#include <QThread>
#include <QTimer>
#include "controller/core/core.h"
#include "controller/interaction.h"

//实际实现功能
class CoreInteraction : public Interaction
{
    Q_OBJECT

Q_SIGNALS:
    QVariant coreOpenImage(const QString &path);  //打开图片  QVariant -> QHash<int,QList<int>>
    QStandardItemModel *coreGetAlbumModel();      //设置为背景图
    void coreChangeImage(const int &type);        //切换图片
    void coreChangeWidgetSize(const QSize &size); //切换窗口大小
    void coreGetDefaultProportion(const QSize &size);
    void coreChangeImageShowSize(ImageShowStatus::ChangeShowSizeType, QPoint); //图片显示状态（放大缩小）
    void coreFlip(const Processing::FlipWay &way);                             //翻转处理
    void coreDeleteImage();                                                    //删除图片
    void coreSetAsBackground();                                                //设置为背景图
    void coreOpenInfile();                                                     //文件夹中打开
    void coreChangeOpenIcon(QString theme);                   //更改主题，切换相册打开按钮图标
    void coreClose();                                         //结束进程
    void coreRename(QString oldPath, QString newPath);        //重命名
    void corePrint(QPrinter *printer);                        //打印
    void coreCut();                                           //裁剪
    void coreGetText();                                       //获取当前打开图片中文字
    void coreCutRegion(const QPoint &point);                  //裁剪区域
    void coreStartChange(const QPoint &point);                //起始点-裁剪
    void coreRealsePos(const QPoint &point);                  //裁剪-释放鼠标
    void coreReleaseCutChange(int type, const QPoint &point); //裁剪-鼠标释放后，再次进行尺寸改变
    void coreExitCut();                                       //退出裁剪
    void coreNeedSaveCutImage(int proportion);                //保存裁剪图片
    void coreSaveCutImage(int proportion, QPoint startP, QPoint endP);  //保存裁剪图片
    void coreNeedSaveas(QString path);
    void coreReloadImage(QModelIndex modelIndex); //图片加载
    // qml-----------------------------------------
    void coreQmlStartCutDisplay(QPointF startPos, QSizeF displaySize, double imageScale); // qml裁剪
    //裁剪-显示区域全图
    void coreCutFullViableImage();

    //标注
    void coreMarkImage(QVariant var); //按下标注
    void coreStartMark();
    void coreMarkUndo();
    void coreMarkSave(QString savePath);
    void coreMarkClear();
    void coreQmlTiffImagePage(int pageNum);

    //背景替换
    void corePicMatting(QImage pic);
    void coreReplaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter);
    void coreNeedSaveReplaceImage(QString path);
    void coreMattingFlip(const Processing::FlipWay &way);

    //扫描黑白件
    void coreScanImage();
    void coreNeedSaveScanImage(QString savePath);
    void corePrintScanImage(QPrinter *printer);
    void coreScannerSavePDF(QPrinter *printer);
    void coreScannerAddWatermark(bool isWatermark, QString vmContent);
    void coreScannerFlip(const Processing::FlipWay &way);
    void coreScannerSaveCutImage(int proportion, QPoint startP, QPoint endP);  //保存裁剪图片

    void coreResizeImage(int w, int h);
public:
    CoreInteraction();
    ~CoreInteraction();
    //调整图片大小
    virtual void resizeImage(int w, int h);
#ifndef UNUI
protected:
#endif
    virtual void creatCore(const QStringList &list);       //初始化核心模块
    virtual void initUiFinish();                           // UI初始化完成
    virtual void openImage(const QString &path);           //打开图片  QVariant -> QHash<int,QList<int>>
#ifndef UNUI
    virtual void changeImage(const int &type);             //切换图片
#endif
    virtual void nextImage();                              //切换图片
    virtual void backImage();                              //切换图片
    virtual void changeWidgetSize(const QSize &size);      //切换窗口大小
    virtual void getDefaultProportion(const QSize &size);
#ifndef UNUI
    virtual void watchBigImage(QPoint focus = QPoint());   //图片放大
    virtual void watchSmallImage(QPoint focus = QPoint()); //图片缩小
#endif
    virtual void watchOriginalImage();                     //查看原图
    virtual void watchAutoImage();                         //自适应窗口大小
    virtual void rotate(const bool &clockwise);                          //旋转
    virtual void flipH();                                                //水平翻转
    virtual void flipV();                                                //垂直翻转
    virtual void deleteImage();                                          //删除图片
    virtual void setAsBackground();                                      //设置为背景图
#ifndef UNUI
    virtual void openImageInfile();                                      //在文件夹中打开
#endif
    virtual void changeOpenIcon(QString theme);                          //更改主题，切换相册打开按钮图标
    virtual void close();                                                //关闭进程
    virtual QStandardItemModel *getAlbumModel();                         //获取相册model指针
    virtual void reName(QString oldPath, QString newPath);               //重命名
#ifndef UNUI
    virtual void printImage(QPrinter *printer);                          //打印
    virtual void cutImage();                                             //裁剪
#endif
    virtual void getText();                                              //获取当前打开图片中文字
    virtual void cutRegion(const QPoint &point);                         //裁剪区域
    virtual void startChange(const QPoint &point);                       //起始点-裁剪
    virtual void releasePos(const QPoint &point);                        //裁剪-鼠标释放
    virtual void releaseCutChange(int type, const QPoint &point); //裁剪-鼠标释放后，再次进行尺寸改变
    virtual void exitCut();                                       //退出裁剪
    virtual void needSaveCutImage(int proportion);                //保存裁剪图片
#ifndef UNUI
    virtual void saveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint);  //保存裁剪图片
#endif
    virtual void needSaveAs(QString path);
    virtual void reloadImage(QModelIndex modelIndex);

    // qml------------------------------------------------------
    virtual void qmlStartCutDisplay(QPointF startPos, QSizeF displaySize, double imageScale);
    virtual void clickCutFullViableImage();

    //标注
    virtual void startMark();
    virtual void toMarkImage(QVariant var);
    virtual void markUndo();
    virtual void markSave(QString savePath);
    virtual void markClear();
    virtual void qmlTiffImagePage(int pageNum);
#ifndef UNUI
    //背景替换
    virtual void picMatting(QImage pic);
    virtual void replaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter);
    virtual void needSaveReplaceImage(QString path);
    virtual void mattingRotate(const bool &clockwise = true);
    virtual void mattingFlipH();
    virtual void mattingFlipV();

    //扫描黑白件
    virtual void scanImage();                                               //扫描为黑白图片
    virtual void needSaveScanImage(QString savePath);                       //保存扫描图片
    virtual void printScanImage(QPrinter *printer);                         //全局打印
    virtual void scannerSavePDF(QPrinter *printer) ;                        //自定义打印
    virtual void scannerAddWatermark(bool isWatermark, QString vmContent);  //添加水印
    virtual void scannerRotate(const bool &clockwise = true);               //旋转
    virtual void scannerFlipH();                                            //水平翻转
    virtual void scannerFlipV();                                            //垂直翻转
    virtual void scannerSaveCutImage(int proportion, const QPoint &beginPoint, const QPoint &endPoint);  //保存裁剪图片
#endif
#ifndef UNUI
private:
#endif
    void initConnect();                        //初始化绑定
    bool coreOperateTooOften();                //操作过于频繁
    bool m_isCoreInit = false;                 //防止多次初始化核心
    QString m_needStartWithOpenImagePath = ""; // UI未初始化时设置，使其初始化完成立即加载图片
    QTimer *m_canProcess = nullptr;            //限制操作频率，降低cpu占用
    Core *m_core = nullptr;                    //核心对象
    QThread *m_thread;
};

#endif // CoreINTERACTIONCoreH
