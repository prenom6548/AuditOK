/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
#ifndef GRADIENTBORDERITEM_H
#define GRADIENTBORDERITEM_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QPropertyAnimation>
#include "./ukui/dt-theme.h"
namespace UkuiQuick {
class GradientBorderItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(qreal borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(qreal deg READ deg WRITE setDeg NOTIFY degChanged)
    Q_PROPERTY(bool enableSpecificBorder READ enableSpecificBorder WRITE setEnableSpecificBorder NOTIFY enableSpecificBorderChanged)
    Q_PROPERTY(GradientColor* borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
public:
    explicit GradientBorderItem(QQuickItem *parent = nullptr);

    qreal borderWidth() const {return m_borderWidth;}
    qreal borderRadius() const {return m_borderRadius;}
    qreal deg() const {return m_deg;}
    bool enableSpecificBorder() const {return m_enableSpecificBorder;}
    GradientColor* borderColor() const {return m_borderColor;}
public Q_SLOTS:
    void setBorderWidth(qreal width);
    void setBorderRadius(qreal radius);
    void setDeg(qreal deg);
    void setEnableSpecificBorder(bool enable);
    void setBorderColor(GradientColor* color);
private:
    void startCurveAnimation(bool start);
Q_SIGNALS:
    void borderWidthChanged();
    void borderRadiusChanged();
    void degChanged();
    void enableSpecificBorderChanged();
    void borderColorChanged();
protected:
    void paint(QPainter *painter) override;
    void componentComplete() override;
private:
    qreal m_borderWidth = 2.0;
    qreal m_borderRadius = 6.0;
    //开启贝塞尔曲线动画后描边的渐变色角度
    qreal m_deg = 0;
    //是否开启特殊的（非DT色）渐变色描边
    bool m_enableSpecificBorder = false;
    //未开启贝塞尔曲线动画时的描边颜色
    GradientColor* m_borderColor = nullptr;
    //控制描边渐变色角度的动画
    QPropertyAnimation *m_animation = nullptr;
};
}
#endif // GRADIENTBORDERITEM_H
