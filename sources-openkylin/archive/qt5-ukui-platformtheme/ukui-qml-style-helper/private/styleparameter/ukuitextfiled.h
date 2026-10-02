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

#ifndef UKUITEXTFILED_H
#define UKUITEXTFILED_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUITextFiled : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(double leftRightPadding READ leftRightPadding WRITE setLeftRightPadding NOTIFY leftRightPaddingChanged)
    Q_PROPERTY(double upDownMargin READ upDownMargin WRITE setUpDownMargin NOTIFY upDownMarginChanged)
    Q_PROPERTY(double space READ space WRITE setSpace NOTIFY spaceChange)

    Q_PROPERTY(int normalWidth READ normalWidth WRITE setNormalWidth NOTIFY normalWidthChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setNormalHeight NOTIFY normalHeightChanged)
    Q_PROPERTY(QBrush normalBC READ normalBC WRITE setNormalBC NOTIFY normalBCChanged)
    Q_PROPERTY(QBrush inputNormalBC READ inputNormalBC WRITE setInputNormalBC NOTIFY inputNormalBCChanged)

    Q_PROPERTY(QBrush clickedBC READ clickedBC WRITE setClickedBC NOTIFY clickedBCChanged)
    Q_PROPERTY(QBrush hoveredBC READ hoveredBC WRITE setHoveredBC NOTIFY hoveredBCChanged)
    Q_PROPERTY(QBrush disableBC READ disableBC WRITE setDisableBC NOTIFY disableBCChanged)

    Q_PROPERTY(QBrush placeHolderNormalTextColor READ placeHolderNormalTextColor WRITE setPlaceHolderNormalTextColor NOTIFY placeHolderNormalTextColorChanged)
    Q_PROPERTY(QBrush placeHolderDisableTextColor READ placeHolderDisableTextColor WRITE setPlaceHolderDisableTextColor NOTIFY placeHolderDisableTextColorChanged)
    Q_PROPERTY(QBrush disableTextColor READ disableTextColor WRITE setDisableTextColor NOTIFY disableTextColorChanged)
    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY normalTextColorChanged)

    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(int focusBorderWidth READ focusBorderWidth WRITE setFocusBorderWidth NOTIFY focusBorderWidthChanged)
    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoverBorderColor READ hoverBorderColor WRITE setHoverBorderColor NOTIFY hoverBorderColorChanged)

    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickBorderColorChanged)
    Q_PROPERTY(QBrush disableBorderColor READ disableBorderColor WRITE setDisableBorderColor NOTIFY disableBorderColorChanged)
    Q_PROPERTY(QBrush focusBorderColor READ focusBorderColor WRITE setFocusBorderColor NOTIFY focusBorderColorChanged)

    Q_PROPERTY(QBrush normalTransparentBC READ normalTransparentBC WRITE setNormalTransparentBC NOTIFY normalTransparentBCChanged)
    Q_PROPERTY(QBrush inputNormalTransparentBC READ inputNormalTransparentBC WRITE setInputNormalTransparentBC NOTIFY inputNormalTransparentBCChanged)
    Q_PROPERTY(QBrush clickedTransparentBC READ clickedTransparentBC WRITE setClickedTransparentBC NOTIFY clickedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredTransparentBC READ hoveredTransparentBC WRITE setHoveredTransparentBC NOTIFY hoveredTransparentBCChanged)
    Q_PROPERTY(QBrush disableTransparentBC READ disableTransparentBC WRITE setDisableTransparentBC NOTIFY disableTransparentBCChanged)
    
public:
    explicit UKUITextFiled(QQuickItem *parent = nullptr);
    ~UKUITextFiled();

    static UKUITextFiled* qmlAttachedProperties(QObject* parent);

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

    int focusBorderWidth() const;
    void setFocusBorderWidth(int newFocusBorderWidth);

    const QBrush &normalBorderColor() const;
    void setNormalBorderColor(const QBrush &newNormalBorderColor);

    const QBrush &clickBorderColor() const;
    void setClickBorderColor(const QBrush &newClickBorderColor);

    const QBrush &disableBorderColor() const;
    void setDisableBorderColor(const QBrush &newDisableBorderColor);

    const QBrush &focusBorderColor() const;
    void setFocusBorderColor(const QBrush &newFocusBorderColor);

    const QBrush &hoverBorderColor() const;
    void setHoverBorderColor(const QBrush &newHoverBorderColor);

    const QBrush &placeHolderNormalTextColor() const;
    void setPlaceHolderNormalTextColor(const QBrush &newPlaceHolderNormalTextColor);

    const QBrush &placeHolderDisableTextColor() const;
    void setPlaceHolderDisableTextColor(const QBrush &newPlaceHolderDisableTextColor);

    const QBrush &inputNormalBC() const;
    void setInputNormalBC(const QBrush &newInputNormalBC);
    
    const QBrush &normalTransparentBC() const;
    void setNormalTransparentBC(const QBrush &newNormalTransparentBC);
    
    const QBrush &inputNormalTransparentBC() const;
    void setInputNormalTransparentBC(const QBrush &newInputNormalTransparentBC);
    
    const QBrush &clickedTransparentBC() const;
    void setClickedTransparentBC(const QBrush &newClickedTransparentBC);
    
    const QBrush &hoveredTransparentBC() const;
    void setHoveredTransparentBC(const QBrush &newHoveredTransparentBC);
    
    const QBrush &disableTransparentBC() const;
    void setDisableTransparentBC(const QBrush &newDisableTransparentBC);
    
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

    void hoverBorderColorChanged();

    void placeHolderNormalTextColorChanged();

    void placeHolderDisableTextColorChanged();

    void inputNormalBCChanged();

    void parametryChanged();
    
    void normalTransparentBCChanged();
    
    void inputNormalTransparentBCChanged();
    
    void clickedTransparentBCChanged();
    
    void hoveredTransparentBCChanged();
    
    void disableTransparentBCChanged();
    
private:

    double m_radius;
    double m_leftRightPadding;
    double m_upDownMargin;
    double m_space;
    int m_normalWidth;
    int m_normalHeight;
    Q_INVOKABLE QBrush m_normalBC;
    Q_INVOKABLE QBrush m_clickedBC;
    Q_INVOKABLE QBrush m_hoveredBC;
    Q_INVOKABLE QBrush m_disableBC;
    Q_INVOKABLE QBrush m_disableTextColor;
    Q_INVOKABLE QBrush m_normalTextColor;
    int m_borderWidth;
    int m_focusBorderWidth;
    Q_INVOKABLE QBrush m_normalBorderColor;
    Q_INVOKABLE QBrush m_clickBorderColor;
    Q_INVOKABLE QBrush m_disableBorderColor;
    Q_INVOKABLE QBrush m_focusBorderColor;
    Q_INVOKABLE QBrush m_hoverBorderColor;
    Q_INVOKABLE QBrush m_placeHolderNormalTextColor;
    Q_INVOKABLE QBrush m_placeHolderDisableTextColor;
    Q_INVOKABLE QBrush m_inputNormalBC;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;
    
    QBrush m_normalTransparentBC;
    QBrush m_inputNormalTransparentBC;
    QBrush m_clickedTransparentBC;
    QBrush m_hoveredTransparentBC;
    QBrush m_disableTransparentBC;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUITextFiled, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUITEXTFILED_H
