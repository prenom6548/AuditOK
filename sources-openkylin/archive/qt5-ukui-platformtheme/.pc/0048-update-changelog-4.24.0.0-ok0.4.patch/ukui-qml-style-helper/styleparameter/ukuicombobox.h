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

#ifndef UKUICOMBOBOX_H
#define UKUICOMBOBOX_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIComboBox : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(double leftRightPadding READ leftRightPadding WRITE setLeftRightPadding NOTIFY leftRightPaddingChanged)
    Q_PROPERTY(double upDownMargin READ upDownMargin WRITE setUpDownMargin NOTIFY upDownMarginChanged)
    Q_PROPERTY(double space READ space WRITE setSpace NOTIFY spaceChange)

    Q_PROPERTY(int normalWidth READ normalWidth WRITE setNormalWidth NOTIFY normalWidthChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setNormalHeight NOTIFY normalHeightChanged)
    Q_PROPERTY(QBrush normalBC READ normalBC WRITE setNormalBC NOTIFY normalBCChanged)
    Q_PROPERTY(QBrush clickedBC READ clickedBC WRITE setClickedBC NOTIFY clickedBCChanged)
    Q_PROPERTY(QBrush hoveredBC READ hoveredBC WRITE setHoveredBC NOTIFY hoveredBCChanged)
    Q_PROPERTY(QBrush disableBC READ disableBC WRITE setDisableBC NOTIFY disableBCChanged)
    Q_PROPERTY(QBrush disableTextColor READ disableTextColor WRITE setDisableTextColor NOTIFY disableTextColorChanged)
    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY normalTextColorChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int focusBorderWidth READ focusBorderWidth WRITE setFocusBorderWidth NOTIFY focusBorderWidthChanged)
    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoveredBorderColor READ hoveredBorderColor WRITE setHoveredBorderColor NOTIFY hoveredBorderColorChanged)
    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickBorderColorChanged)
    Q_PROPERTY(QBrush disableBorderColor READ disableBorderColor WRITE setDisableBorderColor NOTIFY disableBorderColorChanged)
    Q_PROPERTY(QBrush focusBorderColor READ focusBorderColor WRITE setFocusBorderColor NOTIFY focusBorderColorChanged)

    Q_PROPERTY(QBrush  normalTransparentBC READ  normalTransparentBC WRITE  setNormalTransparentBC NOTIFY  normalTransparentBCChanged)
    Q_PROPERTY(QBrush clickedTransparentBC READ clickedTransparentBC WRITE setClickedTransparentBC NOTIFY clickedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredTransparentBC READ hoveredTransparentBC WRITE setHoveredTransparentBC NOTIFY hoveredTransparentBCChanged)
    Q_PROPERTY(QBrush disableTransparentBC READ disableTransparentBC WRITE setDisableTransparentBC NOTIFY disableTransparentBCChanged)

    Q_PROPERTY(QBrush  normalTransparentBorderColor READ  normalTransparentBorderColor WRITE  setNormalTransparentBorderColor NOTIFY  normalTransparentBorderColorChanged)
    Q_PROPERTY(QBrush clickedTransparentBorderColor READ clickedTransparentBorderColor WRITE setClickedTransparentBorderColor NOTIFY clickedTransparentBorderColorChanged)
    Q_PROPERTY(QBrush hoveredTransparentBorderColor READ hoveredTransparentBorderColor WRITE setHoveredTransparentBorderColor NOTIFY hoveredTransparentBorderColorChanged)
    Q_PROPERTY(QBrush disableTransparentBorderColor READ disableTransparentBorderColor WRITE setDisableTransparentBorderColor NOTIFY disableTransparentBorderColorChanged)


public:
    explicit UKUIComboBox(QQuickItem *parent = nullptr);
    ~UKUIComboBox();

    static UKUIComboBox* qmlAttachedProperties(QObject* parent);

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);


    double radius() const;
    void setRadius(double newRadius);

    double leftRightPadding() const;
    void setLeftRightPadding(double newLeftRightPadding);

    double upDownMargin() const;
    void setUpDownMargin(double newUpDownMargin);

    double space() const;
    void setSpace(double newSpace);

    int normalWidth() const;
    void setNormalWidth(int newNormalWidth);

    int normalHeight() const;
    void setNormalHeight(int newNormalHeight);

    Q_INVOKABLE QBrush normalBC() const;
    void setNormalBC(QBrush newNormalBC);

    Q_INVOKABLE QBrush clickedBC() const;
    void setClickedBC(QBrush newClickedBC);

    Q_INVOKABLE QBrush hoveredBC() const;
    void setHoveredBC(QBrush newHoveredBC);

    Q_INVOKABLE QBrush disableBC() const;
    void setDisableBC(QBrush newDisableBC);

    Q_INVOKABLE QBrush disableTextColor() const;
    void setDisableTextColor(QBrush newDisableTextColor);

    Q_INVOKABLE QBrush normalTextColor() const;
    void setNormalTextColor(QBrush newNormalTextColor);

    int borderWidth() const;
    void setBorderWidth(int newBorderWidth);

    int focusBorderWidth() const;
    void setFocusBorderWidth(int newFocusBorderWidth);

    Q_INVOKABLE QBrush normalBorderColor() const;
    void setNormalBorderColor(QBrush newNormalBorderColor);

    Q_INVOKABLE QBrush clickBorderColor() const;
    void setClickBorderColor(QBrush newClickBorderColor);

    Q_INVOKABLE QBrush disableBorderColor() const;
    void setDisableBorderColor(QBrush newDisableBorderColor);

    Q_INVOKABLE QBrush focusBorderColor() const;
    void setFocusBorderColor(QBrush newFocusBorderColor);

    const QBrush &normalTransparentBC() const;
    void setNormalTransparentBC(const QBrush &newNormalTransparentBC);

    const QBrush &clickedTransparentBC() const;
    void setClickedTransparentBC(const QBrush &newClickedTransparentBC);

    const QBrush &hoveredTransparentBC() const;
    void setHoveredTransparentBC(const QBrush &newHoveredTransparentBC);

    const QBrush &disableTransparentBC() const;
    void setDisableTransparentBC(const QBrush &newDisableTransparentBC);

    const QBrush &hoveredBorderColor() const;
    void setHoveredBorderColor(const QBrush &newHoveredBorderColor);

    const QBrush &normalTransparentBorderColor() const;
    void setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor);

    const QBrush &clickedTransparentBorderColor() const;
    void setClickedTransparentBorderColor(const QBrush &newClickedTransparentBorderColor);

    const QBrush &hoveredTransparentBorderColor() const;
    void setHoveredTransparentBorderColor(const QBrush &newHoveredTransparentBorderColor);

    const QBrush &disableTransparentBorderColor() const;
    void setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor);

signals:

    void radiusChanged();

    void leftRightPaddingChanged();

    void upDownMarginChanged();

    void spaceChange();

    void normalWidthChanged();

    void normalHeightChanged();

    void normalBCChanged();

    void clickedBCChanged();

    void hoveredBCChanged();

    void disableBCChanged();

    void disableTextColorChanged();

    void normalTextColorChanged();

    void borderWidthChanged();

    void focusBorderWidthChanged();

    void normalBorderColorChanged();

    void clickBorderColorChanged();

    void disableBorderColorChanged();

    void focusBorderColorChanged();

    void parametryChanged();

    void normalTransparentBCChanged();

    void clickedTransparentBCChanged();

    void hoveredTransparentBCChanged();

    void disableTransparentBCChanged();

    void hoveredBorderColorChanged();

    void normalTransparentBorderColorChanged();

    void clickedTransparentBorderColorChanged();

    void hoveredTransparentBorderColorChanged();

    void disableTransparentBorderColorChanged();

private:

    double m_radius;
    double m_leftRightPadding;
    double m_upDownMargin;
    double m_space;
    int m_normalWidth;
    int m_normalHeight;
    int m_borderWidth;
    int m_focusBorderWidth;

    Q_INVOKABLE QBrush m_normalBC           ;
    Q_INVOKABLE QBrush m_clickedBC          ;
    Q_INVOKABLE QBrush m_hoveredBC          ;
    Q_INVOKABLE QBrush m_disableBC          ;
    Q_INVOKABLE QBrush m_disableTextColor   ;
    Q_INVOKABLE QBrush m_normalTextColor    ;
    Q_INVOKABLE QBrush m_normalBorderColor  ;
    Q_INVOKABLE QBrush m_clickBorderColor   ;
    Q_INVOKABLE QBrush m_disableBorderColor ;
    Q_INVOKABLE QBrush m_focusBorderColor   ;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_normalTransparentBC;
    QBrush m_clickedTransparentBC;
    QBrush m_hoveredTransparentBC;
    QBrush m_disableTransparentBC;
    QBrush m_hoveredBorderColor;
    QBrush m_normalTransparentBorderColor;
    QBrush m_clickedTransparentBorderColor;
    QBrush m_hoveredTransparentBorderColor;
    QBrush m_disableTransparentBorderColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIComboBox, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUICOMBOBOX_H
