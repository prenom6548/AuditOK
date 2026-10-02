/*
 * KWin Style UKUI
 *
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef SHADOWHELPER_H
#define SHADOWHELPER_H

#include <KDecoration2/DecorationShadow>
#include <QMap>
#include <QPixmap>
#include <QSharedPointer>

namespace UKUI
{

class Decoration;

class ShadowInfo
{
public:
    explicit ShadowInfo(const QColor &shadowColor, const QColor &borderColor, int topLeft, int topRight, int bottomLeft, int bottomRight, int borderWidth, QPoint shadowShift, bool ubr);
    bool operator==(const ShadowInfo &index);
    bool operator<(const ShadowInfo &index) const;

    QColor shadowColor() const;
    QColor borderColor() const;
    int topLeft() const;
    int topRight() const;
    int bottomLeft() const;
    int bottomRight() const;
    int borderWidth() const;
    QPoint shadowShift() const;
    bool ubr() const;
    QVector<int> uid() const; // 返回唯一标识阴影信息的数组

private:
    QColor m_shadowColor;
    QColor m_borderColor;
    int m_topLeft = -1;
    int m_topRight = -1;
    int m_bottomLeft = -1;
    int m_bottomRight = -1;
    int m_borderWidth = -1;
    QPoint m_shadowShift;
    bool m_ubr = false;
    QVector<int> m_uid; // 用于唯一标识阴影信息的数组
};

class ShadowHelper
{
    friend class Decoration;

public:
    static ShadowHelper *globalInstance();

    void releaseShadows();

    QSharedPointer<KDecoration2::DecorationShadow> getCreatedShadow(const ShadowInfo &info);

private:
    ShadowHelper();
    QPixmap getShadowPixmap(const QColor &shadowColor,
                            const QColor &borderColor,
                            int shadow_border,
                            int borderRadiusTopLeft,
                            int borderRadiusTopRight,
                            int borderRadiusBottomLeft,
                            int borderRadiusBottomRight,
                            QPoint shadow_shift,
                            bool ubrEnabled);

    QMap<ShadowInfo, QSharedPointer<KDecoration2::DecorationShadow>> m_shadowsCache;

    QPainterPath caculateRelativePainterPath(qreal borderRadiusTopLeft, qreal borderRadiusTopRight, qreal borderRadiusBottomLeft, qreal borderRadiusBottomRight, bool ubrEnabled, bool border);
};

}

#endif // SHADOWHELPER_H
