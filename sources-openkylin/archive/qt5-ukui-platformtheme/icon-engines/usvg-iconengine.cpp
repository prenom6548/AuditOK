/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2025, KylinSoft Co., Ltd.
** 
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/



#include"usvg-iconengine.h"

#include <QGuiApplication>
#include <QMimeDatabase>
#include <QScreen>
#include <QWindow>
#include <QHash>
#include <QPair>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include <QtSvg/QSvgRenderer>
#include <QPixmapCache>
#include <QPainter>
#include <QtXml/QDomElement>

#include <QString>
#include <sstream>
#include <type_traits>



/*
    一些辅助结构、函数，非类相关
    static或者inline
*/

// 日志相关
#define ENABLE_LOGS 0

#if ENABLE_LOGS

template<typename T>
void appendToStream(std::ostringstream& oss, const T& value) 
{
    oss << value;
}

// QString 特化
void appendToStream(std::ostringstream& oss, const QString& value) 
{
    oss << value.toStdString();
}

// const char* 特化
void appendToStream(std::ostringstream& oss, const char* value) 
{
    oss << value;
}
//用于输出 QPixmap信息，特化
void appendToStream(std::ostringstream& oss,const QPixmap& pm)
{
    oss << "[QPixmap " << pm.width() << "*" << pm.height() << "]";
}

void logsHelper(std::ostringstream& oss)//递归终止时的情况下，logsHelper后面 可变参数空，此函数会被调用
{
    qDebug().noquote() << QString::fromStdString(oss.str());
}

template<typename T, typename... Args>
void logsHelper(std::ostringstream& oss, const T& first, const Args&... params) 
{
    appendToStream(oss, first);
    logsHelper(oss, params...);
}

//日志主函数的，接受可变参数
template<typename... Args>
void logs(const Args&... args)
{
    std::ostringstream oss;
    logsHelper(oss, args...);
}

#else
template<typename... Args>
void logs(const Args&...)
{
}

#endif


//数据结构
typedef QPair<QIcon::Mode, QIcon::State> MSKeyPair;

struct MatchedEntry //用于存储匹配结果
{
    QString svgPath;
    QIcon::Mode mode;
    QIcon::State state;

    // void addFile(const QString &fileName, const QSize &size = QSize(), Mode mode = Normal, State state = Off);
    MatchedEntry()
    {
       svgPath= QString();
       mode = QIcon::Normal;
       state = QIcon::Off;
    }

    void store(QString svgPath,QIcon::Mode mode,QIcon::State state)
    {
        this->svgPath = svgPath;
        this->mode = mode;
        this->state = state;
    }
};

QDebug operator<<(QDebug debug,const MatchedEntry& entry)
{
    debug.nospace() << "MatchedEntry("
                    << "svgPath: " << entry.svgPath << ","
                    << "mode: " << entry.mode << ","
                    << "state: " << entry.state << ")";
    return debug.space();
}



/**
 * 基于DOM操作, 修改svg的颜色
 * elem:DOM树的一个元素句柄   targetTag：标签名  attr:属性名    attrValue:属性值
 */
inline void setElemAttr(QDomElement& elem,QString targetTag,QString attr,QString attrValue)
{
    qDebug() << "setElemAttr: 当前 elem.tagName = " << elem.tagName();
    qDebug() << "                               elem.Attr = " << attr;
    //找到了目标标签，则直接修改属性对应的属性值（这个场景下，属性值就是我们需要改变的目标颜色）
    if(elem.tagName() == targetTag)
    {
        elem.setAttribute(attr,attrValue);
    }
    for(int i=0;i<elem.childNodes().count();i++)
    {
        if(!elem.childNodes().at(i).isElement())
        {
            continue;
        }
        QDomElement childElem = elem.childNodes().at(i).toElement();
        setElemAttr(childElem,targetTag,attr,attrValue);
    }
}

//RGB颜色转换为16进制
//因为svg的xml中描述颜色是用 #D32520 这种16进制的表达，而Qt中一般用 QColor
inline QString rgb2HexStr(const QColor& color)
{
    QString hexStr;
    QString redStr = QString::number(color.red(),16).rightJustified(2,'0');
    QString greenStr = QString::number(color.green(),16).rightJustified(2,'0');
    QString blueStr = QString::number(color.blue(),16).rightJustified(2,'0');
    hexStr.append("#").append(redStr).append(greenStr).append(blueStr);
    return hexStr;
//    return color.name(QColor::HexRgb);//Qt有处理这个的API
}

//把一张svg图片渲染为指定颜色，并转化为一个QPimxmap，并基于此生成一个QIcon返回
inline QIcon loadSvgAsIcon(const QString& svgPath,const QSize& targetPixmapIconSize,const QColor& targetColor)
{
    //加载该svg，得到byteArray数据，基于该byteArray构造QDomDocument，基于该DomDocument获取QDomElement，修改QDomElement
    QFile file(svgPath);
    file.open(QIODevice::ReadOnly);
    QByteArray bytes = file.readAll();

    QDomDocument document;
    document.setContent(bytes);
    file.close();

    QDomElement rootElem = document.documentElement();
    setElemAttr(rootElem,"path","fill",rgb2HexStr(targetColor));

    //将修改完毕的QDomElement，用于QSvgRenderer渲染
    QSvgRenderer renderer(document.toByteArray());
    if(!renderer.isValid())
    {
        return QIcon();
    }
    //转化为pixmap，作为QIcon返回
    QPixmap pm(targetPixmapIconSize);
    qDebug() << "pm.size = " << pm.size();
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    renderer.render(&painter);

    return QIcon(pm);
}




//日志输出到桌面
// inline void logToDesktop(const QString& str)
// {
//     // const QString filepath = "/home/xxx/桌面/engine_logs.txt";
//     const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
//     const QString fileName = "engine_logs.txt";
//     const QString filepath = desktopPath + "/" + fileName;
//     QFile file(filepath);
//     if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
//     {
//         QTextStream stream(&file);
//         stream << str << "," << "appName = " << qAppName() << "\n";
//         file.close();
//     }
//     else
//     {
//         fprintf(stderr,"打开日志文件失败:%s\n",filepath.toLocal8Bit().constData());
//     }
// }

// 用于数据存储的key
//Mode:0~3,State:0~1
inline int MSKey(QIcon::Mode mode, QIcon::State state)
{
    // qDebug() << "[inline int MSKey]: 生成 MS key = "  << (mode*2+state);
    return mode*2 + state;
}

inline QString pixmapCacheKey(const QString& svgPath,const QSize& size, QIcon::Mode mode, QIcon::State state)
{
    return QStringLiteral("pixmapCache_key_%1_%2_%3_%4_%5")
//            .arg(qHash(svgPath))
            .arg(svgPath)
            .arg(size.width())
            .arg(size.height())
            .arg(static_cast<int>(mode))
            .arg(static_cast<int>(state));
}
inline QString pixmapCacheKey(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
    return QStringLiteral("pixmapCache_key_%1_%2_%3_%4")
            .arg(size.width())
            .arg(size.height())
            .arg(static_cast<int>(mode))
            .arg(static_cast<int>(state));
}


enum class IconFileType //输入图标文件的类型
{
    svg,   // mimeType = "image/svg+xml"
    png,   // mimeType = "image/png"
    jpeg,  // mimeType = "image/jpeg"
    other
};


//基于MimeDatabase检测图标文件的类型
inline IconFileType probeIconMimeType(const QString& filePath)
{
    QMimeDatabase db;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.read(1024); // 读取文件的前1024个字节用于MIME类型检测
        file.close();
        QString mimeStr = db.mimeTypeForData(data).name();
        if("image/svg+xml" == mimeStr)
        {
            return IconFileType::svg;
        }
        else if("image/png" == mimeStr)
        {
            return IconFileType::png;
        }
        else if("image/jpeg" == mimeStr)
        {
            return IconFileType::jpeg;
        }
    }
    return IconFileType::other;
}

// inline bool isValidSvg(const QString& filepath)
// {
//     if(filepath.isEmpty() || (IconFileType::svg) != probeIconMimeType(filepath))//确保是svg类型的图片
//     {
//         return false;
//     }
//     QSvgRenderer tmpRender(filepath);
//     return tmpRender.isValid();
// }
//加载图片为Qpixmap
inline bool loadFileToPixmap(const QString& filePath,QPixmap& pix)
{
    bool ret = pix.load(filePath);
    if(!ret)
    {
        logs("从以下路径中load QPixmap失败: ",filePath);
        return false;
    }
    return ret;
}

inline QString getAbsPath(const QString& filepath)
{
    QFileInfo fi(filepath);
    return fi.absoluteFilePath();
}

inline qreal get_dpr()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpr = screen->devicePixelRatio();
    return dpr; 
}



/**
    Private类
*/
class USvgIconEnginePrivate
{
public:
    /**
     *  容器: pixmapCache
     *  主要应对在用户层面使用 addPixmap()这种额外接口添加的QPixmap(用户可能对于svg，直接转化为QPixmap，调用QIcon::addPixmap，进行传入
     *  QIcon::addPixmap最终调用到IconEngine::addPixmap
     *  addPixmap仅有图片对象的参数 mode、state，因此key只能包含这2个因素
     *  key: 当前svg图片的 mode，state 转化成的int key
     *  value: 当前svg图片，以mode，state状态转化为的 QPixmap
     */

    QHash<int,QPixmap>* pixmapCache;
    QHash<int,QString>* svgCache;
    QHash<MSKeyPair,QPixmap>* svgSerializedPixmap;//用于序列化，key不能用hash生成key，因为不可逆，反序列化时无法解析出mode，state

public:
    USvgIconEnginePrivate()
    {
        pixmapCache = new QHash<int,QPixmap>;
        svgCache = new QHash<int,QString>;
        svgSerializedPixmap = new QHash<MSKeyPair,QPixmap>;
    }
    ~USvgIconEnginePrivate()
    {
        if(pixmapCache)
        {
            pixmapCache->clear();
            delete pixmapCache;
            pixmapCache = nullptr;
        }
        if(svgCache)
        {
            svgCache->clear();
            delete svgCache;
            svgCache = nullptr;
        }
        if(svgSerializedPixmap)
        {
            svgSerializedPixmap->clear();
            delete svgSerializedPixmap;
            svgSerializedPixmap = nullptr;
        }
    }

    bool matched(QIcon::Mode mode, QIcon::State state)
    {
        return svgCache->contains(MSKey(mode,state));
    }

    bool svgMatch(QIcon::Mode mode, QIcon::State state,MatchedEntry* entry)
    {
        if(matched(mode,state))//对于当前mode，state，直接匹配成功，数据存入MatchedEntry
        {
            entry->store(svgCache->value(MSKey(mode,state)),mode,state);
            return true;
        }

        //匹配失败，执行回退策略
        //fallback();
        QIcon::State oppositeState = (state == QIcon::On) ? QIcon::Off : QIcon::On;
        if (mode == QIcon::Disabled || mode == QIcon::Selected)//Disabled、Selected
        {
            QIcon::Mode oppositeMode = (mode == QIcon::Disabled) ? QIcon::Selected : QIcon::Disabled;
            if(matched(QIcon::Normal,state))//若回退的 Normal可以匹配成功，则直接返回 Normal
            {
                entry->store(svgCache->value(MSKey(QIcon::Normal,state)),QIcon::Normal,state);
                return false;
            }
            if(matched(QIcon::Active,state))//若回退的 Active 可以匹配成功，则直接返回 Active
            {
                entry->store(svgCache->value(MSKey(QIcon::Active,state)),QIcon::Active,state);
                return false;
            }

            if(matched(mode,oppositeState))//若以上不能匹配，尝试切换state
            {
                entry->store(svgCache->value(MSKey(mode,oppositeState)),mode,oppositeState);
                return false;
            }
            if(matched(QIcon::Normal,oppositeState))
            {
                entry->store(svgCache->value(MSKey(QIcon::Normal,oppositeState)),QIcon::Normal,oppositeState);
                return false;
            }
            if(matched(QIcon::Active,oppositeState))
            {
                entry->store(svgCache->value(MSKey(QIcon::Active,oppositeState)),QIcon::Active,oppositeState);
                return false;
            }
            if(matched(oppositeMode,state))//若以上不能匹配，再尝试切换mode
            {
                entry->store(svgCache->value(MSKey(oppositeMode,state)),oppositeMode,state);
                return false;
            }
            if(matched(oppositeMode,oppositeState))//尝试mode，state都切换
            {
                entry->store(svgCache->value(MSKey(oppositeMode,oppositeState)),oppositeMode,oppositeState);
                return false;
            }
        }
        else // Normal、Active
        {
            QIcon::Mode oppositeMode = (mode == QIcon::Normal) ? QIcon::Active : QIcon::Normal;
            if(matched(oppositeMode, state))
            {
                entry->store(svgCache->value(MSKey(oppositeMode,state)),oppositeMode,state);
                return false;
            }
            if (matched(mode, oppositeState))
            {
                entry->store(svgCache->value(MSKey(mode,oppositeState)),mode,oppositeState);
                return false;
            }
            if (matched(oppositeMode, oppositeState))
            {
                entry->store(svgCache->value(MSKey(oppositeMode,oppositeState)),oppositeMode,oppositeState);
                return false;
            }
            if(matched(QIcon::Disabled, state))
            {
                entry->store(svgCache->value(MSKey(QIcon::Disabled, state)),QIcon::Disabled, state);
                return false;
            }
            if(matched(QIcon::Selected, state))
            {
                entry->store(svgCache->value(MSKey(QIcon::Selected, state)),QIcon::Selected, state);
                return false;
            }
            if(matched(QIcon::Disabled, oppositeState))
            {
                entry->store(svgCache->value(MSKey(QIcon::Disabled, oppositeState)),QIcon::Disabled, oppositeState);
                return false;
            }
            if(matched(QIcon::Selected, oppositeState))
            {
                entry->store(svgCache->value(MSKey(QIcon::Selected, oppositeState)),QIcon::Selected, oppositeState);
                return false;
            }
        }
        entry->store(svgCache->value(MSKey(QIcon::Normal, QIcon::Off)),QIcon::Normal, QIcon::Off);
        return false;
    }

    QPixmap renderSvgWithSize(QSvgRenderer* renderer,QIcon::Mode mode,QIcon::State state,const QSize& targetSize,MatchedEntry* entry)
    {
        logs("renderSvgWithSize()");
        if(!svgCache || !renderer)
        {
            return QPixmap();
        }

        bool match = svgMatch(mode,state,entry);

        qreal dpr = 1.0;
        if (QWindow *window = qApp->focusWindow())
        {
            dpr = window->devicePixelRatio();
        }
        else
        {
            dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
        }

        renderer->load(entry->svgPath);//这里逻辑是不是有问题？应该load 当前图片baseSvg吧？ -------不不，没有错，就是应该load这个 fallback得到的最终path---------------

        if(!renderer->isValid())
        {
            return QPixmap();
        }

       // 使用QImage作为中间转换步骤
        QPixmap pixmp;

        QSize retSize = renderer->defaultSize();//获取renderer渲染的svg document的默认真实size，
        if(!retSize.isNull())
        {
            QSize tmpSize = retSize;
            retSize.scale(targetSize,Qt::KeepAspectRatio);//保持长宽比例放大,这里可能与、有个问题,scale缩放可能导致
        }
        if(retSize.isEmpty())//isEmpty(): width,height 任一<=0: true
        {
            return pixmp;
        }

        QImage tmpImg(retSize, QImage::Format_ARGB32_Premultiplied);//使用QImage转化为QPixmap

        // tmpImg.setDevicePixelRatio(dpr);//会导致关联的QPainter缩放
        tmpImg.fill(Qt::transparent);
        QPainter painter(&tmpImg);

        //painter.setRenderHint(QPainter::Antialiasing);
        renderer->render(&painter);//将svg渲染到QPainter上

        pixmp = QPixmap::fromImage(tmpImg);
        pixmp.setDevicePixelRatio(dpr);
        return pixmp;

    //    直接渲染到QPixmap上，效果一致
    //    QPixmap pix(targetSize);
    //    pix.fill(Qt::transparent);

    //    QPainter painter(&pix);
    //    renderer->render(&painter);

    //    painter.end();
    //    pix.setDevicePixelRatio(dpr);//必须放这里，不然图标会溢出边界

    //    return pix;
    }
};


/** 
    主类  USvgIconEngine
*/

USvgIconEngine::USvgIconEngine()
   : d_ptr(new USvgIconEnginePrivate)
{
}

//拷贝构造
USvgIconEngine::USvgIconEngine(const USvgIconEngine& other)
   : QIconEngine(other), d_ptr(new USvgIconEnginePrivate)
{
    logs("[USvgIconEngine 拷贝构造函数]");
    if(d_ptr->pixmapCache)
    {
        d_ptr->pixmapCache = new QHash<int,QPixmap>(*(other.d_ptr->pixmapCache));//指针深拷贝
    }
    if(d_ptr->svgCache)
    {
        d_ptr->svgCache = new QHash<int,QString>(*(other.d_ptr->svgCache));
    }
    if(d_ptr->svgSerializedPixmap)
    {
        d_ptr->svgSerializedPixmap = new QHash<MSKeyPair,QPixmap>(*(other.d_ptr->svgSerializedPixmap));
    }
}

USvgIconEngine::~USvgIconEngine()
{
    delete d_ptr;
}

QSize USvgIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    logs("USvgIconEngine::actualSize()");
    // return QIconEngine::actualSize(size,mode,state);
    QPixmap pixmp = pixmap(size,mode,state);
    if(!pixmp.isNull())
    {
        return pixmp.size();
    }
    return QSize();
}

QPixmap USvgIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    logs("USvgIconEngine::pixmap()");
    qreal dpr = get_dpr();

    //先检查Qpixmap的存储
    if (d_ptr->pixmapCache)
    {
        QPixmap tmpix = d_ptr->pixmapCache->value(MSKey(mode,state));
        if(!tmpix.isNull() && tmpix.size() == size)
        {
            logs("pixmapCache找到, tmpix = ",tmpix);
            return tmpix;
        }
    }

    bool match = d_ptr->matched(mode,state);
    if(match)//说明对于当前QIcon对象，用户确实通过addFile设置过一张mode，state的svg，则拿到对应svg Path后，检查QPixmapCache
    {
        //根据当前mode，state获取对应svg的path，看能否找到，但凡addFile了的，肯定能找到
        QString curSvgAbsPath = d_ptr->svgCache->value(MSKey(mode,state));

        const QString pcKey = pixmapCacheKey(curSvgAbsPath,size,mode,state);
        logs("pcKey = ",pcKey);

        logs("检查QPixmapCache...");
        QPixmap tmpix;

        if(QPixmapCache::find(pcKey,&tmpix))
        {
            logs("QPixmapCache 找到, tmpix = ",tmpix);
            return tmpix;
        }
    }
    else//Qt应用程序，没有设置一张基于所需mode，state的 svg图片
    {
        //pass ... fallback处理
    }

    QPixmap pixmp;
    QSvgRenderer renderer;
    MatchedEntry entry;

    pixmp = d_ptr->renderSvgWithSize(&renderer,mode,state,size,&entry);

    const QString endKey = pixmapCacheKey(entry.svgPath,size,mode,state);

    if(!pixmp.isNull())
    {
        QPixmapCache::insert(endKey,pixmp);
    }

    logs("插入缓存中 QPixmapCache::insert: endKey = ",endKey,",pixmp = ",pixmp);
    return pixmp;
}


void USvgIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
    logs("USvgIconEngine::addPixmap()");

    int key = MSKey(mode,state);
    d_ptr->pixmapCache->insert(key,pixmap);
}

void USvgIconEngine::addFile(const QString &fileName, const QSize &, QIcon::Mode mode, QIcon::State state)
{
    logs("USvgIconEngine::addFile()");

    if(fileName.isEmpty())
    {
        return ;
    }

    const QString absPath = getAbsPath(fileName);//图片的绝对路径，当前程序作为插件加载，需要拿到图片的绝对路径

    //判断传入图片的类型
    if(probeIconMimeType(fileName) == IconFileType::svg)//若是svg类型图片
    {
        // if(!absPath.isEmpty()) {} 

        d_ptr->svgCache->insert(MSKey(mode,state),absPath);//若存在该key，则会覆盖的
        int k = MSKey(mode,state);
        logs("addFile::svgCache->insert: [k:",k,",v:",absPath,"]");
    }
    else//其他类型，例如png，直接存储为QPixmap
    {
        QPixmap tmp;
        if(loadFileToPixmap(absPath,tmp))
        {
            d_ptr->pixmapCache->insert(MSKey(mode,state),tmp);
        }
    }
}
void USvgIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    logs("USvgIconEngine::paint()");
    qreal dpr = 1.0;//缩放比
    auto paintDevice = painter->device();
    if (QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps)) //如果应用程序开启了 高DPI适配，那么实际绘制pixmap的大小应该是size*dpr
    {
        dpr = paintDevice ? paintDevice->devicePixelRatioF() : qApp->devicePixelRatio();//获取绘制目标设备的设备像素比
    }
    QPixmap pm = pixmap(dpr * rect.size(),mode,state);
    painter->drawPixmap(rect,pm);//这个绘制区域 rect需要根据dpr进行处理吗？还是Qt年内部会默认放大之类的？
}

// 用于标记当前IconEngine的key
QString USvgIconEngine::key() const
{
   logs("USvgIconEngine::key()");
   return QStringLiteral("USvgIconEngine");
}
QIconEngine *USvgIconEngine::clone() const
{
    logs("USvgIconEngine::clone()");
    return new USvgIconEngine(*this);
}

//反序列号：从流中读取数据
bool USvgIconEngine::read(QDataStream &in)
{
    logs("USvgIconEngine::read()");
    quint32 mpSize;
    in >> mpSize;
    for(auto i=0;i<mpSize;i++)
    {
        quint32 curMode,curState;
        in >> curMode >> curState;

        QImage image;
        in >> image;

        QPixmap pixmap = QPixmap::fromImage(image);
        //转换为原始的类型，存入map
        QIcon::Mode mode = static_cast<QIcon::Mode>(curMode);
        QIcon::State state = static_cast<QIcon::State>(curState);
        d_ptr->svgSerializedPixmap->insert(qMakePair(mode,state),pixmap);
    }
    return true;
}

//序列化：把数据写入流
bool USvgIconEngine::write(QDataStream &out) const
{
    logs("USvgIconEngine::write()");
    out << static_cast<quint32>(d_ptr->svgSerializedPixmap->size());//使用quint32固定类型的，适应于不同平台
    for(const auto& key:d_ptr->svgSerializedPixmap->keys()) //QHash<MSKeyPair,QPixmap>
    {
        const MSKeyPair& pair = key;
        const QPixmap& mp = d_ptr->svgSerializedPixmap->value(pair);

        out << static_cast<quint32>(pair.first) << static_cast<quint32>(pair.second);
        out << mp.toImage();//使用QImage，兼容性
    }
    return true;
}
void USvgIconEngine::virtual_hook(int id, void *data)
{
    logs("USvgIconEngine::virtual_hook()");
    QIconEngine::virtual_hook(id, data);
}

