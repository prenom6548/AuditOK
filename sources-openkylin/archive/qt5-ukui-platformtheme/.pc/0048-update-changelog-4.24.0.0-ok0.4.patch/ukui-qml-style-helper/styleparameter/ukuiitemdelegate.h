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

#ifndef UKUIITEMDELEGATE_H
#define UKUIITEMDELEGATE_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIItemDelegate : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)

    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY normalTextColorChanged)
    Q_PROPERTY(QBrush disableTextColor READ disableTextColor WRITE setDisableTextColor NOTIFY disableTextColorChanged)
    Q_PROPERTY(QBrush normalBC READ normalBC WRITE setNormalBC NOTIFY normalBCChanged)
    Q_PROPERTY(QBrush clickedBC READ clickedBC WRITE setClickedBC NOTIFY clickedBCChanged)
    Q_PROPERTY(QBrush hoveredBC READ hoveredBC WRITE setHoveredBC NOTIFY hoveredBCChanged)
    Q_PROPERTY(QBrush disableBC READ disableBC WRITE setDisableBC NOTIFY disableBCChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoverBorderColor READ hoverBorderColor WRITE setHoverBorderColor NOTIFY hoverBorderColorChanged)
    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickBorderColorChanged)
    Q_PROPERTY(QBrush disableBorderColor READ disableBorderColor WRITE setDisableBorderColor NOTIFY disableBorderColorChanged)

    Q_PROPERTY(QBrush  normalHTextColor READ  normalHTextColor WRITE  setNormalHTextColor NOTIFY  normalHTextColorChanged)
    Q_PROPERTY(QBrush disableHTextColor READ disableHTextColor WRITE setDisableHTextColor NOTIFY disableHTextColorChanged)
    Q_PROPERTY(QBrush normalHBC READ normalHBC WRITE setNormalHBC NOTIFY normalHBCChanged)
    Q_PROPERTY(QBrush clickedHBC READ clickedHBC WRITE setClickedHBC NOTIFY clickedHBCChanged)
    Q_PROPERTY(QBrush hoveredHBC READ hoveredHBC WRITE setHoveredHBC NOTIFY hoveredHBCChanged)
    Q_PROPERTY(QBrush disableHBC READ disableHBC WRITE setDisableHBC NOTIFY disabledHBCChanged)
    Q_PROPERTY(QBrush normalBorderHColor READ normalBorderHColor WRITE setNormalBorderHColor NOTIFY normalBorderHColorChanged)
    Q_PROPERTY(QBrush hoverBorderHColor READ hoverBorderHColor WRITE setHoverBorderHColor NOTIFY hoverBorderHColorChanged)
    Q_PROPERTY(QBrush clickBorderHColor READ clickBorderHColor WRITE setClickBorderHColor NOTIFY clickBorderHColorChanged)
    Q_PROPERTY(QBrush disableBorderHColor READ disableBorderHColor WRITE setDisableBorderHColor NOTIFY disabledBorderHColorChanged)

    Q_PROPERTY(QBrush normalCheckedBC READ normalCheckedBC WRITE setNormalCheckedBC NOTIFY normalCheckedBCChanged)
    Q_PROPERTY(QBrush clickedCheckedBC READ clickedCheckedBC WRITE setClickedCheckedBC NOTIFY clickedCheckedBCChanged)
    Q_PROPERTY(QBrush hoveredCheckedBC READ hoveredCheckedBC WRITE setHoveredCheckedBC NOTIFY hoveredCheckedBCChanged)
    Q_PROPERTY(QBrush disableCheckedBC READ disableCheckedBC WRITE setDisableCheckedBC NOTIFY disableCheckedBCChanged)
    Q_PROPERTY(QBrush normalBorderCheckedColor READ normalBorderCheckedColor WRITE setNormalBorderCheckedColor NOTIFY normalBorderCheckedColorChanged)
    Q_PROPERTY(QBrush hoverBorderCheckedColor READ hoverBorderCheckedColor WRITE setHoverBorderCheckedColor NOTIFY hoverBorderCheckedColorChanged)
    Q_PROPERTY(QBrush clickBorderCheckedColor READ clickBorderCheckedColor WRITE setClickBorderCheckedColor NOTIFY clickBorderCheckedColorChanged)
    Q_PROPERTY(QBrush disableBorderCheckedColor READ disableBorderCheckedColor WRITE setDisableBorderCheckedColor NOTIFY disableBorderCheckedColorChanged)
    Q_PROPERTY(int implicitHeight READ implicitHeight WRITE setImplicitHeight NOTIFY implicitHeightChanged)


    Q_PROPERTY(QBrush  normalTransparentBC READ  normalTransparentBC WRITE  setNormalTransparentBC NOTIFY  normalTransparentBCChanged)
    Q_PROPERTY(QBrush clickedTransparentBC READ clickedTransparentBC WRITE setClickedTransparentBC NOTIFY clickedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredTransparentBC READ hoveredTransparentBC WRITE setHoveredTransparentBC NOTIFY hoveredTransparentBCChanged)
    Q_PROPERTY(QBrush disableTransparentBC READ disableTransparentBC WRITE setDisableTransparentBC NOTIFY disableTransparentBCChanged)
    Q_PROPERTY(QBrush  normalTransparentBorderColor READ  normalTransparentBorderColor WRITE  setNormalTransparentBorderColor NOTIFY  normalTransparentBorderColorChanged)
    Q_PROPERTY(QBrush clickedTransparentBorderColor READ clickedTransparentBorderColor WRITE setClickedTransparentBorderColor NOTIFY clickedTransparentBorderColorChanged)
    Q_PROPERTY(QBrush hoveredTransparentBorderColor READ hoveredTransparentBorderColor WRITE setHoveredTransparentBorderColor NOTIFY hoveredTransparentBorderColorChanged)
    Q_PROPERTY(QBrush disableTransparentBorderColor READ disableTransparentBorderColor WRITE setDisableTransparentBorderColor NOTIFY disableTransparentBorderColorChanged)

    Q_PROPERTY(QBrush  normalCheckedTransparentBC READ  normalCheckedTransparentBC WRITE  setNormalCheckedTransparentBC NOTIFY  normalCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush clickedCheckedTransparentBC READ clickedCheckedTransparentBC WRITE setClickedCheckedTransparentBC NOTIFY clickedCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredCheckedTransparentBC READ hoveredCheckedTransparentBC WRITE setHoveredCheckedTransparentBC NOTIFY hoveredCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush disableCheckedTransparentBC READ disableCheckedTransparentBC WRITE setDisableCheckedTransparentBC NOTIFY disableCheckedTransparentBCChanged)
    Q_PROPERTY(QBrush  normalCheckedTransparentBorderColor READ  normalCheckedTransparentBorderColor WRITE  setNormalCheckedTransparentBorderColor NOTIFY  normalCheckedTransparentBorderColorChanged)
    Q_PROPERTY(QBrush clickedCheckedTransparentBorderColor READ clickedCheckedTransparentBorderColor WRITE setClickedCheckedTransparentBorderColor NOTIFY clickedCheckedTransparentBorderColorChanged)
    Q_PROPERTY(QBrush hoveredCheckedTransparentBorderColor READ hoveredCheckedTransparentBorderColor WRITE setHoveredCheckedTransparentBorderColor NOTIFY hoveredCheckedTransparentBorderColorChanged)
    Q_PROPERTY(QBrush disableCheckedTransparentBorderColor READ disableCheckedTransparentBorderColor WRITE setDisableCheckedTransparentBorderColor NOTIFY disableCheckedTransparentBorderColorChanged)


    Q_PROPERTY(QBrush  normalAlternateBC   READ normalAlternateBC WRITE setNormalAlternateBC NOTIFY  normalAlternateBCChanged)
    Q_PROPERTY(QBrush clickedAlternatedBC READ clickedAlternateBC WRITE setClickedAlternateBC NOTIFY clickedAlternateBCChanged)
    Q_PROPERTY(QBrush hoveredAlternatedBC READ hoveredAlternateBC WRITE setHoveredAlternateBC NOTIFY hoveredAlternateBCChanged)
    Q_PROPERTY(QBrush disableAlternateBC READ disableAlternateBC WRITE setDisableAlternateBC NOTIFY disableAlternateBCChanged)
    Q_PROPERTY(QBrush  normalAlternateTransparentBC READ  normalAlternateTransparentBC WRITE  setNormalAlternateTransparentBC NOTIFY  normalAlternateTransparentBCChanged)
    Q_PROPERTY(QBrush clickedAlternateTransparentBC READ clickedAlternateTransparentBC WRITE setClickedAlternateTransparentBC NOTIFY clickedAlternateTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredAlternateTransparentBC READ hoveredAlternateTransparentBC WRITE setHoveredAlternateTransparentBC NOTIFY hoveredAlternateTransparentBCChanged)
    Q_PROPERTY(QBrush disableAlternateTransparentBC READ disableAlternateTransparentBC WRITE setDisableAlternateTransparentBC NOTIFY disableAlternateTransparentBCChanged)

public:
    explicit UKUIItemDelegate(QQuickItem *parent = nullptr);
    ~UKUIItemDelegate();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUIItemDelegate* qmlAttachedProperties(QObject* parent);


    Q_INVOKABLE QBrush normalTextColor() const;
    void setNormalTextColor(QBrush newNormalTextColor);

    Q_INVOKABLE QBrush disableTextColor() const;
    void setDisableTextColor(QBrush newDisableTextColor);

    Q_INVOKABLE QBrush normalBC() const;
    void setNormalBC(QBrush newNormalBC);

    Q_INVOKABLE QBrush clickedBC() const;
    void setClickedBC(QBrush newClickedBC);

    Q_INVOKABLE QBrush hoveredBC() const;
    void setHoveredBC(QBrush newHoveredBC);

    Q_INVOKABLE QBrush disableBC() const;
    void setDisableBC(QBrush newDisableBC);

    int radius() const;
    void setRadius(int newRadius);

    int borderWidth() const;
    void setBorderWidth(int newBorderWidth);

    Q_INVOKABLE QBrush normalBorderColor() const;
    void setNormalBorderColor(QBrush newNormalBorderColor);

    Q_INVOKABLE QBrush hoverBorderColor() const;
    void setHoverBorderColor(QBrush newHoverBorderColor);

    Q_INVOKABLE QBrush clickBorderColor() const;
    void setClickBorderColor(QBrush newClickBorderColor);

    Q_INVOKABLE QBrush disableBorderColor() const;
    void setDisableBorderColor(QBrush newDisableBorderColor);

    const QBrush &clickedHBC() const;
    void setClickedHBC(const QBrush &newClickedHBC);

    const QBrush &hoveredHBC() const;
    void setHoveredHBC(const QBrush &newHoveredHBC);

    const QBrush &hoverBorderHColor() const;
    void setHoverBorderHColor(const QBrush &newHoverBorderHColor);

    const QBrush &clickBorderHColor() const;
    void setClickBorderHColor(const QBrush &newClickBorderHColor);

    const QBrush &normalHBC() const;
    void setNormalHBC(const QBrush &newNormalHBC);

    const QBrush &normalBorderHColor() const;
    void setNormalBorderHColor(const QBrush &newNormalBorderHColor);

    const QBrush &normalCheckedBC() const;
    void setNormalCheckedBC(const QBrush &newNormalCheckedBC);

    const QBrush &clickedCheckedBC() const;
    void setClickedCheckedBC(const QBrush &newClickedCheckedBC);

    const QBrush &hoveredCheckedBC() const;
    void setHoveredCheckedBC(const QBrush &newHoveredCheckedBC);

    const QBrush &normalBorderCheckedColor() const;
    void setNormalBorderCheckedColor(const QBrush &newNormalBorderCheckedColor);

    const QBrush &hoverBorderCheckedColor() const;
    void setHoverBorderCheckedColor(const QBrush &newHoverBorderCheckedColor);

    const QBrush &clickBorderCheckedColor() const;
    void setClickBorderCheckedColor(const QBrush &newClickBorderCheckedColor);

    int implicitHeight() const;
    void setImplicitHeight(int newImplicitHeight);

    int padding() const;
    void setPadding(int newPadding);

    const QBrush &normalHTextColor() const;
    void setNormalHTextColor(const QBrush &newNormalHTextColor);

    const QBrush &disableHTextColor() const;
    void setDisableHTextColor(const QBrush &newDisableHTextColor);

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

    const QBrush &clickedTransparentBorderColor() const;
    void setClickedTransparentBorderColor(const QBrush &newClickedTransparentBorderColor);

    const QBrush &hoveredTransparentBorderColor() const;
    void setHoveredTransparentBorderColor(const QBrush &newHoveredTransparentBorderColor);

    const QBrush &disableTransparentBorderColor() const;
    void setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor);

    const QBrush &disableHBC() const;
    void setDisableHBC(const QBrush &newDisableHBC);

    const QBrush &disableBorderHColor() const;
    void setDisableBorderHColor(const QBrush &newDisableBorderHColor);

    const QBrush &disableCheckedBC() const;
    void setDisableCheckedBC(const QBrush &newDisableCheckedBC);

    const QBrush &disableBorderCheckedColor() const;
    void setDisableBorderCheckedColor(const QBrush &newDisableBorderCheckedColor);

    const QBrush &normalCheckedTransparentBC() const;
    void setNormalCheckedTransparentBC(const QBrush &newNormalCheckedTransparentBC);

    const QBrush &clickedCheckedTransparentBC() const;
    void setClickedCheckedTransparentBC(const QBrush &newClickedCheckedTransparentBC);

    const QBrush &hoveredCheckedTransparentBC() const;
    void setHoveredCheckedTransparentBC(const QBrush &newHoveredCheckedTransparentBC);

    const QBrush &disableCheckedTransparentBC() const;
    void setDisableCheckedTransparentBC(const QBrush &newDisableCheckedTransparentBC);

    const QBrush &normalCheckedTransparentBorderColor() const;
    void setNormalCheckedTransparentBorderColor(const QBrush &newNormalCheckedTransparentBorderColor);

    const QBrush &clickedCheckedTransparentBorderColor() const;
    void setClickedCheckedTransparentBorderColor(const QBrush &newClickedCheckedTransparentBorderColor);

    const QBrush &hoveredCheckedTransparentBorderColor() const;
    void setHoveredCheckedTransparentBorderColor(const QBrush &newHoveredCheckedTransparentBorderColor);

    const QBrush &disableCheckedTransparentBorderColor() const;
    void setDisableCheckedTransparentBorderColor(const QBrush &newDisableCheckedTransparentBorderColor);

    QBrush normalAlternateBC() const;
    void setNormalAlternateBC(const QBrush &newNormalAlternateBC);

    QBrush clickedAlternateBC() const;
    void setClickedAlternateBC(const QBrush &newClickeAlternatedBC);

    QBrush hoveredAlternateBC() const;
    void setHoveredAlternateBC(const QBrush &newHovereAlternatedBC);

    QBrush disableAlternateBC() const;
    void setDisableAlternateBC(const QBrush &newDisableAlternateBC);

    QBrush normalAlternateTransparentBC() const;
    void setNormalAlternateTransparentBC(const QBrush &newNormalAlternateTransparentBC);

    QBrush clickedAlternateTransparentBC() const;
    void setClickedAlternateTransparentBC(const QBrush &newClickedAlternateTransparentBC);

    QBrush hoveredAlternateTransparentBC() const;
    void setHoveredAlternateTransparentBC(const QBrush &newHoveredAlternateTransparentBC);

    QBrush disableAlternateTransparentBC() const;
    void setDisableAlternateTransparentBC(const QBrush &newDisableAlternateTransparentBC);

signals:

    void normalTextColorChanged();

    void disableTextColorChanged();

    void normalBCChanged();

    void clickedBCChanged();

    void hoveredBCChanged();

    void disableBCChanged();

    void radiusChanged();

    void borderWidthChanged();

    void normalBorderColorChanged();

    void hoverBorderColorChanged();

    void clickBorderColorChanged();

    void disableBorderColorChanged();

    void parametryChanged();

    void clickedHBCChanged();

    void hoveredHBCChanged();

    void hoverBorderHColorChanged();

    void clickBorderHColorChanged();

    void normalHBCChanged();

    void normalBorderHColorChanged();

    void normalCheckedBCChanged();

    void clickedCheckedBCChanged();

    void hoveredCheckedBCChanged();

    void normalBorderCheckedColorChanged();

    void hoverBorderCheckedColorChanged();

    void clickBorderCheckedColorChanged();

    void implicitHeightChanged();

    void paddingChanged();

    void normalHTextColorChanged();

    void disableHTextColorChanged();

    void normalTransparentBCChanged();

    void clickedTransparentBCChanged();

    void hoveredTransparentBCChanged();

    void disableTransparentBCChanged();

    void normalTransparentBorderColorChanged();

    void clickedTransparentBorderColorChanged();

    void hoveredTransparentBorderColorChanged();

    void disableTransparentBorderColorChanged();

    void disabledHBCChanged();

    void disabledBorderHColorChanged();

    void disableCheckedBCChanged();

    void disableBorderCheckedColorChanged();

    void normalCheckedTransparentBCChanged();

    void clickedCheckedTransparentBCChanged();

    void hoveredCheckedTransparentBCChanged();

    void disableCheckedTransparentBCChanged();

    void normalCheckedTransparentBorderColorChanged();

    void clickedCheckedTransparentBorderColorChanged();

    void hoveredCheckedTransparentBorderColorChanged();

    void disableCheckedTransparentBorderColorChanged();

    void normalAlternateBCChanged();

    void clickedAlternateBCChanged();

    void hoveredAlternateBCChanged();

    void disableAlternateBCChanged();

    void normalAlternateTransparentBCChanged();

    void clickedAlternateTransparentBCChanged();

    void hoveredAlternateTransparentBCChanged();

    void disableAlternateTransparentBCChanged();

private:
    Q_INVOKABLE QBrush m_normalTextColor    ;
    Q_INVOKABLE QBrush m_disableTextColor   ;
    Q_INVOKABLE QBrush m_normalBC           ;
    Q_INVOKABLE QBrush m_clickedBC          ;
    Q_INVOKABLE QBrush m_hoveredBC          ;
    Q_INVOKABLE QBrush m_disableBC          ;
    Q_INVOKABLE QBrush m_normalBorderColor  ;
    Q_INVOKABLE QBrush m_hoverBorderColor   ;
    Q_INVOKABLE QBrush m_clickBorderColor   ;
    Q_INVOKABLE QBrush m_disableBorderColor ;
    int m_radius;
    int m_borderWidth;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_clickedHBC;
    QBrush m_hoveredHBC;
    QBrush m_hoverBorderHColor;
    QBrush m_clickBorderHColor;
    QBrush m_normalHBC;
    QBrush m_normalBorderHColor;
    QBrush m_normalCheckedBC;
    QBrush m_clickedCheckedBC;
    QBrush m_hoveredCheckedBC;
    QBrush m_normalBorderCheckedColor;
    QBrush m_hoverBorderCheckedColor;
    QBrush m_clickBorderCheckedColor;
    int m_implicitHeight = 36;
    int m_padding;
    QBrush m_normalHTextColor;
    QBrush m_disableHTextColor;
    QBrush m_normalTransparentBC;
    QBrush m_clickedTransparentBC;
    QBrush m_hoveredTransparentBC;
    QBrush m_disableTransparentBC;
    QBrush m_normalTransparentBorderColor;
    QBrush m_clickedTransparentBorderColor;
    QBrush m_hoveredTransparentBorderColor;
    QBrush m_disableTransparentBorderColor;
    QBrush m_disableHBC;
    QBrush m_disableBorderHColor;
    QBrush m_disableCheckedBC;
    QBrush m_disableBorderCheckedColor;
    QBrush m_normalCheckedTransparentBC;
    QBrush m_clickedCheckedTransparentBC;
    QBrush m_hoveredCheckedTransparentBC;
    QBrush m_disableCheckedTransparentBC;
    QBrush m_normalCheckedTransparentBorderColor;
    QBrush m_clickedCheckedTransparentBorderColor;
    QBrush m_hoveredCheckedTransparentBorderColor;
    QBrush m_disableCheckedTransparentBorderColor;
    QBrush m_normalAlternateBC;
    QBrush m_clickedAlternatedBC;
    QBrush m_hoveredAlternatedBC;
    QBrush m_disableAlternateBC;
    QBrush m_normalAlternateTransparentBC;
    QBrush m_clickedAlternateTransparentBC;
    QBrush m_hoveredAlternateTransparentBC;
    QBrush m_disableAlternateTransparentBC;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIItemDelegate, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUIITEMDELEGATE_H
