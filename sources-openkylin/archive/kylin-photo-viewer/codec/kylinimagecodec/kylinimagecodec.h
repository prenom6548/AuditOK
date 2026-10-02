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
#ifndef KYLIN_IMAGE_CODEC_H
#define KYLIN_IMAGE_CODEC_H

#include "kylinimagecodec_global.h"
#include "savemovie.h"
#include "loadmovie.h"
#include <atomic>

struct KylinImageCodecResultBase
{
    int fps = 0;
    QFileInfo info;
    int maxFrame = 0;
    bool openSuccess = true; //判断打开是否成功，默认为true，打开成功
};
struct ImageResult : public KylinImageCodecResultBase
{
    QImage image;
    QList<QImage> *imageList = nullptr;
};

class KylinImageCodecSignals : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void loadMovieFinish(const QString &path);
    void saveMovieFinish(const QString &path);
};


class KYLINIMAGECODEC_EXPORT KylinImageCodec : public KylinImageCodecSignals
{
    Q_OBJECT

public:
    static void exitApp();                              //中止movie线程,退出app
    static std::atomic<bool> g_exitApp;                 //是否退出app标志
    static QList<LoadMovie*> m_LoadMovieThreads;        //LoadMovie 线程列表
    static std::atomic<bool> m_flagGifLoad;             //暂时禁用gif图片编辑

    static KylinImageCodecSignals *getSignalObj();

    static bool isSaving(const QString &path);
    static bool allSaveFinish();
    static QString savePath();

    //加载图片
    static ImageResult loadImage(const QString &path, QString realFormat);
    static ImageResult loadImageFromFile(const QString &path, const QString &realFormat = "");  //加载图片（静图和动图）
    //保存图片
    static bool saveImage(const QImage &image, const QString &savepath,
                          const QString &saveFormat = "", bool replace = true);
    static bool saveImage(QList<QImage> *list, const int &fps, const QString &savepath,
                          const QString &saveFormat, bool replace = true);

    //获取文件格式
    static FREE_IMAGE_FORMAT get_real_format(const QString &path);
    static const QHash<QString, int> m_formats;
    static QStringList g_jpegList;
    static QStringList g_bmpList;
    static QStringList g_koaList;
    static QStringList g_lbmList;
    static QStringList g_tgaList;
    static QStringList g_tiffList;
    static QStringList g_pictList;
    static QStringList g_faxList;
    static QStringList g_pngList;
    static QStringList g_pbmList;
    static QStringList g_allFormat;
    static QStringList g_otherFormatList;
    static bool g_needSaveAs;

    static const QStringList m_supportFormats;          // 完整支持的格式列表

private:
    //加载图片
    static ImageResult loadStaticToImage(const QString &path, const QString &suffix);       //加载静态图片
    static ImageResult loadMovieToImage(const QString &path, const QString &suffix);        //加载动态图片
    static QImage loadImageOnFreeimage(const QString &path);                                //通过freeimage加载图片
    static QImage loadImageFirstQt(const QString &path, const QString &suffix);             //优先Qt加载图片
    //保存图片
    static QString saveWay(const QString &savepath, bool replace);                                  //返回保存路径
    static bool save(const QImage &image, const QString &savepath, const QString &saveFormat);      //保存静态图片
    static bool save(QList<QImage> *list, const int &fps, const QString &savepath,                  //保存动态图片
                     const QString &saveFormat);
    static bool saveOnFreeImage(const QString &path, const QImage &image, FREE_IMAGE_FORMAT fif = FIF_UNKNOWN);//通过freeimage保存图片
    static bool saveMovie(QList<QImage> *list, const int &fps, const QString &savepath,             //保存movie图片
                          const QString &saveFormat);
    static bool writeFIBITMAPToFile(FIBITMAP *dib, const QString &path, int flag = 0);              //写freeimage位图到文件
    static int getDelay(const QString &path, const QString &suffix);                                //获取动图的延时
    static int gifDelay(const QString &path);                                                       //获取gif的延时
    //格式转换
    static QImage FIBitmap2QImage(FIBITMAP *dib);                                                   //freeimage位图转换为QImage
    static FIBITMAP *QImge2FIBitMap(const QImage &image, FREE_IMAGE_FORMAT fif = FIF_UNKNOWN);      //QImage转换为freeimage位图


    static QHash<QString, int> creatFormats();

    static KylinImageCodecSignals *m_signalObj;
    static QStringList *m_list;                         //用来保存正在保存的动图队列
    static QString m_savePath;                          //图片保存路径

    static const QStringList m_qtSupported;                         //qt支持打开的格式
    static const QStringList m_supportFormatsSaveOnQt;              // Qt image可直接保存的格式
    static const QStringList m_supportFormatsSaveOnfreeimage;       // freeimage可直接保存的格式列表
    static const QStringList m_freeimageSupportFormats;             // freeimage支持的格式列表 读取
    static const QStringList m_otherSupportFormats;                 //  其他支持格式列表

    static const QString m_supportFormatsClassify;                  //  打开文件格式列表
    static const QStringList m_supportFormatsMT;
};

#endif // KYLIN_IMAGE_CODEC_H
