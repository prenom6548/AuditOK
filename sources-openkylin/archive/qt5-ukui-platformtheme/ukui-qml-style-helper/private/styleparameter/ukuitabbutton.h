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

#ifndef UKUITABBUTTON_H
#define UKUITABBUTTON_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUITabButton : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QBrush normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)
    Q_PROPERTY(QBrush hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(QBrush checkedColor READ checkedColor WRITE setCheckedColor NOTIFY checkedColorChanged)
    Q_PROPERTY(QBrush clickedColor READ clickedColor WRITE setClickedColor NOTIFY clickedColorChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoverBorderColor READ hoverBorderColor WRITE setHoverBorderColor NOTIFY hoverBorderColorChanged)
    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickBorderColorChanged)
    Q_PROPERTY(QBrush checkedBorderColor READ checkedBorderColor WRITE setCheckedBorderColor NOTIFY checkedBorderColorChanged)

    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY disableTextColorChanged)

    Q_PROPERTY(QBrush normalTransparentColor READ normalTransparentColor WRITE setNormalTransparentColor NOTIFY normalTransparentColorChanged)
    Q_PROPERTY(QBrush hoverTransparentColor READ hoverTransparentColor WRITE setHoverTransparentColor NOTIFY hoverTransparentColorChanged)
    Q_PROPERTY(QBrush checkedTransparentColor READ checkedTransparentColor WRITE setCheckedTransparentColor NOTIFY checkedTransparentColorChanged)
    Q_PROPERTY(QBrush clickedTransparentColor READ clickedTransparentColor WRITE setClickedTransparentColor NOTIFY clickedTransparentColorChanged)
    Q_PROPERTY(QBrush focusBorderColor READ focusBorderColor WRITE setFocusBorderColor NOTIFY focusBorderColorChanged)

public:
    explicit UKUITabButton(QQuickItem *parent = nullptr);
    ~UKUITabButton();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUITabButton* qmlAttachedProperties(QObject* parent);

    const QBrush &normalColor() const;
    void setNormalColor(const QBrush &newNormalColor);

    const QBrush &hoverColor() const;
    void setHoverColor(const QBrush &newHoverColor);

    const QBrush &checkedColor() const;
    void setCheckedColor(const QBrush &newCheckedColor);

    const QBrush &clickedColor() const;
    void setClickedColor(const QBrush &newClickedColor);

    int borderWidth() const;
    void setBorderWidth(int newBorderWidth);

    const QBrush &normalBorderColor() const;
    void setNormalBorderColor(const QBrush &newNormalBorderColor);

    const QBrush &hoverBorderColor() const;
    void setHoverBorderColor(const QBrush &newHoverBorderColor);

    const QBrush &clickBorderColor() const;
    void setClickBorderColor(const QBrush &newClickBorderColor);

    const QBrush &checkedBorderColor() const;
    void setCheckedBorderColor(const QBrush &newCheckedBorderColor);

    const QBrush &normalTextColor() const;
    void setNormalTextColor(const QBrush &newNormalTextColor);

    const QBrush &normalTransparentColor() const;
    void setNormalTransparentColor(const QBrush &newNormalTransparentColor);

    const QBrush &hoverTransparentColor() const;
    void setHoverTransparentColor(const QBrush &newHoverTransparentColor);

    const QBrush &checkedTransparentColor() const;
    void setCheckedTransparentColor(const QBrush &newCheckedTransparentColor);

    const QBrush &clickedTransparentColor() const;
    void setClickedTransparentColor(const QBrush &newClickedTransparentColor);

    const QBrush &focusBorderColor() const;
    void setFocusBorderColor(const QBrush &newFocusBorderColor);

signals:
    void normalColorChanged();

    void hoverColorChanged();

    void checkedColorChanged();

    void clickedColorChanged();

    void borderWidthChanged();

    void normalBorderColorChanged();

    void hoverBorderColorChanged();

    void clickBorderColorChanged();

    void checkedBorderColorChanged();

    void disableTextColorChanged();

    void parametryChanged();

    void normalTransparentColorChanged();

    void hoverTransparentColorChanged();

    void checkedTransparentColorChanged();

    void clickedTransparentColorChanged();

    void focusBorderColorChanged();

private:
    Q_INVOKABLE QBrush m_normalColor = QBrush("#FFFFFF");
    Q_INVOKABLE QBrush m_hoverColor = QBrush("#F2F2F2");
    Q_INVOKABLE QBrush m_checkedColor = QBrush("#F2F2F2");
    Q_INVOKABLE QBrush m_clickedColor = QBrush("#EEEEEE");
    int m_borderWidth;
    Q_INVOKABLE QBrush m_normalBorderColor;
    Q_INVOKABLE QBrush m_hoverBorderColor;
    Q_INVOKABLE QBrush m_clickBorderColor;
    Q_INVOKABLE QBrush m_checkedBorderColor;
    Q_INVOKABLE QBrush m_normalTextColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_normalTransparentColor;
    QBrush m_hoverTransparentColor;
    QBrush m_checkedTransparentColor;
    QBrush m_clickedTransparentColor;
    QBrush m_focusBorderColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUITabButton, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUITABBUTTON_H
