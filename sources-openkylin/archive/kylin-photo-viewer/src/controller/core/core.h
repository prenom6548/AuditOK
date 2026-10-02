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
#ifndef CORE_H
#define CORE_H

#include "base.h"
#include <cmath>
#include <climits>
#include "ocr.h"
#include <QThread>
#define SET_BACKGROUND_PICTURE_GSETTINGS_PATH "org.mate.background"
#define SET_BACKGROUND_PICTURE_GSETTINGS_NAME "pictureFilename"

class MyStandardItem : public QStandardItem
{
public:
    void setPath(const QString &path);
    QString getPath();

private:
    QString m_path;
};

class Core : public QObject, public ImageShowStatus
{
    Q_OBJECT

Q_SIGNALS:
    void needOpenImage(QString path);                  //启动时打开图片
    void openFinish(QVariant var);                     //打开完成
    void coreProgremExit();                            //结束进程
    void changeAlbumHighLight(QModelIndex modelIndex); //在相册中选中到所切换的图片
    void delayShow(bool isLoading);                    //处理图片加载缓慢问题
    void openFromAlbum();                              //从相册打开
    void renameResult(int code, QFileInfo newPath);    //返回报错信息和新路径
    void canDelImage(bool canDelOrnot);                //返回是否可删除信号
    void updateSideNum(int sideBength);                //更新相册长度
    void fullScreen();                                 //处理dbus信号-全屏
    void releaseRegion(QPoint start, QPoint end);
    void cutSaveFinish();           //裁剪结束
    void returnMouseType(int type); //回传鼠标形状
    void isCutNotSave();            //关闭时-正在裁剪，给前端发信号询问是否保存更改
    void getTextResult(QVector<QString>);
    void exitOcrModeSign();
    void startOcr(QString path);

    void getLeftUpPosAccordingImage(QPointF leftUpPos); //返回已经显示的部分相对在图片上的位置

    void saveAsFaild(bool isSaveAsFaild);        //另存为失败
    void updateDefaultProportion(double proportion);  //拖拽窗口更新自适应窗口比例
    void saveCutImageFaild(QString format);

    void returnReplaceAndOrigImage(QImage replaceImage, QImage origImage); //返回背景替换之后的图片和原图
    void returnFlipAndOrigImage(QImage filpImage, QImage origImage);       //返回翻转图片
    void returnScanAndOrigImage(QImage scanImage, QImage origImage);       //返回扫描之后的图片和原图
    void returnScannerFlipAndOrigImage(QImage filpImage, QImage origImage);//返回翻转图片
    void returnScannerSaveResult(bool result);                             //返回保存结果

    void showLoadingImageWid();

public:
    Core();
    ~Core();
    QString initDbus(const QStringList &arguments);    //初始化Dbus模块
    void initMainImageLoad(QString fullPath);
    void findAllImageFromDir(QString fullPath);        //寻找目录下所有支持的图片
    void findImageFromDirFinish(QStringList list, QString path);
    void openImage(QString fullPath);                  //打开图片
    void changeImage(const int &type);                 //切换图片
    void changeImageFromClick(QModelIndex modelIndex); //切换图片
    void changeWidgetSize(const QSize &size);          //切换窗口大小
    void changeImageShowSize(ImageShowStatus::ChangeShowSizeType type,
                             QPoint focus = QPoint()); //图片显示状态（放大缩小）
    void flipImage(const Processing::FlipWay &way);                                      //翻转处理
    void deleteImage();                                                                  //删除图片
    void setAsBackground();                                                              //设置为背景图
    #ifndef UNUI
    void openInfile();                                                                   //文件夹中打开
    #endif
    void changeOpenIcon(QString theme);        //主题改变时切换相册打开按钮
    void saveMovieFinish(const QString &path); //异步处理完成
    void tiffOperateFinish(const QString &path);
    void close();                                            //关闭进程
    QStandardItemModel *getAlbumModel();                     //获取相册model指针
    void albumLoadFinish(QVariant var1, QVariant var2, QVariant var3);//预览加载缩略图完成
    void toCoreChangeName(QString oldName, QString newName); //接收重命名的处理
    void toPrintImage(QPrinter *printer);                    //打印
    bool openFail(QString fullPath);                         //打开失败，判断是否为拖拽打开失败
#ifndef UNUI
    void cutImage();                                         //开始裁剪，处理显示的图片
#endif
    void cutRegion(const QPoint &point);                     //裁剪区域
    void startChange(const QPoint &point);                   //起始点-裁剪
    void realsePos(const QPoint &point);                     //裁剪-释放鼠标
    void releaseCutChange(int type, const QPoint &point);    //裁剪-释放鼠标后再次改变尺寸
    void exitCut();                                          //退出裁剪
    void needSaveCutImage(int proportion);                   //保存裁剪图片
    void needSaveAsImage(QString path);                      //另存为
    void reloadImage(QModelIndex modelIndex);                //判断是否需要加载
    void getTextFromImage();                                 //获取当前打开图片中文字
    void getDefaultProportion(const QSize &size);



    // qml------------------------------------------
    void qmlCutImageDisplay(QPointF startPos, QSizeF displaySize, double imageScale); //处理裁剪
    void qmlTiffPage(int pageNum, QPointF leftUpPosAccordingShowImage, QSizeF displaySize, double imageScale);
    void qmlTiffImagePage(int pageNum);
    //裁剪-显示区域全图
    void cutFullViableImage();

    //标注
    void startMark();
    void markImage(QVariant var); //按下后，图片标注
    void markUndo();
    void markSave(QString savePath);
    void markClear();

    void resizeImage(int w, int h);

    //背景替换
#ifdef USE_ONNXRUNTIME
    QImage m_transparentImage;
    QImage m_origImage;
    QImage m_replaceImage;
    QImage m_mattingCutImage;
    QImage m_origMattingImage;
    bool m_isSaveCutImage = false;
    void matToSamples();
    void picMatting(QImage pic);
    void replaceBackground(uint b, uint g, uint r, uint a, bool isFirstEnter);
    void finishMatting(QImage image);
    void needSaveReplaceImage(QString path);
    void mattingFlip(const Processing::FlipWay &way);
    void saveCutImage(int proportion, const QPoint &startP, const QPoint &endP);  //保存裁剪图片
#endif

    //扫描黑白件
#ifdef SUPPORTSCANNER
    bool m_isWatermark = false;             //是否添加水印
    QImage m_scanImage;                          //扫描的图片
    QImage m_watermarkImage;                     //添加水印的图片
    QImage m_origScanImage;                 //原图
    void scanImage();                                                           //扫描图片
    void needSaveScanImage(QString savePath);                                   //保存图片
    void printScanImage(QPrinter *printer);                                     //调用全局打印
    void scannerSavePDF(QPrinter *printer);                                     //另存为PDF
    void scannerAddWatermark(bool isWatermark, QString vmContent);              //添加水印
    void scannerFlip(const Processing::FlipWay &way);                           //旋转和翻转操作
    void scannerSaveCutImage(int proportion, const QPoint &startP, const QPoint &endP);
#endif

private Q_SLOTS:
    void onSleep(bool sleeping);        //睡眠处理

private:
    void initCore();                    //初始化核心
    Dbus *m_dbus = nullptr;             // DBus模块对象
    void mainImageLoadFinish(QString path, QVariant var1, QVariant var2);
    void initMainImageLoadFinish(QString path, QVariant var1, QVariant var2);
    void showImage(const QImage &img); //显示图片
    void showImageOrMovie();            //显示图片或动图
    void creatImage(const int &proportion = -1,
                    bool defaultOpen = true); //生成图像，defaultOpen区分是否是打开图片，无其他操作
    void defaultImage(int proportion, double defaultProportion); //默认显示图片原则
    void operateImage(int proportion, double defaultProportion); //进行操作显示图片原则
    void processingCommand(const QStringList &cmd);           //处理终端命令
    void manageCacheSize(); // 管理缓存大小，实现LRU淘汰策略
    QString processingApi(const QStringList &cmd);            //处理外部命令
    void processingFullscreen();
    void creatModule(QStringList list);                       //创建module
    void loadAlbum(QStringList list, int start = 1, int end = 1); //加载相册

    void playMovie();                                                               //播放动图的槽函数
    inline void changeImageType(QString path = "");                                 //修改图片标签
    QImage changeMat(QImage image);                                                         //更改当前图片
    void deleteAlbumItem(const QString &path); //删除相册中的项
    #ifndef UNUI
    ChamgeImageType nextOrBack(const QString &oldPath, const QString &newPath);
    #endif
    RenameState successOrFail(const QString &oldPath, const QString &newPath); //返回重命名结果
    QString nextImagePath(const QString &oldPath);
    QString backImagePath(const QString &oldPath);
    void setHighLight(const QString &path);
    bool coreOperateTooOften();              //操作过于频繁
    void progremExit();                      //结束进程
    bool apiFunction();                      //处理api函数
    bool isSamePath(QString path);           //判断打开的是不是相同路径
    QStringList diffPath(QStringList image); //判断打开的同一个路径下的图片文件夹内是否还有其他图片
    void needSave();                         //判断是否需要保存图片
    void processNewLoadImage(); //处理--上一次图片没处理完就进行下一次操作时，更新对齐动图每一帧的操作状态
    bool m_isclose = false;
    MyStandardItem *m_item0 = nullptr;
    bool m_shouldClose = false;     //可以关闭
    bool m_isApi = false;           //作为API接口运行
    bool m_apiReplaceFile = false;  // api替换原图
    QStringList m_apiCmd;           //保存操作命令
    QTimer *m_canProcess = nullptr; //限制操作频率，降低cpu占用

    //裁剪部分
    QPoint boundingSet(QPoint point, int xMax, int yMax); //计算边界值
    void cutFinishReset();                                //裁剪完毕，状态重置
    void openSaveImage();                                 //针对于裁剪和另存为，打开保存的图片
    void loadOnChange(QString path);                      //切换时加载--拿QModelIndex

    bool m_isCanDel = false;                          //判断是否可删除，默认为true
    QImage m_origCutImage;                            //待继续处理和显示的裁剪的图，带灰色蒙层的
    QImage m_origCutImageBefore;                      //待显示的裁剪的背景图-原图
    bool m_isCutting = false;                         //正在裁剪
    QPoint m_cutBeforeStartPosition = QPoint(-1, -1); //记录起始点
    QPoint m_cutStartPostionSave = QPoint(-1, -1);    //保存前一个裁剪的起始点
    QSize m_cuthightlightSize;                        //高亮区域大小;
    QSize m_cuthightlightSizeBefore = QSize(0, 0);    //高亮区域大小-上一次的
    int m_cutspaceWidth = 0;                          //导航栏窗口与缩略图左边缘距离
    int m_cutspaceHeight = 0;                         //导航栏窗口与缩略图上边缘距离
    QImage m_cutReleaseImage;                                //记录鼠标释放后的图像
    bool m_hasCutBox = false;                         //有裁剪框存在的情况
    QPoint m_startP = QPoint(-1, -1);
    QPoint m_endP = QPoint(-1, -1);
    QPoint m_backPointRelease = QPoint(-1, -1);
    bool m_isReleasePaint = false; //正在进行释放后又绘制的动作--默认为false，没有进行动作
    int m_backType = -1;           //标记上一个动作的鼠标类型
    bool m_isCutSaved = false;     //记录裁剪是否保存--默认为false--已完成或未开始
    QString m_paperFormat;
    QStringList m_loadedImage;  //已经加载了的图片
    QHash<QString, QPair<ImageAndFileinfo, QImage>> m_loadedImageCache;  //预加载图片
    QHash<QString, QImage> m_bigImageCache;  //大分辨率图片按窗口大小缩放缓存图片
    QStringList m_allImageList; //所有图片的路径
    int m_currentPage = 0;      //存当前页，处理某些特殊情况

    static const int MAX_CACHE_SIZE = 50;  // 缓存最大图片数量
    static const int MAX_ALBUM_THREADS = 10; // 相册加载最大线程数
    QList<QString> m_cacheAccessOrder; // LRU缓存访问顺序
    // 动态调整的阈值（低内存时降低）
    int m_currentMaxAlbumThreads = MAX_ALBUM_THREADS;
    int m_currentMaxCacheSize = MAX_CACHE_SIZE;
    // 获取系统可用内存（MB）
    qint64 getAvailableMemoryMB();
    // 检查内存并动态调整线程和缓存阈值
    bool checkAndAdjustForLowMemory();

    bool judgeSizeIsZero(QSize size); //判断尺寸为中宽高有等于0的
    CutRegion cutRegionSelection(QPointF startPos, QSize cutSize, QImage nowImage);
    CutRegion calcCutRegionSelection(QPointF startPos, QSize cutSize, QImage nowImage);
    bool m_isClickBtnChangeImageSize = false; //判断是通过点击缩放按钮来改变的图片尺寸，默认不是
    int m_imageShowWay = 3;                   //图片显示方式--默认自适应
    bool m_imageNeedUpdate = true;            //图片是需要更新--默认需要
    bool m_isDragUpdateImage = true; //是否为拖动导航栏显示图片-需要更新图片，默认为拖动，更新图片
    void processLog();
    QProcess *m_process = nullptr;
    void fileChangedNeedUpdated();
    QString m_monitoredFiles = "";
    void sendOcrResult(QString path, QVector<QString> result); // ocr识别完成后，发送出去给前端

    QString m_backImagePath = "";

    QPoint m_imageLeftPos = QPoint(-1, -1);


    QPointF qmlBoundaryPos(QPointF pointf, int xMax, int yMax);
    QSizeF qmlBoundarySize(QPointF pointf, QSizeF displaySize);
    double m_qmlProportion = 0;
    QPointF m_backHightLight = QPointF(-1, -1);
    QList<QImage> m_tiffImageList;
    void tiffOperate();
    QList<FIBITMAP *> m_fibilist;
    int m_tiffPage = 1;
    int m_backPage = -1;
    bool m_tiffHasOperatedFinish = true;
    QString m_nowRealFormat = "";
    Mark::MarkTool m_markTool;
    QImage m_origPix;
    QList<Processing::FlipWay> m_rotateRecord;
    QString m_naviBackPixPath = "";

    bool m_needDel = false;

    QPointer<FindImageFromDir> m_findImageThread = nullptr;
    QList<AlbumThumbnail*> m_albumThumbnailLists;
};

#endif // CORE_H
