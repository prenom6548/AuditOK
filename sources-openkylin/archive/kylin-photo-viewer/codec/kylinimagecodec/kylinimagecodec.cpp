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
#include "kylinimagecodec.h"
#include <QStringList>
KylinImageCodecSignals *KylinImageCodec::m_signalObj = nullptr;

KylinImageCodecSignals *KylinImageCodec::getSignalObj()
{
    if (m_signalObj == nullptr) {
        m_signalObj = new KylinImageCodec;
    }
    return m_signalObj;
}
std::atomic<bool> KylinImageCodec::g_exitApp{false};
QList<LoadMovie*> KylinImageCodec::m_LoadMovieThreads;
std::atomic<bool> KylinImageCodec::m_flagGifLoad{false};

bool KylinImageCodec::g_needSaveAs = false;

const QStringList KylinImageCodec::m_qtSupported = {
    "bmp", "xpm", "xbm", "jpe", "jpg", "jpeg", "pnm",
    "webp", "wbmp", "ico", "tif", "tiff","png"
};

const QStringList KylinImageCodec::m_supportFormatsSaveOnQt = {
     "png","jpg", "jpeg","bmp","pbm", "ppm", "xbm", "xpm", "ico","wbmp", "webp",  //"pgm", 会变黑色
};

const QStringList KylinImageCodec::m_supportFormatsSaveOnfreeimage = {
    "png", "jpg", "jpeg", "jfi", "jif", "jp2", "j2k", "jng",            //JPEG格式图片保存会加深绿色,"jpg", "jpeg",可以qt保存
    "bmp", "pbm", "pgm",  "pnm", "ppm", "xbm", "tif", "tiff",
    "psd", "wbmp","webp","dib", "exr",  "jpe",  "tga", "sr"             //ras 不支持保存
};

const QStringList KylinImageCodec::m_freeimageSupportFormats = {       //可读取
    "exr", "psd",  "jfi", "jif", "jp2", "j2k", "jng", "wbmp", "xbm", "pnm", "tiff",
    "tif", "webp", "bmp", "dib", "png", "jpg", "jpe", "jpeg", "pbm", "pgm", "ppm"};

const QStringList KylinImageCodec::m_otherSupportFormats = { "svg", "gif", "apng" }; //"tga","ico", "xpm"

const QString KylinImageCodec::m_supportFormatsClassify = "(*.jpg *.jpe *.jpeg);;"
                                                         "(*.pnm *.pbm);;"
                                                         "(*.pgm *.ppm);;"
                                                         "(*.tiff *.tif);;"
                                                         "(*.png *.apng);;"
                                                         "(*.ico);;(*.sr);;"
                                                         "(*.ras);;(*.tga);;"
                                                         "(*.svg);;(*.gif);;"
                                                         "(*.webp);;(*.bmp *.dib);;"
                                                         "(*.exr);;(*.psd);;(*.jfi *.jif);;"
                                                         "(*.jp2 *.j2k *.jng);;(*.wbmp);;"
                                                         "(*.xbm *.xpm)";
const QStringList KylinImageCodec::m_supportFormats = [](){
    QStringList list =  KylinImageCodec::m_supportFormatsSaveOnQt
                        + KylinImageCodec::m_freeimageSupportFormats
                        + KylinImageCodec::m_otherSupportFormats;
    list.sort();
    list.removeDuplicates();
    return list;
}();

const QStringList KylinImageCodec::m_supportFormatsMT = {"image/vnd.microsoft.icon",
                                                         "image/x-jp2-codestream",
                                                         "image/vnd.adobe.photoshop",
                                                         "image/webp",
                                                         "image/svg+xml",
                                                         "image/x-jng",
                                                         "image/jp2",
                                                         "image/gif",
                                                         "image/vnd.wap.wbmp",
                                                         "image/x-xbitmap",
                                                         "image/x-xpixmap",
                                                         "image/x-exr",
                                                         "image/x-portable-graymap",
                                                         "image/x-portable-anymap",
                                                         "image/x-portable-pixmap",
                                                         "image/x-cmu-raster",
                                                         "image/x-portable-bitmap",
                                                         "image/x-tga",
                                                         "image/x-sun-raster",
                                                         "image/bmp",
                                                         "image/jpeg",
                                                         "image/tiff",
                                                         "image/png"};
const QHash<QString, int> KylinImageCodec::m_formats = KylinImageCodec::creatFormats();
QStringList KylinImageCodec::g_allFormat = {
    "BMP",    "ICO", "JPG", "JPE",  "JPS",    "JPEG", "KOALA", "KOA",    "LBM", "IFF", "MNG",   "PBM",
    "PBMRAW", "PCD", "PCX", "PGM",  "PGMRAW", "PNG",  "PPM",   "PPMRAW", "RAS", "TGA", "TARGA", "TIFF",
    "TIF",    "PCT", "PIC", "PICT", "WEBP",   "JXR",  "PFM",   "DDS",    "GIF", "HDR", "FAX",   "G3",
    "SGI",    "CUT", "JNG", "WBMP", "PSD",    "XBM",  "XPM",   "EXR",    "JP2", "J2K"};
QHash<QString, int> KylinImageCodec::creatFormats()
{
    QHash<QString, int> tmpFormats;
    tmpFormats.insert("BMP", FIF_BMP);
    tmpFormats.insert("ICO", FIF_ICO);
    tmpFormats.insert("JPG", FIF_JPEG);
    tmpFormats.insert("JPE", FIF_JPEG);
    tmpFormats.insert("JPS", FIF_JPEG);
    tmpFormats.insert("JPEG", FIF_JPEG);
    tmpFormats.insert("KOALA", FIF_KOALA);
    tmpFormats.insert("KOA", FIF_KOALA);
    tmpFormats.insert("LBM", FIF_LBM);
    tmpFormats.insert("IFF", FIF_LBM);
    tmpFormats.insert("MNG", FIF_MNG);
    tmpFormats.insert("PBM", FIF_PBM);
    tmpFormats.insert("PBMRAW", FIF_PBMRAW);
    tmpFormats.insert("PCD", FIF_PCD);
    tmpFormats.insert("PCX", FIF_PCX);
    tmpFormats.insert("PGM", FIF_PGM);
    tmpFormats.insert("PGMRAW", FIF_PGMRAW);
    tmpFormats.insert("PNG", FIF_PNG);
    tmpFormats.insert("PPM", FIF_PPM);
    tmpFormats.insert("PPMRAW", FIF_PPMRAW);
    tmpFormats.insert("RAS", FIF_RAS);
    tmpFormats.insert("TGA", FIF_TARGA);
    tmpFormats.insert("TARGA", FIF_TARGA);
    tmpFormats.insert("TIFF", FIF_TIFF);
    tmpFormats.insert("TIF", FIF_TIFF);
    tmpFormats.insert("PCT", FIF_PICT);
    tmpFormats.insert("PIC", FIF_PICT);
    tmpFormats.insert("PICT", FIF_PICT);
    tmpFormats.insert("WEBP", FIF_WEBP);
    tmpFormats.insert("JXR", FIF_JXR);
    tmpFormats.insert("PFM", FIF_PFM);
    tmpFormats.insert("DDS", FIF_DDS);
    tmpFormats.insert("GIF", FIF_GIF);
    tmpFormats.insert("HDR", FIF_HDR);
    tmpFormats.insert("FAX", FIF_FAXG3);
    tmpFormats.insert("G3", FIF_FAXG3);
    tmpFormats.insert("SGI", FIF_SGI);
    tmpFormats.insert("CUT", FIF_CUT);
    tmpFormats.insert("JNG", FIF_JNG);
    tmpFormats.insert("WBMP", FIF_WBMP);
    tmpFormats.insert("PSD", FIF_PSD);
    tmpFormats.insert("XBM", FIF_XBM);
    tmpFormats.insert("XPM", FIF_XPM);
    tmpFormats.insert("EXR", FIF_EXR);
    tmpFormats.insert("JP2", FIF_JP2);
    tmpFormats.insert("J2K", FIF_J2K);
    return tmpFormats;
}
QStringList KylinImageCodec::g_jpegList = {"jpg", "jpe", "jps", "jpeg", "jif", "jfi"};
QStringList KylinImageCodec::g_bmpList = {"bmp", "dib"};
QStringList KylinImageCodec::g_koaList = {"koa", "koala"};
QStringList KylinImageCodec::g_lbmList = {"lbm", "iff"};
QStringList KylinImageCodec::g_tgaList = {"tga", "targa"};
QStringList KylinImageCodec::g_tiffList = {"tiff", "tif"};
QStringList KylinImageCodec::g_pictList = {"pct", "pic", "pict"};
QStringList KylinImageCodec::g_faxList = {"fax", "g3"};
QStringList KylinImageCodec::g_pngList = {"png", "apng"};
QStringList KylinImageCodec::g_otherFormatList = {"sr",  "ras", "exr",  "psd", "jp2", "j2k", "jng", "wbmp",
                                                  "xbm", "pnm", "webp", "svg", "gif", "ico", "xpm"};
QStringList KylinImageCodec::g_pbmList = {"pbm", "pgm", "ppm", "pnm"};

