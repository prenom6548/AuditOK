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
#ifndef VARIABLE_H
#define VARIABLE_H

#include "../codec/kylinimagecodec/kylinimagecodec.h"
#include <QString>
#include <QGSettings>
#include <QDir>
#include <QSize>
#include <QPixmap>
#include <QStandardPaths>
#include <QDebug>
#include <unistd.h>
struct ImageAndInfo
{
    QFileInfo info;                    //信息
    QImage image;                     //图片
    QImage originImage;               //原图
    int imageNumber;                   //标签
    double proportion;                 //比例
    double origProportion;             //只针对大图片的原图比例
    QString colorSpace;                //图片空间
    QString imageSize;                 //图片尺寸
    bool openSuccess = true;           //判断打开原图是否成功，默认为true，打开成功
    QSize displaySizeOfPicture;        //正在显示的图片的尺寸
    int imageShowWay = 3;              //图片显示方式
    bool imageNeedUpdate = true;       //图片需要更新，默认需要
    QSize actualSize;                  //记录实际大小
    QPoint zoomFocus;                  //焦点坐标
    QPointF leftUpPos = QPointF(0, 0); //左上角坐标
    QPointF actualClickPos;            //实际点击坐标
    QPointF actualPos;                 //实际坐标
    bool movieImage;                   //是否是动图
    QList<QImage> imageList;           //图片的image集合--目前只需要对tiff这样做
    QString realFormat;
};
struct cutBoxColor
{
    QPoint startPostion; //--备用--起点
    QPoint endPostion;
    bool colorChange = true; //判断是黑变白还是白变黑，默认黑变白-true
    int key = 50;            //透明度
};

struct MarkPainterSet
{
    QPoint recordStartPos = QPoint(-1, -1);  //记录开始位置--标注
    QPoint recoedMovePos = QPoint(-1, -1);   //记录移动位置--标注
    QPoint recordEndPos = QPoint(-1, -1);    //记录结束位置
    int arrowWidth = 10;                     //记录箭头的宽度
    int arrowHeight = 18;                    //记录箭头的高度
    QPoint arrowStartPos = QPoint(-1, -1);   //记录箭头的第一个点
    QPoint arrowEndPos = QPoint(-1, -1);     //记录箭头的第三个点
    double opactity = 0.5;                   //记录透明度
    int type = -1;                           //记录画笔类型--标注：0，1，2，3，4，5，6，7
    int thickness = -1;                      //记录画笔粗细--标注：
    QColor painterColor = QColor(255, 0, 0); //记录画笔颜色--标注-默认红色
    QVector<QPoint> vp;                      //记录自由绘制的点的集合
    QRect rectInfo = QRect(0, 0, 100, 100);  //记录矩形和马赛克的方块
    double circleLeftX = 0;                  //记录圆/椭圆的左上角的x
    double circleLeftY = 0;                  //记录圆/椭圆的左上角的y
    int paintWidth = 100;                    //记录绘图的宽度
    int paintHeight = 100;                   //记录绘图的高度
    //文字设置保存
    struct TextSet
    {
        bool isBold = false;
        bool isDeleteLine = false;
        bool isUnderLine = false;
        bool isItalics = false;
        QString fontType = "华文宋体";
        int fontSize = 6;
        QString textContent = QString("测试文字");
    } m_textSet;
};
Q_DECLARE_METATYPE(MarkPainterSet) // QVarant 注册自定义类型
enum class DisplayMode : int {
    NormalMode = 0,
    CuttingMode, //剪裁模式
    OCRMode,     // OCR模式
    SignMode,    //标注模式
    ScannerMode, //扫描模式
    MattingMode, //抠图模式
    FilterMode   //滤镜模式
};
extern DisplayMode displayMode;

enum class OperayteMode : int {
    NormalMode = 0,
    ZoomIn = 1,       //放大
    ZoomOut = 2,      //缩小
    RotateN = 3,      //逆时针旋转
    RotateS = 4,      //顺时针旋转
    FlipH = 5,        //水平
    FlipV = 6,        //垂直
    Window = 7,       //适应窗口
    LifeSize = 8,     //原始尺寸
    Ocr = 9,          // ocr
    Cut = 10,         //裁剪
    NoOperate = 11,   //无操作
    ChangeImage = 12, //切图
    ExitCut = 13,     //退出裁剪
    ExitOcr = 14,     //退出裁剪
    Sign = 15,        //进入标注
    ExitSign = 16,    //退出标注
    NextImage = 17,   //下一张
    BackImage = 18,   //上一张
    Matting = 19,      //抠图
    Scanner = 20,    //扫描黑白件
};
extern OperayteMode operayteMode;

enum class MattingOperayteMode : int {
    NormalMode = 0,
    ZoomIn = 1,       //放大
    ZoomOut = 2,      //缩小
    RotateN = 3,      //逆时针旋转
    RotateS = 4,      //顺时针旋转
    FlipH = 5,        //水平
    FlipV = 6,        //垂直
    Window = 7,       //适应窗口
    LifeSize = 8,     //原始尺寸
    Cut = 10,         //裁剪
    NoOperate = 11,   //无操作
    ChangeImage = 12, //切图
};
extern MattingOperayteMode mattingOperayteMode;

enum class ScannerOperayteMode : int {
    NormalMode = 0,
    ZoomIn = 1,       //放大
    ZoomOut = 2,      //缩小
    RotateN = 3,      //逆时针旋转
    RotateS = 4,      //顺时针旋转
    FlipH = 5,        //水平
    FlipV = 6,        //垂直
    Window = 7,       //适应窗口
    LifeSize = 8,     //原始尺寸
    Cut = 10,         //裁剪
    NoOperate = 11,   //无操作
    ChangeImage = 12, //切图
    ExitCut = 13,     //退出裁剪
};
extern ScannerOperayteMode scannerOperayteMode;

struct CutRegion
{
    QPointF startPostion; //起点
    int width = 1;        //裁剪宽度，最小宽度1
    int height = 1;       //裁剪高度，最小高度1
};
Q_DECLARE_METATYPE(ImageAndInfo) // QVarant 注册自定义类型
Q_DECLARE_METATYPE(cutBoxColor)  // QVarant 注册自定义类型

namespace CutMinValue
{
constexpr int minValue = 1;
}
class Variable
{

public:
    static const QString FITTHEMEWINDOW;
    static const QString FITTRANSPARENCY;
    static const QString PROGRAM_NAME;                     //项目名称
    static const QString PHOTO_VIEW_DBUS_SERVICENAME;      // DBUS服务名称
    static const QString PHOTO_VIEW_GSETTINGS_SERVICENAME; // QGSettings服务名称
    static const QString TEMP_PATH;                        //临时文件路径
    static const QString PHOTO_VIEW_DBUS_PARH;             // DBUS路径
    static const QString PHOTO_VIEW_DBUS_INTERFACE;        // DBUS接口
    static const QPair<QString, QString> LOG_PATH;         //日志路径
    static QtMsgType g_logLevel;                           //日志级别
    static bool g_logToFile;                               //日志写入文件
    static const qint64 MAX_LOG_SIZE;                      //最大日志文件大小
    static const QMap<QString, QString> SUPPORT_CMD;       //支持的命令列表
    static const QStringList SUPPORT_FORMATS;              //支持的格式列表
    static const QString SUPPORT_FORMATS_CLASSIFY;         //支持的格式列表分类
    static const QStringList BACKGROUND_SUPPORT_FORMATS;   //壁纸支持的格式列表
    static const QStringList SUPPORT_FORMATS_END;          //
    static const QString SUPPORT_FORMATS_SAVEAS;           //另存为格式
    static const QStringList SUPPORT_FORMATS_NOT; //另存为格式-暂时不支持别的格式保存为此些格式
    static const QSize ALBUM_IMAGE_SIZE;          //相册缩略图尺寸
    static const int PICTURE_DEEPEN_KEY;          //图片加深值
    static const int REFRESH_RATE;                //刷新间隔
    static const int RESIZE_KEY;                  //每次放大缩小的值5-150
    static const int RESIZE_KEY_SECOND;           //每次放大缩小的值150-300
    static const int RESIZE_KEY_THIRD;            //每次放大缩小的值300-500
    static const int RESIZE_KEY_FOURTH;           //每次放大缩小的值500-1000
    static const int RESIZE_KEY_MAX;              //能够放大的最大值
    static const int RESIZE_KEY_MIN;              //能够缩小的最小值
    static const int RESIZE_SPEED;                //缩放速度
    static const int DEFAULT_MOVIE_TIME_INTERVAL; //默认动图时间间隔
    static const QString API_TYPE;                //是否为外部作为API调用的标识
    static const QStringList canSupportFormats;   //支持打开的格式列表
    static QString g_themeStyle;                                       //主题切换
    static QStringList g_damagedImage;                                 //存所有损坏图片路径
    static void setSettings(const QString &key, const QVariant &vlue); //写入配置文件
    static QVariant getSettings(const QString &key);                   //读取配置文件
    static int LOADIMAGE_NUM;                                          //一次加载的数量
    static const int DEFAULT_WHITE_COLOR;                              //绘制的默认白色色值
    static const int DEFAULT_BLACK_COLOR;                              //绘制的默认黑色色值
    static const QString DAMAGED_IMAGE_BLACK;                          //深色模式下的损坏图片路径
    static const QString DAMAGED_IMAGE_WHITE;                          //浅色模式下的损坏图片路径
    static const double PERCENTAGE;                                    //百分比
    static QString platForm;                                           //平台架构
    static QString startAppName;                                       //启动看图的应用名称
    static QStringList startAppNameList;                               //启动看图的应用名称列表
    static const QString SIGN_APP_NAME;                                //
    //标注
    static bool g_hasrote; //是否被翻转过
    static bool g_allowChangeImage; //标注的时候是否允许切换图片
    static bool g_needImageSaveTip; //是否需要弹窗提示图片保存
    static bool g_needEnterSign;
    static bool g_hasRotation;
    static bool g_hasIllegalSuffix;
    static bool g_delOpenNext;
    static bool g_needSaveas;
    static double g_acturalWidHeight;
    static double g_acturalWidWidth;
    static bool g_needChangeAlbumPos;
    static bool g_mainlineVersion;
    static const QStringList MOVIETYPE;
    static bool g_nextOrPreviousTiff; //记录是通过tiff小工具栏的窗口进行的切换tiff页

    static bool g_isClosing;
    static QSize g_widSize;
    static bool g_sidebarState;
    static bool g_clearImage;
    static std::atomic<bool> g_exitApp;

    static const int BIGRESOLUTION_IMAGE;  //超过此值则为大分辨率图片（可根据实际情况修改）
    static bool g_isShowOrigImage;         //是否需要显示大分辨率图片的原图
    static double g_devicePixelRatio;       //系统缩放率
    static bool checkGraphicsCardModel();   //显卡判断
private:
    static QGSettings *m_settings;                       //配置文件对象
    static QGSettings *getSettings();                    //初始化gsettings
    static QMap<QString, QString> getSupportCmd();       //创建支持的命令列表
    static QString platFormType();                       //获得平台名字
};

#endif // VARIABLE_H
