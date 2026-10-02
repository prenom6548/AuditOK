#include "imageprovider.h"
#include "qdebug.h"
#include "qpixmap.h"
#include "qimage.h"
#include <QFile>
#include <QApplication>
#include <QPalette>
#include <QSize>
#include <QColor>
#include <QPainter>
#include <qmath.h>

#include "../../../libqt5-ukui-style/effects/highlight-effect.h"
#include "icon.h"
#include "tokenparameter.h"
using namespace UKUIQQC2Style;

static QSize defaultSize = QSize(16, 16);

QIcon IconHelper::getDefaultIcon()
{
    QIcon icon;
    loadDefaultIcon(icon);
    return icon;
}

QIcon IconHelper::loadIcon(const QString &id)
{
    QIcon icon;
    if (id.isEmpty()) {
        loadDefaultIcon(icon);
        return icon;
    }

    bool isOk = false;
    QString path = toLocalPath(id);
    //qDebug() << "path......" << id << path;
    if (!path.isEmpty()) {
        QPixmap pixmap;
        isOk = initIcon(path, icon);
//        if (isOk) {
//            icon.addPixmap(pixmap);
//        }

    }
    //qDebug() << "isOk..." << isOk;
    if(!isOk) {
        QString idStr = id;
        if(idStr.startsWith("qrc:/")){
            idStr = idStr.remove(0,5);
        }
        isOk = loadThemeIcon(idStr, icon);
    }

    if (!isOk) {
        loadDefaultIcon(icon);
    }

    //qDebug() << "load icon....." << isOk << id << icon;
    return icon;
}

bool IconHelper::initIcon(const QString &path, QIcon &icon)
{
    if (!QFile::exists(path)) {
        //qWarning() << "Error: initIcon, File dose not exists." << path;
        return false;
    }

     icon.addFile(path);
     return true;
}

bool IconHelper::loadThemeIcon(const QString &name, QIcon &icon)
{
    icon = QIcon::fromTheme(name);
    if(icon.isNull())
        return false;
    return true;
}

void IconHelper::loadDefaultIcon(QIcon &icon)
{
    if (!loadThemeIcon("application-x-desktop", icon)) {
        initIcon(":/res/icon/application-x-desktop.png", icon);
    }
}

// see: https://doc.qt.io/archives/qt-5.12/qurl.html#details
QString IconHelper::toLocalPath(const QUrl &url)
{
    if (url.isEmpty()) {
        return {};
    }

    // file:
    if (url.isLocalFile()) {
        return url.path();
    }

    QString schema = url.scheme();
    if (schema.isEmpty()) {
        QString path = url.path();
        if (path.startsWith("/") || path.startsWith(":")) {
            return path;
        }

    } else {
        // qrc example: the Path ":/images/cut.png" or the URL "qrc:///images/cut.png"
        // see: https://doc.qt.io/archives/qt-5.12/resources.html
        if (schema == "qrc") {
            //qrc path: :/xxx/xxx.png
            return ":" + url.path();
        }
    }

    return {};
}

bool IconHelper::isRemoteServerFile(const QUrl &url)
{
    if (url.isEmpty() || url.scheme().isEmpty()) {
        return false;
    }

    return url.scheme() == "http" || url.scheme() == "https";
}

bool IconHelper::isThemeIcon(const QString &name)
{
    QIcon icon = QIcon::fromTheme(name);
    if(icon.isNull())
        return false;
    return true;
}

bool IconHelper::isLocalFile(const QUrl &url)
{
    return !toLocalPath(url).isEmpty();
}

QPixmap IconHelper::generatePixMap(const QString &id, Icon::Mode mode, const QSize &requestedSize, QSize defaultSize)
{
    //qDebug() << "generatePixMap ......";
    QIcon icon = IconHelper::loadIcon(id);
    //qDebug() << "model...." << mode << "icon...." << icon;
    if(icon.isNull())
        return QPixmap();
    QPixmap pixmap = icon.pixmap(requestedSize.isEmpty() ? defaultSize : requestedSize);
    if(!HighLightEffect::isPixmapPureColor(pixmap) || !HighLightEffect::isSymbolicColor(pixmap)){
        ////qDebug() << "not isSymbolicColor..." << HighLightEffect::isPixmapPureColor(pixmap) << HighLightEffect::isSymbolicColor(pixmap);
        return pixmap;
    }
    if(mode == Icon::NormalWidthThemeColor ||
            mode == Icon::HoveredWidthThemeColor ||
            mode == Icon::ClickedWidthThemeColor ||
            mode == Icon::DisableWidthThemeColor){
        pixmap = generatedWidthThemeColorPixmap(pixmap, mode);

    }
    else if(mode == Icon::Highlight || mode == Icon::HighDisabled){
        ////qDebug() << "hightlight mode.......";
        pixmap = generatedHightlightPixmap(pixmap);

    }
    else{
        pixmap = generatedNormalPixmap(pixmap);
    }
    if(mode == Icon::Disabled || mode == Icon::HighDisabled){
        //        Icon *icon = new Icon();
        pixmap = generatedDisablePixmap(pixmap);
    }

    return pixmap;
}

QPixmap IconHelper::generatedDisablePixmap(QPixmap pixmap)
{
        QPixmap target = pixmap;

//        //Fix me:QT original code
        QImage im = target.toImage().convertToFormat(QImage::Format_ARGB32);

        for (int y = 0; y < im.height(); y++) {
            QRgb* scanLine = reinterpret_cast<QRgb*>(im.scanLine(y));
            for (int x = 0; x < im.width(); x++) {
                QRgb pixel = scanLine[x];//当前像素点 (r,g,b,a)
                int curAlpha = qAlpha(pixel);//0-255整数值
                scanLine[x] = qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), int(curAlpha * 0.45));//给当前像素点赋新值
            }
        }
        return QPixmap::fromImage(im);
        /*
        // Create a colortable based on the background (black -> bg -> white)
        QColor bg = qApp->palette().color(QPalette::Disabled, QPalette::ButtonText);
        int red = bg.red();
        int green = bg.green();
        int blue = bg.blue();
        uchar reds[256], greens[256], blues[256];
        for (int i=0; i<128; ++i) {
            reds[i]   = uchar((red   * (i<<1)) >> 8);
            greens[i] = uchar((green * (i<<1)) >> 8);
            blues[i]  = uchar((blue  * (i<<1)) >> 8);
        }
        for (int i=0; i<128; ++i) {
            reds[i+128]   = uchar(qMin(red   + (i << 1), 255));
            greens[i+128] = uchar(qMin(green + (i << 1), 255));
            blues[i+128]  = uchar(qMin(blue  + (i << 1), 255));
        }

        int intensity = (77 * red + 150 * green + 28 * blue) / 255;
       //qt_intensity(red, green, blue);
        const int factor = 191;

        // High intensity colors needs dark shifting in the color table, while
        // low intensity colors needs light shifting. This is to increase the
        // perceived contrast.
        if ((red - factor > green && red - factor > blue)
            || (green - factor > red && green - factor > blue)
            || (blue - factor > red && blue - factor > green))
            intensity = qMin(255, intensity + 91);
//        else if (intensity <= 128)
//            intensity -= 51;
        bool isPureColor= HighLightEffect::isPixmapPureColor(pixmap);

        for (int y=0; y<im.height(); ++y) {
            QRgb *scanLine = (QRgb*)im.scanLine(y);
            for (int x=0; x<im.width(); ++x) {
                QRgb pixel = *scanLine;
                // Calculate color table index, taking intensity adjustment
                // and a magic offset into account.
                uint ci = uint(qGray(pixel)/3 + (130 - intensity / 3));

                int r = int(reds[ci]);
                int g = int(greens[ci]);
                int b = int(blues[ci]);
                int a = qAlpha(pixel)/255.0 > 0.7 ? 0.7 * 255 : qAlpha(pixel);
                if(isPureColor){
                    r = qMax(int(reds[ci]), bg.red());
                    g = qMax(int(greens[ci]), bg.green());
                    b = qMax(int(blues[ci]), bg.blue());
                    a = qAlpha(pixel);
                }
                *scanLine = qRgba(r, g, b, a);
                ++scanLine;
            }
        }

        return QPixmap::fromImage(im);


        //Fix me:set same color to text when set icon mode disable.But it has error in color icons.

//        QColor bg = option->palette.color(QPalette::Disabled, QPalette::WindowText);
//        bg.setAlphaF(0.5);
//        QPainter p(&target);
//        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
//        p.fillRect(target.rect(), bg);
//        p.end();

//        return target;
        */

}

QPixmap IconHelper::generatedHightlightPixmap(QPixmap pixmap)
{
    QPixmap target = pixmap;
    QPainter p(&target);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    QColor color = qApp ? qApp->palette().color(QPalette::HighlightedText) : Qt::white;
    if(qApp && qApp->property("qqc2-globaltoken").isValid())
    {
        TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
        color = token->getInstance()->highLightedTextActive().color();
    }
    p.fillRect(target.rect(), color);
    return target;
}

QPixmap IconHelper::generatedNormalPixmap(QPixmap pixmap)
{
    if(qApp){
        QPixmap target = pixmap;
        QPainter p(&target);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        QColor color = qApp ? qApp->palette().color(QPalette::Highlight) : Qt::white;
        if(qApp && qApp->property("qqc2-globaltoken").isValid())
        {
            TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
            color = token->getInstance()->textActive().color();
        }
        p.fillRect(target.rect(), color);
        return target;
    }
    return pixmap;
}

QPixmap IconHelper::generatedWidthThemeColorPixmap(QPixmap pixmap,Icon::Mode mode)
{
    if(qApp){
        QPixmap target = pixmap;
        QPainter p(&target);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        QColor color = qApp ? qApp->palette().color(QPalette::Text) : Qt::white;
        if(qApp && qApp->property("qqc2-globaltoken").isValid())
        {
            TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
            if(mode == Icon::NormalWidthThemeColor)
                color = token->getInstance()->textActive().color();
            else if(mode == Icon::HoveredWidthThemeColor)
                color = token->getInstance()->kBrandHover().color();
            else if(mode == Icon::ClickedWidthThemeColor)
                color = token->getInstance()->kBrandClick().color();
            else if(mode == Icon::DisableWidthThemeColor)
                color = token->getInstance()->textDisable().color();
        }
        p.fillRect(target.rect(), color);
        return target;
    }
    return pixmap;
}

ImageProvider::ImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    ////qDebug() << "ImageProvider.............";
}

//requestPixmap函数的重写，在qml中使用source加载图片时会自动调用requestPixmap或requestImage函数（根据图片类型不同），返回QPixmap或QImage对象
QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    //qDebug() << "requestPixmap....." << id;
    int lastIndex = id.lastIndexOf("/");
    QString model, iconID = id;
    if(lastIndex != -1){
        iconID = id.left(lastIndex);
       model = id.right(id.length() - lastIndex - 1);
    }
    if(model != "normal" && model != "disenable" && model != "clicked" && model != "hover" && model != "highlight" && model != "highDisbale"
            && model != "hoveredWidthThemeColor" && model != "clickedWidthThemeColor" &&
            model != "normalWidthThemeColor" && model != "disableWidthThemeColor"){
        iconID = id;
        model = "";
    }
    ////qDebug() << "requestPixmap...." << model << iconID;


    Icon::Mode im = Icon::Mode::Normal;
    if(model == "disenable")
        im = Icon::Disabled;
    else if(model == "clicked")
        im = Icon::Selected;
    else if(model == "hover")
        im = Icon::Hovered;
    else if(model == "highlight")
        im = Icon::Highlight;
    else if(model == "highDisbale")
        im = Icon::HighDisabled;
    else if(model == "normalWidthThemeColor")
        im = Icon::NormalWidthThemeColor;
    else if(model == "hoveredWidthThemeColor")
        im = Icon::HoveredWidthThemeColor;
    else if(model == "clickedWidthThemeColor")
        im = Icon::ClickedWidthThemeColor;
    else if(model == "disableWidthThemeColor")
        im = Icon::DisableWidthThemeColor;

    QPixmap pixmap = IconHelper::generatePixMap(iconID, im, requestedSize, defaultSize);

    if(!requestedSize.isEmpty() && pixmap.size() != requestedSize)
        pixmap = pixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    else if(requestedSize.isEmpty())
        pixmap = pixmap.scaled(defaultSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (size) {
        QSize pixmapSize = pixmap.size();
        size->setWidth(pixmapSize.width());
        size->setHeight(pixmapSize.height());
    }

    return pixmap;
}

