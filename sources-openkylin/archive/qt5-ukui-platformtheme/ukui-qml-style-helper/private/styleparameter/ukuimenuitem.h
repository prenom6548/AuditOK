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

#ifndef UKUIMENUITEM_H
#define UKUIMENUITEM_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIMenuItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int leftRightPadding READ leftRightPadding WRITE setLeftRightPadding NOTIFY leftRightPaddingChanged)
    Q_PROPERTY(int topBottomPadding READ topBottomPadding WRITE setTopBottomPadding NOTIFY topBottomPaddingChanged)
    Q_PROPERTY(int imageWidth READ imageWidth WRITE setImageWidth NOTIFY imageWidthChanged)
    Q_PROPERTY(int imageSpace READ imageSpace WRITE setImageSpace NOTIFY imageSpaceChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setNormalHeight NOTIFY normalHeightChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY RadiusChanged)

    Q_PROPERTY(QBrush normalBC READ normalBC WRITE setNormalBC NOTIFY normalBCChanged)
    Q_PROPERTY(QBrush clickedBC READ clickedBC WRITE setClickedBC NOTIFY clickedBCChanged)
    Q_PROPERTY(QBrush hoveredBC READ hoveredBC WRITE setHoveredBC NOTIFY hoveredBCChanged)
    Q_PROPERTY(QBrush disableBC READ disableBC WRITE setDisableBC NOTIFY disableBCChanged)
    Q_PROPERTY(QBrush disableTextColor READ disableTextColor WRITE setDisableTextColor NOTIFY disableTextColorChanged)
    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY disableTextColorChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoverBorderColor READ hoverBorderColor WRITE setHoverBorderColor NOTIFY hoverBorderColorChanged)
    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickBorderColorChanged)
    Q_PROPERTY(QBrush disableBorderColor READ disableBorderColor WRITE setDisableBorderColor NOTIFY disableBorderColorChanged)

    Q_PROPERTY(QBrush  normalTransparentBC READ  normalTransparentBC WRITE  setNormalTransparentBC NOTIFY  normalTransparentBCChanged)
    Q_PROPERTY(QBrush clickedTransparentBC READ clickedTransparentBC WRITE setClickedTransparentBC NOTIFY clickedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredTransparentBC READ hoveredTransparentBC WRITE setHoveredTransparentBC NOTIFY hoveredTransparentBCChanged)
    Q_PROPERTY(QBrush disableTransparentBC READ disableTransparentBC WRITE setDisableTransparentBC NOTIFY disableTransparentBCChanged)
    Q_PROPERTY(QBrush normalTransparentBorderColor READ normalTransparentBorderColor WRITE setNormalTransparentBorderColor NOTIFY normalTransparentBorderColorChanged)
    Q_PROPERTY(QBrush hoverTransparentBorderColor READ hoverTransparentBorderColor WRITE setHoverTransparentBorderColor NOTIFY hoverTransparentBorderColorChanged)
    Q_PROPERTY(QBrush clickTransparentBorderColor READ clickTransparentBorderColor WRITE setClickTransparentBorderColor NOTIFY clickTransparentBorderColorChanged)
    Q_PROPERTY(QBrush disableTransparentBorderColor READ disableTransparentBorderColor WRITE setDisableTransparentBorderColor NOTIFY disableTransparentBorderColorChanged)

    Q_PROPERTY(QBrush normalCheckedBC READ normalCheckedBC WRITE setNormalCheckedBC NOTIFY normalCheckedBCChanged)
    Q_PROPERTY(QBrush clickedCheckedBC READ clickedCheckedBC WRITE setClickedCheckedBC NOTIFY clickedCheckedBCChanged)
    Q_PROPERTY(QBrush hoveredCheckedBC READ hoveredCheckedBC WRITE setHoveredCheckedBC NOTIFY hoveredCheckedBCChanged)
    Q_PROPERTY(QBrush disableCheckedBC READ disableCheckedBC WRITE setDisableCheckedBC NOTIFY disableCheckedBCChanged)
    Q_PROPERTY(QBrush normalBorderCheckedColor READ normalBorderCheckedColor WRITE setNormalBorderCheckedColor NOTIFY normalBorderCheckedColorChanged)
    Q_PROPERTY(QBrush hoverBorderCheckedColor READ hoverBorderCheckedColor WRITE setHoverBorderCheckedColor NOTIFY hoverBorderCheckedColorChanged)
    Q_PROPERTY(QBrush clickBorderCheckedColor READ clickBorderCheckedColor WRITE setClickBorderCheckedColor NOTIFY clickBorderCheckedColorChanged)
    Q_PROPERTY(QBrush disableBorderCheckedColor READ disableBorderCheckedColor WRITE setDisableBorderCheckedColor NOTIFY disableBorderCheckedColorChanged)

    Q_PROPERTY(QBrush normalCheckedTransparentBC  READ  normalCheckedTransparentBC WRITE  setNormalCheckedTransparentBC NOTIFY  normalCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush clickedCheckedTransparentBC READ clickedCheckedTransparentBC WRITE setClickedCheckedTransparentBC NOTIFY clickedCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredCheckedTransparentBC READ hoveredCheckedTransparentBC WRITE setHoveredCheckedTransparentBC NOTIFY hoveredCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush disableCheckedTransparentBC READ disableCheckedTransparentBC WRITE setDisableCheckedTransparentBC NOTIFY disableCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush normalBorderCheckedTransparentColor READ normalBorderCheckedTransparentColor WRITE setNormalBorderCheckedTransparentColor NOTIFY normalBorderCheckedColorTransparentChanged)
    Q_PROPERTY(QBrush hoverBorderCheckedTransparentColor READ hoverBorderCheckedTransparentColor WRITE setHoverBorderCheckedTransparentColor NOTIFY hoverBorderCheckedColorTransparentChanged)
    Q_PROPERTY(QBrush clickBorderCheckedTransparentColor READ clickBorderCheckedTransparentColor WRITE setClickBorderCheckedTransparentColor NOTIFY clickBorderCheckedColorTransparentChanged)
    Q_PROPERTY(QBrush disableBorderCheckedTransparentColor READ disableBorderCheckedTransparentColor WRITE setDisableBorderCheckedTransparentColor NOTIFY disableBorderCheckedColorTransparentChanged)

    Q_PROPERTY(QBrush  normalHTextColor READ  normalHTextColor WRITE  setNormalHTextColor NOTIFY  normalHTextColorChanged)
    Q_PROPERTY(QBrush disableHTextColor READ disableHTextColor WRITE setDisableHTextColor NOTIFY disableHTextColorChanged)
    Q_PROPERTY(QBrush  normalCheckedHBC READ  normalCheckedHBC WRITE  setNormalCheckedHBC NOTIFY  normalCheckedHBCChanged)
    Q_PROPERTY(QBrush clickedCheckedHBC READ clickedCheckedHBC WRITE setClickedCheckedHBC NOTIFY clickedCheckedHBCChanged)
    Q_PROPERTY(QBrush hoveredCheckedHBC READ hoveredCheckedHBC WRITE setHoveredCheckedHBC NOTIFY hoveredCheckedHBCChanged)
    Q_PROPERTY(QBrush disableCheckedHBC READ disableCheckedHBC WRITE setDisableCheckedHBC NOTIFY disableCheckedHBCChanged)
    Q_PROPERTY(QBrush  normalCheckedHBorderColor READ  normalCheckedHBorderColor WRITE  setNormalCheckedHBorderColor NOTIFY  normalCheckedHBorderColorChanged)
    Q_PROPERTY(QBrush clickedCheckedHBorderColor READ clickedCheckedHBorderColor WRITE setClickedCheckedHBorderColor NOTIFY clickedCheckedHBorderColorChanged)
    Q_PROPERTY(QBrush hoveredCheckedHBorderColor READ hoveredCheckedHBorderColor WRITE setHoveredCheckedHBorderColor NOTIFY hoveredCheckedHBorderColorChanged)
    Q_PROPERTY(QBrush disableCheckedHBorderColor READ disableCheckedHBorderColor WRITE setDisableCheckedHBorderColor NOTIFY disableCheckedHBorderColorChanged)

    Q_PROPERTY(QBrush menuSeparatorColor READ menuSeparatorColor WRITE setMenuSeparatorColor NOTIFY menuSeparatorColorChanged)

public:
    explicit UKUIMenuItem(QQuickItem *parent = nullptr);
    ~UKUIMenuItem();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUIMenuItem* qmlAttachedProperties(QObject* parent);

    int leftRightPadding() const;
    void setLeftRightPadding(int newLeftRightPadding);

    int topBottomPadding() const;
    void setTopBottomPadding(int newTopBottomPadding);

    int imageWidth() const;
    void setImageWidth(int newImageWidth);

    int imageSpace() const;
    void setImageSpace(int newImageSpace);

    int normalHeight() const;
    void setNormalHeight(int newNormalHeight);

    int radius() const;
    void setRadius(int newRadius);

    const QBrush &normalBC() const;
    void setNormalBC(const QBrush &newNormalBC);

    const QBrush &clickedBC() const;
    void setClickedBC(const QBrush &newClickedBC);

    const QBrush &hoveredBC() const;
    void setHoveredBC(const QBrush &newHoveredBC);

    const QBrush &disableBC() const;
    void setDisableBC(const QBrush &newDisableBC);

    const QBrush &disableTextColor() const;
    void setDisableTextColor(const QBrush &newDisableTextColor);

    const QBrush &normalTextColor() const;
    void setNormalTextColor(const QBrush &newNormalTextColor);

    int borderWidth() const;
    void setBorderWidth(int newBorderWidth);

    const QBrush &normalBorderColor() const;
    void setNormalBorderColor(const QBrush &newNormalBorderColor);

    const QBrush &hoverBorderColor() const;
    void setHoverBorderColor(const QBrush &newHoverBorderColor);

    const QBrush &clickBorderColor() const;
    void setClickBorderColor(const QBrush &newClickBorderColor);

    const QBrush &disableBorderColor() const;
    void setDisableBorderColor(const QBrush &newDisableBorderColor);

    const QBrush &normalTransparentBC() const;
    void setNormalTransparentBC(const QBrush &newNormalTransparentBC);

    const QBrush &clickedTransparentBC() const;
    void setClickedTransparentBC(const QBrush &newClickedTransparentBC);

    const QBrush &hoveredTransparentBC() const;
    void setHoveredTransparentBC(const QBrush &newHoveredTransparentBC);

    const QBrush &disableTransparentBC() const;
    void setDisableTransparentBC(const QBrush &newDisableTransparentBC);

    const QBrush &normalTransparentBorderColor() const;
    void setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor);

    const QBrush &hoverTransparentBorderColor() const;
    void setHoverTransparentBorderColor(const QBrush &newHoverTransparentBorderColor);

    const QBrush &clickTransparentBorderColor() const;
    void setClickTransparentBorderColor(const QBrush &newClickTransparentBorderColor);

    const QBrush &disableTransparentBorderColor() const;
    void setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor);

    const QBrush &normalCheckedBC() const;
    void setNormalCheckedBC(const QBrush &newNormalCheckedBC);

    const QBrush &clickedCheckedBC() const;
    void setClickedCheckedBC(const QBrush &newClickedCheckedBC);

    const QBrush &hoveredCheckedBC() const;
    void setHoveredCheckedBC(const QBrush &newHoveredCheckedBC);

    const QBrush &disableCheckedBC() const;
    void setDisableCheckedBC(const QBrush &newDisableCheckedBC);

    const QBrush &normalBorderCheckedColor() const;
    void setNormalBorderCheckedColor(const QBrush &newNormalBorderCheckedColor);

    const QBrush &hoverBorderCheckedColor() const;
    void setHoverBorderCheckedColor(const QBrush &newHoverBorderCheckedColor);

    const QBrush &clickBorderCheckedColor() const;
    void setClickBorderCheckedColor(const QBrush &newClickBorderCheckedColor);

    const QBrush &disableBorderCheckedColor() const;
    void setDisableBorderCheckedColor(const QBrush &newDisableBorderCheckedColor);

    const QBrush &normalHTextColor() const;
    void setNormalHTextColor(const QBrush &newNormalHTextColor);

    const QBrush &disableHTextColor() const;
    void setDisableHTextColor(const QBrush &newDisableHTextColor);

    const QBrush &normalCheckedHBC() const;
    void setNormalCheckedHBC(const QBrush &newNormalCheckedHBC);

    const QBrush &clickedCheckedHBC() const;
    void setClickedCheckedHBC(const QBrush &newClickedCheckedHBC);

    const QBrush &hoveredCheckedHBC() const;
    void setHoveredCheckedHBC(const QBrush &newHoveredCheckedHBC);

    const QBrush &disableCheckedHBC() const;
    void setDisableCheckedHBC(const QBrush &newDisableCheckedHBC);

    const QBrush &normalCheckedHBorderColor() const;
    void setNormalCheckedHBorderColor(const QBrush &newNormalCheckedHBorderColor);

    const QBrush &clickedCheckedHBorderColor() const;
    void setClickedCheckedHBorderColor(const QBrush &newClickedCheckedHBorderColor);

    const QBrush &hoveredCheckedHBorderColor() const;
    void setHoveredCheckedHBorderColor(const QBrush &newHoveredCheckedHBorderColor);

    const QBrush &disableCheckedHBorderColor() const;
    void setDisableCheckedHBorderColor(const QBrush &newDisableCheckedHBorderColor);

    const QBrush &normalCheckedTransparentBC() const;
    void setNormalCheckedTransparentBC(const QBrush &newNormalCheckedTransparentBC);

    const QBrush &clickedCheckedTransparentBC() const;
    void setClickedCheckedTransparentBC(const QBrush &newClickedCheckedTransparentBC);

    const QBrush &hoveredCheckedTransparentBC() const;
    void setHoveredCheckedTransparentBC(const QBrush &newHoveredCheckedTransparentBC);

    const QBrush &disableCheckedTransparentBC() const;
    void setDisableCheckedTransparentBC(const QBrush &newDisableCheckedTransparentBC);

    const QBrush &normalBorderCheckedTransparentColor() const;
    void setNormalBorderCheckedTransparentColor(const QBrush &newNormalBorderCheckedTransparentColor);

    const QBrush &hoverBorderCheckedTransparentColor() const;
    void setHoverBorderCheckedTransparentColor(const QBrush &newHoverBorderCheckedTransparentColor);

    const QBrush &clickBorderCheckedTransparentColor() const;
    void setClickBorderCheckedTransparentColor(const QBrush &newClickBorderCheckedTransparentColor);

    const QBrush &disableBorderCheckedTransparentColor() const;
    void setDisableBorderCheckedTransparentColor(const QBrush &newDisableBorderCheckedTransparentColor);

    const QBrush &menuSeparatorColor() const;
    void setMenuSeparatorColor(const QBrush &newMenuSeparatorColor);

signals:
    void leftRightPaddingChanged();

    void topBottomPaddingChanged();

    void imageWidthChanged();

    void imageSpaceChanged();

    void normalHeightChanged();

    void RadiusChanged();

    void normalBCChanged();

    void clickedBCChanged();

    void hoveredBCChanged();

    void disableBCChanged();

    void disableTextColorChanged();

    void borderWidthChanged();

    void normalBorderColorChanged();

    void hoverBorderColorChanged();

    void clickBorderColorChanged();

    void disableBorderColorChanged();

    void parametryChanged();

    void normalTransparentBCChanged();

    void clickedTransparentBCChanged();

    void hoveredTransparentBCChanged();

    void disableTransparentBCChanged();

    void normalTransparentBorderColorChanged();

    void hoverTransparentBorderColorChanged();

    void clickTransparentBorderColorChanged();

    void disableTransparentBorderColorChanged();

    void normalCheckedBCChanged();

    void clickedCheckedBCChanged();

    void hoveredCheckedBCChanged();

    void disableCheckedBCChanged();

    void normalBorderCheckedColorChanged();

    void hoverBorderCheckedColorChanged();

    void clickBorderCheckedColorChanged();

    void disableBorderCheckedColorChanged();

    void normalHTextColorChanged();

    void disableHTextColorChanged();

    void normalCheckedHBCChanged();

    void clickedCheckedHBCChanged();

    void hoveredCheckedHBCChanged();

    void disableCheckedHBCChanged();

    void normalCheckedHBorderColorChanged();

    void clickedCheckedHBorderColorChanged();

    void hoveredCheckedHBorderColorChanged();

    void disableCheckedHBorderColorChanged();

    void normalCheckedTransparentBCChanged();

    void clickedCheckedTransparentBCChanged();

    void hoveredCheckedTransparentBCChanged();

    void disableCheckedTransparentBCChanged();

    void normalBorderCheckedColorTransparentChanged();

    void hoverBorderCheckedColorTransparentChanged();

    void clickBorderCheckedColorTransparentChanged();

    void disableBorderCheckedColorTransparentChanged();

    void menuSeparatorColorChanged();

private:
    int m_leftRightPadding = 8;
    int m_topBottomPadding = 6;
    int m_imageWidth = 16;
    int m_imageSpace = 4;
    int m_normalHeight = 36;
    int m_radius = 6;
    Q_INVOKABLE QBrush m_normalBC;
    Q_INVOKABLE QBrush m_clickedBC;
    Q_INVOKABLE QBrush m_hoveredBC;
    Q_INVOKABLE QBrush m_disableBC;
    Q_INVOKABLE QBrush m_disableTextColor;
    Q_INVOKABLE QBrush m_normalTextColor;
    int m_borderWidth;
    Q_INVOKABLE QBrush m_normalBorderColor;
    Q_INVOKABLE QBrush m_hoverBorderColor;
    Q_INVOKABLE QBrush m_clickBorderColor;
    Q_INVOKABLE QBrush m_disableBorderColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_normalTransparentBC;
    QBrush m_clickedTransparentBC;
    QBrush m_hoveredTransparentBC;
    QBrush m_disableTransparentBC;
    QBrush m_normalTransparentBorderColor;
    QBrush m_hoverTransparentBorderColor;
    QBrush m_clickTransparentBorderColor;
    QBrush m_disableTransparentBorderColor;
    QBrush m_normalCheckedBC;
    QBrush m_clickedCheckedBC;
    QBrush m_hoveredCheckedBC;
    QBrush m_disableCheckedBC;
    QBrush m_normalBorderCheckedColor;
    QBrush m_hoverBorderCheckedColor;
    QBrush m_clickBorderCheckedColor;
    QBrush m_disableBorderCheckedColor;
    QBrush m_normalHTextColor;
    QBrush m_disableHTextColor;
    QBrush m_normalCheckedHBC;
    QBrush m_clickedCheckedHBC;
    QBrush m_hoveredCheckedHBC;
    QBrush m_disableCheckedHBC;
    QBrush m_normalCheckedHBorderColor;
    QBrush m_clickedCheckedHBorderColor;
    QBrush m_hoveredCheckedHBorderColor;
    QBrush m_disableCheckedHBorderColor;
    QBrush m_normalCheckedTransparentBC;
    QBrush m_clickedCheckedTransparentBC;
    QBrush m_hoveredCheckedTransparentBC;
    QBrush m_disableCheckedTransparentBC;
    QBrush m_normalBorderCheckedTransparentColor;
    QBrush m_hoverBorderCheckedTransparentColor;
    QBrush m_clickBorderCheckedTransparentColor;
    QBrush m_disableBorderCheckedTransparentColor;
    QBrush m_menuSeparatorColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIMenuItem, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUIMENUITEM_H
