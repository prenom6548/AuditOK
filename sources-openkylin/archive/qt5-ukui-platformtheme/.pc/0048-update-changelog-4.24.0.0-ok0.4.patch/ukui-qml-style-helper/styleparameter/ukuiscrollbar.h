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

#ifndef UKUISCROLLBAR_H
#define UKUISCROLLBAR_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIScrollBar : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QBrush normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)
    Q_PROPERTY(QBrush hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(QBrush clickColor READ clickColor WRITE setClickColor NOTIFY clickColorChanged)
    Q_PROPERTY(QBrush disableColor READ disableColor WRITE setDisableColor NOTIFY disableColorChanged)

    Q_PROPERTY(QBrush normalTransparentColor READ normalTransparentColor WRITE setNormalTransparentColor NOTIFY normalTransparentColorChanged)
    Q_PROPERTY(QBrush hoverTransparentColor READ hoverTransparentColor WRITE setHoverTransparentColor NOTIFY hoverTransparentColorChanged)
    Q_PROPERTY(QBrush clickTransparentColor READ clickTransparentColor WRITE setClickTransparentColor NOTIFY clickTransparentColorChanged)
    Q_PROPERTY(QBrush disableTransparentColor READ disableTransparentColor WRITE setDisableTransparentColor NOTIFY disableTransparentColorChanged)

    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)

public:
    explicit UKUIScrollBar(QQuickItem *parent = nullptr);
    ~UKUIScrollBar();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUIScrollBar* qmlAttachedProperties(QObject* parent);

    const QBrush &normalColor() const;
    void setNormalColor(const QBrush &newNormalColor);

    int padding() const;
    void setPadding(int newPadding);

    const QBrush &hoverColor() const;
    void setHoverColor(const QBrush &newHoverColor);

    const QBrush &clickColor() const;
    void setClickColor(const QBrush &newClickColor);

    const QBrush &disableColor() const;
    void setDisableColor(const QBrush &newDisableColor);

    const QBrush &normalTransparentColor() const;
    void setNormalTransparentColor(const QBrush &newNormalTransparentColor);

    const QBrush &hoverTransparentColor() const;
    void setHoverTransparentColor(const QBrush &newHoverTransparentColor);

    const QBrush &clickTransparentColor() const;
    void setClickTransparentColor(const QBrush &newClickTransparentColor);

    const QBrush &disableTransparentColor() const;

    void setDisableTransparentColor(const QBrush &newDisableTransparentColor);

signals:
    void normalColorChanged();

    void paddingChanged();

    void hoverColorChanged();

    void clickColorChanged();

    void disableColorChanged();

    void parametryChanged();

    void normalTransparentColorChanged();

    void hoverTransparentColorChanged();

    void clickTransparentColorChanged();

    void disableTransparentColorChanged();

private:
    Q_INVOKABLE QBrush m_normalColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));

    int m_padding = 2;
    Q_INVOKABLE QBrush m_hoverColor;
    Q_INVOKABLE QBrush m_clickColor;
    Q_INVOKABLE QBrush m_disableColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_normalTransparentColor;
    QBrush m_hoverTransparentColor;
    QBrush m_clickTransparentColor;
    QBrush m_disableTransparentColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIScrollBar, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUISCROLLBAR_H
