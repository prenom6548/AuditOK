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
#include "variable.h"

const QString Variable::FITTHEMEWINDOW = QString("org.ukui.style");
const QString Variable::FITTRANSPARENCY = QString("org.ukui.control-center.personalise");

//项目名称
const QString Variable::PROGRAM_NAME = QString("kylin-photo-viewer");
// QGSettings服务名称
const QString Variable::PHOTO_VIEW_GSETTINGS_SERVICENAME = QString("org.kylin-photo-viewer.settings");
// GSettings
QGSettings *Variable::m_settings = Variable::getSettings();

// DBUS服务名称
const QString Variable::PHOTO_VIEW_DBUS_SERVICENAME = QString("org.ukui.kylin_photo_viewer");
// DBUS路径
const QString Variable::PHOTO_VIEW_DBUS_PARH = QString("/");
// DBUS接口
const QString Variable::PHOTO_VIEW_DBUS_INTERFACE = QString("kylin_photo_viewer.commands");


//支持的命令列表
const QMap<QString, QString> Variable::SUPPORT_CMD = Variable::getSupportCmd();

//支持打开的格式列表
const QStringList Variable::SUPPORT_FORMATS = {
    "exr", "psd", "jfi", "jif", "j2k", "jp2", "jng", "wbmp", "xbm", "tiff", "tif", "webp",
    "pnm", "bmp", "dib", "sr",  "ras", "pbm", "pgm", "ppm",  "png", "jpg",  "jpe", "jpeg",
    "tga", "svg", "gif", "apng", "ico", "xpm", "jfif"};
//支持打开的后缀
const QString Variable::SUPPORT_FORMATS_CLASSIFY = "(*.jpg *.jpe);;"
                                                   "(*.jpeg *jfif);;"
                                                   "(*.pnm *.pbm);;"
                                                   "(*.pgm *.ppm);;"
                                                   "(*.png *.apng);;"
                                                   "(*.ico);;(*.sr);;"
                                                   "(*.ras);;(*.tga);;"
                                                   "(*.svg);;(*.gif);;"
                                                   "(*.webp);;(*.bmp *.dib);;"
                                                   "(*.exr);;(*.psd);;(*.jfi *.jif);;"
                                                   "(*.tif *.tiff);;"
                                                   "(*.j2k *.jp2);;(*.jng);;"
                                                   "(*.wbmp);;"
                                                   "(*.xbm *.xpm)";
//另存为
const QString Variable::SUPPORT_FORMATS_SAVEAS = "PNG(*.png);;JPE(*.jpe);;JPEG(*.jpeg);;"
                                                 "JPG(*.jpg);;"
                                                 "ICO(*.ico);;"
                                                 "BMP(*.bmp);;"
                                                 "JNG(*.jng);;"
                                                 "XPM(*.xpm);;TIFF(*.tiff);;TIF(*.tif);;J2K(*.j2k);;JP2(*.jp2)";
//".gif",".apng",GIF(*.gif);;APNG(*.apng);;
const QStringList Variable::SUPPORT_FORMATS_END = {".tiff", ".tif", ".j2k",  ".jp2", ".jpg", ".jpe", ".jpeg", ".png",
                                                   ".ico",  ".svg", ".webp", ".bmp", ".dib", ".psd", ".jng",  ".xpm"};
//壁纸支持的格式列表
const QStringList Variable::BACKGROUND_SUPPORT_FORMATS =
    { //经测试
      //"jp2","tga","dib","pbm","ppm"，"sr","ras",这几种格式不支持设置为壁纸"tiff""tif"暂时因版权问题不打开，再这里也先去掉
        "jpg", "jpe", "jpeg", "pgm", "pnm", "png", "bmp", "svg", "gif", "apng", "tiff", "tif"};

//不支持转存为此种格式
const QStringList Variable::SUPPORT_FORMATS_NOT = {".jif", ".svg", ".jfi", ".exr", ".wbmp", ".tga", ".xbm",
                                                   ".pgm", ".pbm", ".sr",  ".ras", ".pnm",  ".ppm"};
//不支持裁剪
// const QStringList Variable::SUPPORT_FORMATS_NOT={".exr",".tga",".svg"};

const QSize Variable::ALBUM_IMAGE_SIZE = QSize(94, 56);  //相册缩略图尺寸
const int Variable::PICTURE_DEEPEN_KEY = 50;             //图片加深值
const int Variable::REFRESH_RATE = 15;                   //刷新间隔
const int Variable::RESIZE_KEY = 10;                     //每次放大缩小的值-5-150
const int Variable::RESIZE_KEY_SECOND = 25;              //每次放大缩小的值-150-300
const int Variable::RESIZE_KEY_THIRD = 50;               //每次放大缩小的值-300-500
const int Variable::RESIZE_KEY_FOURTH = 100;             //每次放大缩小的值-500-1000
const int Variable::RESIZE_KEY_MAX = 10000;              //能够放大的最大值
const int Variable::RESIZE_KEY_MIN = 5;                  //能够缩小的最小值
const int Variable::RESIZE_SPEED = 15;                   //缩放速度
const int Variable::DEFAULT_MOVIE_TIME_INTERVAL = 100;   //默认动图时间间隔
const int Variable::DEFAULT_WHITE_COLOR = 255;           //默认白色色值
const int Variable::DEFAULT_BLACK_COLOR = 80;            //默认黑色色值
const QString Variable::API_TYPE = QString("$api$");     //是否为外部作为API调用的标识
int Variable::LOADIMAGE_NUM = 8;                         //相册一次加载的数量
QString Variable::g_themeStyle = "";                     //主题
QStringList Variable::g_damagedImage;                    //损坏图片路径
const QString Variable::DAMAGED_IMAGE_BLACK = QString(":/res/res/damaged_imgB.png");
const QString Variable::DAMAGED_IMAGE_WHITE = QString(":/res/res/damaged_img.png");
DisplayMode displayMode = DisplayMode::NormalMode;
OperayteMode operayteMode = OperayteMode::NoOperate;
MattingOperayteMode mattingOperayteMode = MattingOperayteMode::NoOperate;
ScannerOperayteMode scannerOperayteMode = ScannerOperayteMode::NoOperate;
const double Variable::PERCENTAGE = 0.01; //百分比转换
QString Variable::startAppName = "";
QStringList Variable::startAppNameList = {"kylin-gallery"};
QString Variable::platForm = Variable::platFormType();
const QString Variable::SIGN_APP_NAME = "kolourpaint ";
bool Variable::g_hasIllegalSuffix = false;
bool Variable::g_delOpenNext = false;
bool Variable::g_needSaveas = false;
double Variable::g_acturalWidHeight = 570;
double Variable::g_acturalWidWidth = 1080;
bool Variable::g_needChangeAlbumPos = false;
std::atomic<bool> Variable::g_exitApp{false};

//标注
bool Variable::g_allowChangeImage = true;
bool Variable::g_needImageSaveTip = false;
bool Variable::g_needEnterSign = false;
bool Variable::g_hasRotation = false;
bool Variable::g_mainlineVersion = false;
const QStringList Variable::MOVIETYPE = {"gif", "apng"};
bool Variable::g_isClosing = false;
QSize Variable::g_widSize = QSize(1080, 720);

const int Variable::BIGRESOLUTION_IMAGE = 6500;
bool Variable::g_isShowOrigImage = false;
bool Variable::g_sidebarState = false;
bool Variable::g_clearImage = false;

double Variable::g_devicePixelRatio = 1.0;
QGSettings *Variable::getSettings()
{
    QGSettings *mysetting = nullptr;
    if (QGSettings::isSchemaInstalled(PHOTO_VIEW_GSETTINGS_SERVICENAME.toLocal8Bit())) {
        mysetting = new QGSettings(PHOTO_VIEW_GSETTINGS_SERVICENAME.toLocal8Bit());
        //QObject::connect(mysetting, &QGSettings::changed, &Variable::onGsettingChange);
    }
    return mysetting;
}

QMap<QString, QString> Variable::getSupportCmd()
{
    QMap<QString, QString> cmds;
    cmds.insert("-next", "下一张图片");
    cmds.insert("-back", "上一张图片");
    cmds.insert("-big", "放大显示图片");
    cmds.insert("-small", "缩小显示图片");
    cmds.insert("-origin", "显示原图");
    cmds.insert("-auto", "自适应窗口大小显示图片");
    cmds.insert("-rotate", "旋转图片");
    cmds.insert("-api", "外部接口  -api -flip [path] [cmd] [saveway]\n"
                        "  ├-cmd: -v 垂直翻转 -h 水平翻转 -c 顺时针旋转90度 | 可重复使用，例如-vhcc \n"
                        "  └-saveway:  -b 备份保存 -r 覆盖保存 | 可不填，不填相当于-b");
    return cmds;
}



QString Variable::platFormType()
{
    QFile file("/etc/os-release");
    if (!file.exists()) {
        return "";
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    char buf[512];
    memset(buf, '\0', sizeof(buf));
    while (file.readLine(buf, sizeof(buf)) != -1) {
        QString line = QString(buf).trimmed();
        if (line.contains("PROJECT_CODENAME")) {
            QStringList lineList = line.split(QChar('='));
            if (lineList.size() >= 2) {
                /* 找到具体值 , 直接返回 */
                return lineList.at(1);
            }
        }
        memset(buf, '\0', sizeof(buf));
    }
    return "";
}


//写入配置文件
void Variable::setSettings(const QString &key, const QVariant &vlue)
{
    m_settings->set(key, vlue);
}

//读取配置文件
QVariant Variable::getSettings(const QString &key)
{
    QVariant setting;
    setting = m_settings->get(key);
    if (key == "imagePath") {
        if (setting.toString().isEmpty()) {
            return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        }
    }
    return setting;
}

bool Variable::checkGraphicsCardModel()
{
    QProcess process;
    process.start("lspci");
    process.waitForFinished();
    if (process.exitCode() == 0) {
        QString output = process.readAllStandardOutput();
        return output.contains("Innosilicon", Qt::CaseInsensitive)
                || output.contains("Fantasy", Qt::CaseInsensitive);
    }
    return false;
}
