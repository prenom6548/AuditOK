#include "parsecolorinterface.h"
#include "qdebug.h"
#include "qpixmap.h"
#include "qimage.h"
#include <QFile>
#include <QSize>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <qmath.h>

using namespace UKUIQQC2Style;


ParseColorInterface::ParseColorInterface(QObject *parent):QObject(parent)
{

}

ParseColorInterface* ParseColorInterface::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QObject*>(parent);
    return new ParseColorInterface(p);
}


bool ParseColorInterface::isSolidPattern(QVariant dtColor)
{
    if(dtColor.canConvert<QBrush>()){
        QBrush c = dtColor.value<QBrush>();
        if(c.style() == Qt::SolidPattern || c.style() == Qt::NoBrush)
            return true;
        if(c.style() == Qt::LinearGradientPattern && c.gradient() && c.gradient()->stops().length() == 2){
            if(c.gradient()->stops().at(0).second != c.gradient()->stops().at(1).second)
                return false;
        }
        return true;
    }
    return false;
}

QColor ParseColorInterface::startColor(QVariant dtColor)
{
    if(dtColor.canConvert<QBrush>()){
        QBrush c = dtColor.value<QBrush>();
        if(c.gradient() && c.gradient()->stops().length() >= 1)
            return c.gradient()->stops().at(0).second;
        else
            return c.color();
    }
    else if(dtColor.canConvert<QColor>())
        return dtColor.value<QColor>();
    return QColor();
}

QColor ParseColorInterface::endColor(QVariant dtColor)
{
    if(dtColor.canConvert<QBrush>()){
        QBrush c = dtColor.value<QBrush>();
        if(c.gradient() && c.gradient()->stops().length() == 2)
            return c.gradient()->stops().at(1).second;
        else
            return c.color();
    }
    else if(dtColor.canConvert<QColor>())
        return dtColor.value<QColor>();
    return QColor();
}

QString ParseColorInterface::endString(QString s, QString lastS)
{
    int lastIndex = s.lastIndexOf(lastS);
    QString returnS;
    if(lastIndex != -1){
       returnS = s.right(s.length() - lastIndex - 1);
    }
    return returnS;
}

QColor ParseColorInterface::getDtColor(QVariant dtColorkey)
{
    QString key = dtColorkey.toString();
    const char* c= key.toLocal8Bit().constData();
    if (!key.isEmpty() && qApp && qApp->property(c).isValid()){
        QBrush b = qApp->property(c).value<QBrush>();
        return b.color();
    }
    return QColor();
}
