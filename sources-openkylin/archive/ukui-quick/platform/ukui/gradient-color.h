/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

#ifndef GRADIENTCOLOR_H
#define GRADIENTCOLOR_H
#include <QColor>
#include <QObject>
#include <QPoint>
namespace UkuiQuick {

class DtColorType
{
    Q_GADGET
public:
    enum Type {
       Pure = 0,
       Gradient
    };
    Q_ENUM(Type)
};

class GradientColor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor gradientStart READ gradientStart WRITE setGradientStart NOTIFY gradientStartChanged)
    Q_PROPERTY(QColor gradientEnd READ gradientEnd WRITE setGradientEnd NOTIFY gradientEndChanged)
    Q_PROPERTY(QColor gradientBackground READ gradientBackground WRITE setGradientBackground NOTIFY gradientBackgroundChanged)
    Q_PROPERTY(int gradientDeg READ gradientDeg WRITE setGradientDeg NOTIFY gradientDegChanged)
    Q_PROPERTY(QColor pureColor READ pureColor WRITE setPureColor NOTIFY pureColorChanged)
    Q_PROPERTY(UkuiQuick::DtColorType::Type colorType READ colorType WRITE setColorType NOTIFY colorTypeChanged)
public:
    GradientColor();
    GradientColor(QColor pure);
    GradientColor(int deg, QColor start, QColor end, QColor background) ;
    GradientColor(const GradientColor &color);
    QColor gradientStart() const;
    QColor gradientEnd() const;
    QColor gradientBackground() const;
    int gradientDeg() const;
    QColor pureColor() const;
    UkuiQuick::DtColorType::Type colorType() const;
    GradientColor &operator= (const GradientColor &color);
    bool operator!= (GradientColor *color);
    bool operator!= (QColor color);
    void setGradientStart(QColor start);
    void setGradientEnd(QColor end);
    void setGradientBackground(QColor background);
    void setGradientDeg(int deg);
    void setPureColor(QColor color);
    void setColorType(UkuiQuick::DtColorType::Type type);
    Q_INVOKABLE QColor setAlphaF(QColor color, qreal alpha);
    Q_INVOKABLE QColor mixBackGroundColor(const QColor &color, const QColor &back, const qreal &alpha);
    Q_INVOKABLE QPoint getGradientStartPoint(int deg, int width, int height);
    Q_INVOKABLE QPoint getGradientEndPoint(int deg, int width, int height);
Q_SIGNALS:
    void gradientStartChanged();
    void gradientEndChanged();
    void gradientBackgroundChanged();
    void gradientDegChanged();
    void pureColorChanged();
    void colorTypeChanged();
private:
    int m_gradientDeg;
    QColor m_gradientStart;
    QColor m_gradientEnd;
    QColor m_gradientBacrground;
    QColor m_pureColor;
    DtColorType::Type m_colorType;
};
}
Q_DECLARE_METATYPE(UkuiQuick::GradientColor)

#endif // GRADIENTCOLOR_H
