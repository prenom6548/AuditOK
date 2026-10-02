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

#ifndef UKUISWITCH_H
#define UKUISWITCH_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUISwitch : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QBrush normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)
    Q_PROPERTY(QBrush hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(QBrush clickColor READ clickColor WRITE setClickColor NOTIFY clickColorChanged)
    Q_PROPERTY(QBrush disableColor READ disableColor WRITE setDisableColor NOTIFY disableColorChanged)

    Q_PROPERTY(QBrush checkedNormalColor READ checkedNormalColor WRITE setCheckedNormalColor NOTIFY checkedNormalColorChanged)
    Q_PROPERTY(QBrush checkedHoverColor READ checkedHoverColor WRITE setCheckedHoverColor NOTIFY checkedHoverColorChanged)
    Q_PROPERTY(QBrush checkedClickColor READ checkedClickColor WRITE setCheckedClickColor NOTIFY checkedClickColorChanged)
    Q_PROPERTY(QBrush checkedDisableColor READ checkedDisableColor WRITE setCheckedDisableColor NOTIFY checkedDisableColorChanged)

    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoverBorderColor READ hoverBorderColor WRITE setHoverBorderColor NOTIFY hoverBorderColorChanged)
    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickBorderColorChanged)
    Q_PROPERTY(QBrush disableBorderColor READ disableBorderColor WRITE setDisableBorderColor NOTIFY disableBorderColorChanged)

    Q_PROPERTY(QBrush checkedNormalBorderColor READ checkedNormalBorderColor WRITE setCheckedNormalBorderColor NOTIFY checkedNormalBorderColorChanged)
    Q_PROPERTY(QBrush checkedHoverBorderColor READ checkedHoverBorderColor WRITE setCheckedHoverBorderColor NOTIFY checkedHoverBorderColorChanged)
    Q_PROPERTY(QBrush checkedClickBorderColor READ checkedClickBorderColor WRITE setCheckedClickBorderColor NOTIFY checkedClickBorderColorChanged)
    Q_PROPERTY(QBrush checkedDisableBorderColor READ checkedDisableBorderColor WRITE setCheckedDisableBorderColor NOTIFY checkedDisableBorderColorChanged)

    Q_PROPERTY(QBrush normalIndicatorColor READ normalIndicatorColor WRITE setNormalIndicatorColor NOTIFY normalIndicatorColorChanged)
    Q_PROPERTY(QBrush disableIndicatorColor READ disableIndicatorColor WRITE setDisableIndicatorColor NOTIFY disableIndicatorColorChanged)

    Q_PROPERTY(QBrush normalTransparentColor READ normalTransparentColor WRITE setNormalTransparentColor NOTIFY normalTransparentColorChanged)
    Q_PROPERTY(QBrush hoverTransparentColor READ hoverTransparentColor WRITE setHoverTransparentColor NOTIFY hoverTransparentColorChanged)
    Q_PROPERTY(QBrush clickTransparentColor READ clickTransparentColor WRITE setClickTransparentColor NOTIFY clickTransparentColorChanged)
    Q_PROPERTY(QBrush disableTransparentColor READ disableTransparentColor WRITE setDisableTransparentColor NOTIFY disableTransparentColorChanged)
    Q_PROPERTY(QBrush normalTransparentBorderColor READ normalTransparentBorderColor WRITE setNormalTransparentBorderColor NOTIFY normalTransparentBorderColorChanged)
    Q_PROPERTY(QBrush hoverTransparentBorderColor READ hoverTransparentBorderColor WRITE setHoverTransparentBorderColor NOTIFY hoverTransparentBorderColorChanged)
    Q_PROPERTY(QBrush clickTransparentBorderColor READ clickTransparentBorderColor WRITE setClickTransparentBorderColor NOTIFY clickTransparentBorderColorChanged)
    Q_PROPERTY(QBrush disableTransparentBorderColor READ disableTransparentBorderColor WRITE setDisableTransparentBorderColor NOTIFY disableTransparentBorderColorChanged)

    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int recWidth READ recWidth WRITE setRecWidth NOTIFY recWidthHChanged)
    Q_PROPERTY(int recHeight READ recHeight WRITE setRecHeight NOTIFY recHeightChanged)
    Q_PROPERTY(int indicatorWidth READ indicatorWidth WRITE setIndicatorWidth NOTIFY indicatorWidthChanged)
    Q_PROPERTY(int indicatorLeftRightPadding READ indicatorLeftRightPadding WRITE setIndicatorLeftRightPadding NOTIFY indicatorLeftRightPaddingChanged)

public:
    explicit UKUISwitch(QQuickItem *parent = nullptr);
    ~UKUISwitch();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUISwitch* qmlAttachedProperties(QObject* parent);

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

    const QBrush &checkedNormalColor() const;
    void setCheckedNormalColor(const QBrush &newCheckedNormalColor);

    const QBrush &checkedHoverColor() const;
    void setCheckedHoverColor(const QBrush &newCheckedHoverColor);

    const QBrush &checkedClickColor() const;
    void setCheckedClickColor(const QBrush &newCheckedClickColor);

    const QBrush &checkedDisableColor() const;
    void setCheckedDisableColor(const QBrush &newCheckedDisableColor);

    const QBrush &normalBorderColor() const;
    void setNormalBorderColor(const QBrush &newNormalBorderColor);

    const QBrush &hoverBorderColor() const;
    void setHoverBorderColor(const QBrush &newHoverBorderColor);

    const QBrush &clickBorderColor() const;
    void setClickBorderColor(const QBrush &newClickBorderColor);

    const QBrush &disableBorderColor() const;
    void setDisableBorderColor(const QBrush &newDisableBorderColor);

    const QBrush &checkedNormalBorderColor() const;
    void setCheckedNormalBorderColor(const QBrush &newCheckedNormalBorderColor);

    const QBrush &checkedHoverBorderColor() const;
    void setCheckedHoverBorderColor(const QBrush &newCheckedHoverBorderColor);

    const QBrush &checkedClickBorderColor() const;
    void setCheckedClickBorderColor(const QBrush &newCheckedClickBorderColor);

    const QBrush &checkedDisableBorderColor() const;
    void setCheckedDisableBorderColor(const QBrush &newCheckedDisableBorderColor);

    int borderWidth() const;
    void setBorderWidth(int newBorderWidth);

    const QBrush &normalIndicatorColor() const;
    void setNormalIndicatorColor(const QBrush &newNormalIndicatorColor);

    const QBrush &disableIndicatorColor() const;
    void setDisableIndicatorColor(const QBrush &newDisableIndicatorColor);

    int indicatorWidth() const;
    void setIndicatorWidth(int newIndicatorWidth);

    int recWidth() const;
    void setRecWidth(int newRecWidth);

    int recHeight() const;
    void setRecHeight(int newRecHeight);

    int indicatorLeftRightPadding() const;
    void setIndicatorLeftRightPadding(int newIndicatorLeftRightPadding);

    const QBrush &normalTransparentColor() const;
    void setNormalTransparentColor(const QBrush &newNormalTransparentColor);

    const QBrush &hoverTransparentColor() const;
    void setHoverTransparentColor(const QBrush &newHoverTransparentColor);

    const QBrush &clickTransparentColor() const;
    void setClickTransparentColor(const QBrush &newClickTransparentColor);

    const QBrush &disableTransparentColor() const;
    void setDisableTransparentColor(const QBrush &newDisableTransparentColor);

    const QBrush &normalTransparentBorderColor() const;
    void setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor);

    const QBrush &hoverTransparentBorderColor() const;
    void setHoverTransparentBorderColor(const QBrush &newHoverTransparentBorderColor);

    const QBrush &clickTransparentBorderColor() const;
    void setClickTransparentBorderColor(const QBrush &newClickTransparentBorderColor);

    const QBrush &disableTransparentBorderColor() const;
    void setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor);

signals:
    void normalColorChanged();

    void paddingChanged();

    void hoverColorChanged();

    void clickColorChanged();

    void disableColorChanged();

    void parametryChanged();

    void checkedNormalColorChanged();

    void checkedHoverColorChanged();

    void checkedClickColorChanged();

    void checkedDisableColorChanged();

    void normalBorderColorChanged();

    void hoverBorderColorChanged();

    void clickBorderColorChanged();

    void disableBorderColorChanged();

    void checkedNormalBorderColorChanged();

    void checkedHoverBorderColorChanged();

    void checkedClickBorderColorChanged();

    void checkedDisableBorderColorChanged();

    void borderWidthChanged();

    void normalIndicatorColorChanged();

    void disableIndicatorColorChanged();

    void indicatorWidthChanged();

    void recWidthHChanged();

    void recHeightChanged();

    void indicatorLeftRightPaddingChanged();

    void normalTransparentColorChanged();

    void hoverTransparentColorChanged();

    void clickTransparentColorChanged();

    void disableTransparentColorChanged();

    void normalTransparentBorderColorChanged();

    void hoverTransparentBorderColorChanged();

    void clickTransparentBorderColorChanged();

    void disableTransparentBorderColorChanged();

private:
    Q_INVOKABLE QBrush m_normalColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));

    int m_padding = 2;
    Q_INVOKABLE QBrush m_hoverColor;
    Q_INVOKABLE QBrush m_clickColor;
    Q_INVOKABLE QBrush m_disableColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_checkedNormalColor;
    QBrush m_checkedHoverColor;
    QBrush m_checkedClickColor;
    QBrush m_checkedDisableColor;
    QBrush m_normalBorderColor;
    QBrush m_hoverBorderColor;
    QBrush m_clickBorderColor;
    QBrush m_disableBorderColor;
    QBrush m_checkedNormalBorderColor;
    QBrush m_checkedHoverBorderColor;
    QBrush m_checkedClickBorderColor;
    QBrush m_checkedDisableBorderColor;
    int m_borderWidth;
    QBrush m_normalIndicatorColor;
    QBrush m_disableIndicatorColor;
    int m_indicatorWidth;
    int m_recWidth;
    int m_recHeight;
    int m_indicatorLeftRightPadding;
    QBrush m_normalTransparentColor;
    QBrush m_hoverTransparentColor;
    QBrush m_clickTransparentColor;
    QBrush m_disableTransparentColor;
    QBrush m_normalTransparentBorderColor;
    QBrush m_hoverTransparentBorderColor;
    QBrush m_clickTransparentBorderColor;
    QBrush m_disableTransparentBorderColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUISwitch, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUISWITCH_H
