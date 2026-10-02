/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef UKUIPROGRESSBAR_H
#define UKUIPROGRESSBAR_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIProgressBar : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QBrush normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)
    Q_PROPERTY(QBrush childrenColor READ childrenColor WRITE setChildrenColor NOTIFY childrenColorChanged)
    Q_PROPERTY(int normalWidth READ normalWidth WRITE setNormalWidth NOTIFY normalWidthChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setNormalHeight NOTIFY normalHeightChanged)
    Q_PROPERTY(int indeterminateChildrenWidth READ indeterminateChildrenWidth  WRITE setIndeterminateChildrenWidth  NOTIFY indeterminateChildrenWidthChanged)
    Q_PROPERTY(QBrush indeterminateChildrenStartColor READ indeterminateChildrenStartColor  WRITE setIndeterminateChildrenStartColor  NOTIFY indeterminateChildrenStartColorChanged)
    Q_PROPERTY(QBrush indeterminateChildrenEndColor READ indeterminateChildrenEndColor  WRITE setIndeterminateChildrenEndColor  NOTIFY indeterminateChildrenEndColorChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QBrush borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)

    Q_PROPERTY(QBrush hightlightTextColor READ hightlightTextColor WRITE setHightlightTextColor NOTIFY hightlightTextColorChanged)

    Q_PROPERTY(QBrush normalTransparentColor READ normalTransparentColor WRITE setNormalTransparentColor NOTIFY normalTransparentColorChanged)
    Q_PROPERTY(QBrush transparentborderColor READ transparentborderColor WRITE setTransparentBorderColor NOTIFY transparentBorderColorChanged)

    Q_PROPERTY(QBrush indeterminateColor READ indeterminateColor WRITE setIndeterminateColor NOTIFY indeterminateColorChanged)
public:
    explicit UKUIProgressBar(QQuickItem *parent = nullptr);
    ~UKUIProgressBar();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUIProgressBar* qmlAttachedProperties(QObject* parent);

    const QBrush &normalColor() const;
    void setNormalColor(const QBrush &newNormalColor);

    int radius() const;
    void setRadius(int newRadius);

    const QBrush &childrenColor() const;
    void setChildrenColor(const QBrush &newChildrenColor);

    int normalWidth() const;
    void setNormalWidth(int newNormalWidth);

    int normalHeight() const;
    void setNormalHeight(int newNormalHeight);

    int indeterminateChildrenWidth() const;
    void setIndeterminateChildrenWidth(int newIndeterminateChildrenWidth);

    const QBrush &indeterminateChildrenStartColor() const;
    void setIndeterminateChildrenStartColor(const QBrush &newIndeterminateChildrenStartColor);

    const QBrush &indeterminateChildrenEndColor() const;
    void setIndeterminateChildrenEndColor(const QBrush &newIndeterminateChildrenEndColor);

    int borderWidth() const;
    void setBorderWidth(int newBorderWidth);

    const QBrush &borderColor() const;
    void setBorderColor(const QBrush &newBorderColor);

    const QBrush &hightlightTextColor() const;
    void setHightlightTextColor(const QBrush &newHightlightTextColor);

    const QBrush &normalTransparentColor() const;
    void setNormalTransparentColor(const QBrush &newNormalTransparentColor);

    const QBrush &transparentborderColor() const;
    void setTransparentBorderColor(const QBrush &newTransparentborderColor);

    const QBrush &indeterminateColor() const;
    void setIndeterminateColor(const QBrush &newIndeterminateColor);

    int calculateHighColor(int c);

signals:
    void normalColorChanged();

    void radiusChanged();

    void childrenColorChanged();

    void normalWidthChanged();

    void normalHeightChanged();

    void indeterminateChildrenWidthChanged();

    void indeterminateChildrenStartColorChanged();

    void indeterminateChildrenEndColorChanged();

    void borderWidthChanged();

    void borderColorChanged();

    void parametryChanged();

    void hightlightTextColorChanged();

    void normalTransparentColorChanged();

    void transparentBorderColorChanged();

    void indeterminateColorChanged();

private:
    Q_INVOKABLE QBrush m_normalColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));

    int m_radius;
    Q_INVOKABLE QBrush m_childrenColor;
    int m_normalWidth;
    int m_normalHeight;
    int m_indeterminateChildrenWidth;
    Q_INVOKABLE QBrush m_indeterminateChildrenStartColor;
    Q_INVOKABLE QBrush m_indeterminateChildrenEndColor;
    int m_borderWidth;
    Q_INVOKABLE QBrush m_borderColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_hightlightTextColor;
    QBrush m_normalTransparentColor;
    QBrush m_transparentborderColor;
    QBrush m_indeterminateColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIProgressBar, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUIPROGRESSBAR_H
