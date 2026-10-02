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

#ifndef UKUIBUTTON_H
#define UKUIBUTTON_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QQuickItem>
#include "tokenparameter.h"
#include "parsecolorinterface.h"

using namespace UKUIGlobalDTConfig;
namespace UKUIQQC2Style {

class UKUIButton : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(double leftRightMargin READ leftRightMargin WRITE setLeftRightMargin NOTIFY leftRightMarginChanged)
    Q_PROPERTY(double upDownMargin READ upDownMargin WRITE setUpDownMargin NOTIFY upDownMarginChanged)
    Q_PROPERTY(double space READ space WRITE setSpace NOTIFY spaceChanged)
    Q_PROPERTY(int normalWidth READ normalWidth WRITE setNormalWidth NOTIFY normalWidthChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setNormalHeight NOTIFY normalHeightChanged)
    Q_PROPERTY(QBrush normalBC READ normalBC WRITE setNormalBC NOTIFY normalBCChanged)
    Q_PROPERTY(QBrush clickedBC READ clickedBC WRITE setClickedBC NOTIFY clickedBCChanged)
    Q_PROPERTY(QBrush hoveredBC READ hoveredBC WRITE setHoveredBC NOTIFY hoveredBCChanged)
    Q_PROPERTY(QBrush disableBC READ disableBC WRITE setDisableBC NOTIFY disableBCChanged)
    Q_PROPERTY(QBrush disableTextColor READ disableTextColor WRITE setDisableTextColor NOTIFY disableTextColorChanged)
    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY normalTextColorChanged)
    Q_PROPERTY(QBrush highlightTextColor READ highlightTextColor WRITE setHighlightTextColor NOTIFY highlightTextColorChanged)
    Q_PROPERTY(QBrush highlightTextDisableColor READ highlightTextDisableColor WRITE setHighlightTextDisableColor NOTIFY highlightTextDisableColorChanged)


    Q_PROPERTY(QBrush normalHBC READ normalHBC WRITE setNormalHBC NOTIFY normalHBCChanged)
    Q_PROPERTY(QBrush clickedHBC READ clickedHBC WRITE setClickedHBC NOTIFY clickedHBCChanged)
    Q_PROPERTY(QBrush hoveredHBC READ hoveredHBC WRITE setHoveredHBC NOTIFY hoveredHBCChanged)
    Q_PROPERTY(QBrush disableHBC READ disableHBC WRITE setDisableHBC NOTIFY disableHBCChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int focusBorderWidth READ focusBorderWidth WRITE setFocusBorderWidth NOTIFY focusBorderWidthChanged)
    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoverBorderColor READ hoverBorderColor WRITE setHoverBorderColor NOTIFY hoverBorderColorChanged)
    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickBorderColorChanged)
    Q_PROPERTY(QBrush disableBorderColor READ disableBorderColor WRITE setDisableBorderColor NOTIFY disableBorderColorChanged)
    Q_PROPERTY(QBrush focusBorderColor READ focusBorderColor WRITE setFocusBorderColor NOTIFY focusBorderColorChanged)

    Q_PROPERTY(QBrush normalBorderHColor READ normalBorderHColor WRITE setNormalBorderHColor NOTIFY normalBorderHColorChanged)
    Q_PROPERTY(QBrush hoverBorderHColor READ hoverBorderHColor WRITE setHoverBorderHColor NOTIFY hoverBorderHColorChanged)
    Q_PROPERTY(QBrush clickBorderHColor READ clickBorderHColor WRITE setClickBorderHColor NOTIFY clickBorderHColorChanged)
    Q_PROPERTY(QBrush disableBorderHColor READ disableBorderHColor WRITE setDisableBorderHColor NOTIFY disableBorderHColorChanged)


    Q_PROPERTY(QBrush normalCloseBC READ normalCloseBC WRITE setNormalCloseBC NOTIFY normalCloseBCChanged)
    Q_PROPERTY(QBrush clickedCloseBC READ clickedCloseBC WRITE setClickedCloseBC NOTIFY clickedCloseBCChanged)
    Q_PROPERTY(QBrush hoveredCloseBC READ hoveredCloseBC WRITE setHoveredClosedBC NOTIFY hoveredCloseBCChanged)
    Q_PROPERTY(QBrush disableCloseBC READ disableCloseBC WRITE setDisableCloseBC NOTIFY disableCloseBCChanged)
    Q_PROPERTY(QBrush normalBorderCloseColor READ normalBorderCloseColor WRITE setNormalBorderCloseColor NOTIFY normalBorderCloseColorChanged)
    Q_PROPERTY(QBrush hoverBorderCloseColor READ hoverBorderCloseColor WRITE setHoverBorderCloseColor NOTIFY hoverBorderCloseColorChanged)
    Q_PROPERTY(QBrush clickBorderCloseColor READ clickBorderCloseColor WRITE setClickBorderCloseColor NOTIFY clickBorderCloseColorChanged)
    Q_PROPERTY(QBrush disableBorderCloseColor READ disableBorderCloseColor WRITE setDisableBorderCloseColor NOTIFY disableBorderCloseColorChanged)


    Q_PROPERTY(QBrush normalWindowBC READ normalWindowBC WRITE setNormalWindowBC NOTIFY normalWindowBCChanged)
    Q_PROPERTY(QBrush clickedWindowBC READ clickedWindowBC WRITE setClickedWindowBC NOTIFY clickedWindowBCChanged)
    Q_PROPERTY(QBrush hoveredWindowBC READ hoveredWindowBC WRITE setHoveredWindowBC NOTIFY hoveredWindowBCChanged)
    Q_PROPERTY(QBrush disableWindowBC READ disableWindowBC WRITE setDisableWindowBC NOTIFY disableWindowBCChanged)
    Q_PROPERTY(QBrush normalBorderWindowColor READ normalBorderWindowColor WRITE setNormalBorderWindowColor NOTIFY normalBorderWindowColorChanged)
    Q_PROPERTY(QBrush hoverBorderWindowColor READ hoverBorderWindowColor WRITE setHoverBorderWindowColor NOTIFY hoverBorderWindowColorChanged)
    Q_PROPERTY(QBrush clickBorderWindowColor READ clickBorderWindowColor WRITE setClickBorderWindowColor NOTIFY clickBorderWindowColorChanged)
    Q_PROPERTY(QBrush disableBorderWindowColor READ disableBorderWindowColor WRITE setDisableBorderWindowColor NOTIFY disableBorderWindowColorChanged)


    Q_PROPERTY(QBrush normalTransparentBC  READ  normalTransparentBC WRITE  setNormalTransparentBC NOTIFY  normalTransparentBCChanged)
    Q_PROPERTY(QBrush clickedTransparentBC READ clickedTransparentBC WRITE setClickedTransparentBC NOTIFY clickedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredTransparentBC READ hoveredTransparentBC WRITE setHoveredTransparentBC NOTIFY hoveredTransparentBCChanged)
    Q_PROPERTY(QBrush disableTransparentBC READ disableTransparentBC WRITE setDisableTransparentBC NOTIFY disableTransparentBCChanged)
    Q_PROPERTY(QBrush  normalBorderTransparentBC READ  normalBorderTransparentBC WRITE  setNormalBorderTransparentBC NOTIFY  normalBorderTransparentBCChanged)
    Q_PROPERTY(QBrush clickedBorderTransparentBC READ clickedBorderTransparentBC WRITE setClickedBorderTransparentBC NOTIFY clickedBorderTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredBorderTransparentBC READ hoveredBorderTransparentBC WRITE setHoveredBorderTransparentBC NOTIFY hoveredBorderTransparentBCChanged)
    Q_PROPERTY(QBrush disableBorderTransparentBC READ disableBorderTransparentBC WRITE setDisableBorderTransparentBC NOTIFY disableBorderTransparentBCChanged)

    Q_PROPERTY(QBrush  normalWindowTransparentBC READ  normalWindowTransparentBC WRITE  setNormalWindowTransparentBC NOTIFY  normalWindowTransparentBCChanged)
    Q_PROPERTY(QBrush clickedWindowTransparentBC READ clickedWindowTransparentBC WRITE setClickedWindowTransparentBC NOTIFY clickedWindowTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredWindowTransparentBC READ hoveredWindowTransparentBC WRITE setHoveredWindowTransparentBC NOTIFY hoveredWindowTransparentBCChanged)
    Q_PROPERTY(QBrush disableWindowTransparentBC READ disableWindowTransparentBC WRITE setDisableWindowTransparentBC NOTIFY disableWindowTransparentBCChanged)
    Q_PROPERTY(QBrush  normalBorderWindowTransparentColor READ  normalBorderWindowTransparentColor WRITE  setNormalBorderWindowTransparentColor NOTIFY  normalBorderWindowTransparentColorChanged)
    Q_PROPERTY(QBrush hoveredBorderWindowTransparentColor READ hoveredBorderWindowTransparentColor WRITE setHoveredBorderWindowTransparentColor NOTIFY hoveredBorderWindowTransparentColorChanged)
    Q_PROPERTY(QBrush clickedBorderWindowTransparentColor READ clickedBorderWindowTransparentColor WRITE setClickedBorderWindowTransparentColor NOTIFY clickedBorderWindowTransparentColorChanged)
    Q_PROPERTY(QBrush disableBorderWindowTransparentColor READ disableBorderWindowTransparentColor WRITE setDisableBorderWindowTransparentColor NOTIFY disableBorderWindowTransparentColorChanged)

public:
    explicit UKUIButton(QQuickItem *parent = nullptr);
    ~UKUIButton();

    static UKUIButton* qmlAttachedProperties(QObject* parent);

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);
    double radius() const;
    void setRadius(double newRadius);

    double leftRightMargin() const;
    void setLeftRightMargin(double newMargin);

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

    Q_INVOKABLE QBrush normalHBC() const;
    void setNormalHBC(QBrush newNormalHBC);

    Q_INVOKABLE QBrush clickedHBC() const;
    void setClickedHBC(QBrush newClickedHBC);

    Q_INVOKABLE QBrush hoveredHBC();
    void setHoveredHBC(QBrush newHoveredHBC);

    Q_INVOKABLE QBrush disableHBC() const;
    void setDisableHBC(QBrush newDisableHBC);
    
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
    
    Q_INVOKABLE QBrush normalBorderHColor() const;
    void setNormalBorderHColor(QBrush newNormalBorderHColor);

    Q_INVOKABLE QBrush hoverBorderHColor() const;
    void setHoverBorderHColor(QBrush newHoverBorderHColor);

    Q_INVOKABLE QBrush clickBorderHColor() const;
    void setClickBorderHColor(QBrush newClickBorderHColor);

    Q_INVOKABLE QBrush disableBorderHColor() const;
    void setDisableBorderHColor(QBrush newDisableBorderHColor);

    Q_INVOKABLE QBrush focusBorderColor() const;
    void setFocusBorderColor(QBrush newFocusBorderColor);

    int focusBorderWidth() const;
    void setFocusBorderWidth(int newFocusBorderWidth);

    Q_INVOKABLE QBrush highlightTextColor() const;
    void setHighlightTextColor(QBrush newHighlightTextColor);

    Q_INVOKABLE QBrush highlightTextDisableColor() const;
    void setHighlightTextDisableColor(QBrush newHighlightTextDisableColor);

    const QBrush &normalCloseBC() const;
    void setNormalCloseBC(const QBrush &newNormalCloseBC);

    const QBrush &clickedCloseBC() const;
    void setClickedCloseBC(const QBrush &newClickedCloseBC);

    const QBrush &hoveredCloseBC() const;
    void setHoveredClosedBC(const QBrush &newHoveredCloseBC);

    const QBrush &disableCloseBC() const;
    void setDisableCloseBC(const QBrush &newDisableCloseBC);

    const QBrush &normalBorderCloseColor() const;
    void setNormalBorderCloseColor(const QBrush &newNormalBorderCloseColor);

    const QBrush &hoverBorderCloseColor() const;
    void setHoverBorderCloseColor(const QBrush &newHoverBorderCloseColor);

    const QBrush &clickBorderCloseColor() const;
    void setClickBorderCloseColor(const QBrush &newClickBorderCloseColor);

    const QBrush &disableBorderCloseColor() const;
    void setDisableBorderCloseColor(const QBrush &newDisableBorderCloseColor);

    const QBrush &normalWindowBC() const;
    void setNormalWindowBC(const QBrush &newNormalWindowBC);

    const QBrush &clickedWindowBC() const;
    void setClickedWindowBC(const QBrush &newClickedWindowBC);

    const QBrush &hoveredWindowBC() const;
    void setHoveredWindowBC(const QBrush &newHoveredWindowBC);

    const QBrush &disableWindowBC() const;
    void setDisableWindowBC(const QBrush &newDisableWindowBC);

    const QBrush &normalBorderWindowColor() const;
    void setNormalBorderWindowColor(const QBrush &newNormalBorderWindowColor);

    const QBrush &hoverBorderWindowColor() const;
    void setHoverBorderWindowColor(const QBrush &newHoverBorderWindowColor);

    const QBrush &clickBorderWindowColor() const;
    void setClickBorderWindowColor(const QBrush &newClickBorderWindowColor);

    const QBrush &disableBorderWindowColor() const;
    void setDisableBorderWindowColor(const QBrush &newDisableBorderWindowColor);

    const QBrush &normalTransparentBC() const;
    void setNormalTransparentBC(const QBrush &newNormalTransparentBC);

    const QBrush &clickedTransparentBC() const;
    void setClickedTransparentBC(const QBrush &newClickedTransparentBC);

    const QBrush &hoveredTransparentBC() const;
    void setHoveredTransparentBC(const QBrush &newHoveredTransparentBC);

    const QBrush &disableTransparentBC() const;
    void setDisableTransparentBC(const QBrush &newDisableTransparentBC);

    const QBrush &normalBorderTransparentBC() const;
    void setNormalBorderTransparentBC(const QBrush &newNormalBorderTransparentBC);

    const QBrush &clickedBorderTransparentBC() const;
    void setClickedBorderTransparentBC(const QBrush &newClickedBorderTransparentBC);

    const QBrush &hoveredBorderTransparentBC() const;
    void setHoveredBorderTransparentBC(const QBrush &newHoveredBorderTransparentBC);

    const QBrush &disableBorderTransparentBC() const;
    void setDisableBorderTransparentBC(const QBrush &newDisableBorderTransparentBC);

    const QBrush &normalWindowTransparentBC() const;
    void setNormalWindowTransparentBC(const QBrush &newNormalWindowTransparentBC);

    const QBrush &clickedWindowTransparentBC() const;
    void setClickedWindowTransparentBC(const QBrush &newClickedWindowTransparentBC);

    const QBrush &hoveredWindowTransparentBC() const;
    void setHoveredWindowTransparentBC(const QBrush &newHoveredWindowTransparentBC);

    const QBrush &disableWindowTransparentBC() const;
    void setDisableWindowTransparentBC(const QBrush &newDisableWindowTransparentBC);

    const QBrush &normalBorderWindowTransparentColor() const;
    void setNormalBorderWindowTransparentColor(const QBrush &newNormalBorderWindowTransparentColor);

    const QBrush &hoveredBorderWindowTransparentColor() const;
    void setHoveredBorderWindowTransparentColor(const QBrush &newHoveredBorderWindowTransparentColor);

    const QBrush &clickedBorderWindowTransparentColor() const;
    void setClickedBorderWindowTransparentColor(const QBrush &newClickedBorderWindowTransparentColor);

    const QBrush &disableBorderWindowTransparentColor() const;
    void setDisableBorderWindowTransparentColor(const QBrush &newDisableBorderWindowTransparentColor);

signals:
    void radiusChanged();

    void leftRightMarginChanged();

    void upDownMarginChanged();

    void spaceChanged();

    void normalWidthChanged();

    void normalHeightChanged();

    void normalBCChanged();

    void clickedBCChanged();

    void hoveredBCChanged();

    void disableBCChanged();

    void disableTextColorChanged();

    void normalHBCChanged();

    void clickedHBCChanged();

    void hoveredHBCChanged();

    void disableHBCChanged();
    
    void borderWidthChanged();
    
    void normalBorderColorChanged();
    
    void hoverBorderColorChanged();
    
    void clickBorderColorChanged();
    
    void disableBorderColorChanged();
    
    void normalBorderHColorChanged();

    void hoverBorderHColorChanged();

    void clickBorderHColorChanged();

    void disableBorderHColorChanged();

    void focusBorderColorChanged();

    void focusBorderWidthChanged();

    void normalTextColorChanged();

    void highlightTextColorChanged();

    void highlightTextDisableColorChanged();

    void parametryChanged();

    void normalCloseBCChanged();

    void clickedCloseBCChanged();

    void hoveredCloseBCChanged();

    void disableCloseBCChanged();

    void normalBorderCloseColorChanged();

    void hoverBorderCloseColorChanged();

    void clickBorderCloseColorChanged();

    void disableBorderCloseColorChanged();

    void normalWindowBCChanged();

    void clickedWindowBCChanged();

    void hoveredWindowBCChanged();

    void disableWindowBCChanged();

    void normalBorderWindowColorChanged();

    void hoverBorderWindowColorChanged();

    void clickBorderWindowColorChanged();

    void disableBorderWindowColorChanged();

    void normalTransparentBCChanged();

    void clickedTransparentBCChanged();

    void hoveredTransparentBCChanged();

    void disableTransparentBCChanged();

    void normalBorderTransparentBCChanged();

    void clickedBorderTransparentBCChanged();

    void hoveredBorderTransparentBCChanged();

    void disableBorderTransparentBCChanged();

    void normalWindowTransparentBCChanged();

    void clickedWindowTransparentBCChanged();

    void hoveredWindowTransparentBCChanged();

    void disableWindowTransparentBCChanged();

    void normalBorderWindowTransparentColorChanged();

    void hoveredBorderWindowTransparentColorChanged();

    void clickedBorderWindowTransparentColorChanged();

    void disableBorderWindowTransparentColorChanged();

private:
    double m_radius = 6.0;
    double m_leftRightMargin = 8.0;
    double m_upDownMargin = 4.0;
    double m_space = 4.0;
    int m_normalWidth = 96;
    int m_normalHeight = 36;
    int m_borderWidth;
    int m_focusBorderWidth;

    QBrush m_normalBC            ;
    QBrush m_clickedBC           ;
    QBrush m_hoveredBC           ;
    QBrush m_disableBC           ;
    QBrush m_disableTextColor    ;
    QBrush m_normalTextColor     ;
    QBrush m_normalHBC           ;
    QBrush m_clickedHBC          ;
    QBrush m_hoveredHBC          ;
    QBrush m_disableHBC          ;
    QBrush m_normalBorderColor   ;
    QBrush m_hoverBorderColor    ;
    QBrush m_clickBorderColor    ;
    QBrush m_disableBorderColor  ;
    QBrush m_normalBorderHColor  ;
    QBrush m_hoverBorderHColor   ;
    QBrush m_clickBorderHColor   ;
    QBrush m_disableBorderHColor ;
    QBrush m_focusBorderColor    ;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance  = nullptr;
    QBrush m_highlightTextColor;
    QBrush m_highlightTextDisableColor;
    QBrush m_normalCloseBC;
    QBrush m_clickedCloseBC;
    QBrush m_hoveredCloseBC;
    QBrush m_disableCloseBC;
    QBrush m_normalBorderCloseColor;
    QBrush m_hoverBorderCloseColor;
    QBrush m_clickBorderCloseColor;
    QBrush m_disableBorderCloseColor;
    QBrush m_normalWindowBC;
    QBrush m_clickedWindowBC;
    QBrush m_hoveredWindowBC;
    QBrush m_disableWindowBC;
    QBrush m_normalBorderWindowColor;
    QBrush m_hoverBorderWindowColor;
    QBrush m_clickBorderWindowColor;
    QBrush m_disableBorderWindowColor;
    QBrush m_normalTransparentBC;
    QBrush m_clickedTransparentBC;
    QBrush m_hoveredTransparentBC;
    QBrush m_disableTransparentBC;
    QBrush m_normalBorderTransparentBC;
    QBrush m_clickedBorderTransparentBC;
    QBrush m_hoveredBorderTransparentBC;
    QBrush m_disableBorderTransparentBC;
    QBrush m_normalWindowTransparentBC;
    QBrush m_clickedWindowTransparentBC;
    QBrush m_hoveredWindowTransparentBC;
    QBrush m_disableWindowTransparentBC;
    QBrush m_normalBorderWindowTransparentColor;
    QBrush m_hoveredBorderWindowTransparentColor;
    QBrush m_clickedBorderWindowTransparentColor;
    QBrush m_disableBorderWindowTransparentColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIButton, QML_HAS_ATTACHED_PROPERTIES)

//Q_DECLARE_METATYPE(UKUIQQC2Style::UKUIButton);
//Q_DECLARE_METATYPE(std::shared_ptr<UKUIQQC2Style::UKUIButton>)

#endif // UKUIBUTTON_H
