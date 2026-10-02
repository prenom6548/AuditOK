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

#ifndef UKUISPINBOX_H
#define UKUISPINBOX_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUISpinBox : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QBrush normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)
    Q_PROPERTY(QBrush hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(QBrush clickColor READ clickColor WRITE setClickColor NOTIFY clickColorChanged)
    Q_PROPERTY(QBrush disableColor READ disableColor WRITE setDisableColor NOTIFY disableColorChanged)
    Q_PROPERTY(QBrush focusColor READ focusColor WRITE setFocusColor NOTIFY focusColorChanged)

    Q_PROPERTY(QBrush normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(QBrush hoverBorderColor READ hoverBorderColor WRITE setHoverBorderColor NOTIFY hoverColorBorderChanged)
    Q_PROPERTY(QBrush clickBorderColor READ clickBorderColor WRITE setClickBorderColor NOTIFY clickColorBorderChanged)
    Q_PROPERTY(QBrush disableBorderColor READ disableBorderColor WRITE setDisableBorderColor NOTIFY disableBorderColorChanged)
    Q_PROPERTY(QBrush focusBorderColor READ focusBorderColor WRITE setFocusBorderColor NOTIFY focusBorderColorChanged)

    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY normalTextColorChanged)
    Q_PROPERTY(QBrush disableTextColor READ disableTextColor WRITE setDisableTextColor NOTIFY disableTextColorChanged)

    Q_PROPERTY(QBrush btnNormalColor READ btnNormalColor WRITE setBtnNormalColor NOTIFY btnNormalColorChanged)
    Q_PROPERTY(QBrush btnHoverColor READ btnHoverColor WRITE setBtnHoverColor NOTIFY btnHoverColorChanged)
    Q_PROPERTY(QBrush btnClickColor READ btnClickColor WRITE setBtnClickColor NOTIFY btnClickColorChanged)
    Q_PROPERTY(QBrush btnDisableColor READ btnDisableColor WRITE setBtnDisableColor NOTIFY btnDisableColorChanged)
    Q_PROPERTY(QBrush btnFocusColor READ btnFocusColor WRITE setBtnFocusColor NOTIFY btnFocusColorChanged)

    Q_PROPERTY(QBrush btnNormalBorderColor READ btnNormalBorderColor WRITE setBtnNormalBorderColor NOTIFY btnNormalBorderColorChanged)
    Q_PROPERTY(QBrush btnHoverBorderColor READ btnHoverBorderColor WRITE setBtnHoverBorderColor NOTIFY btnHoverBorderColorChanged)
    Q_PROPERTY(QBrush btnClickBorderColor READ btnClickBorderColor WRITE setBtnClickBorderColor NOTIFY btnClickBorderColorChanged)
    Q_PROPERTY(QBrush btnDisableBorderColor READ btnDisableBorderColor WRITE setBtnDisableBorderColor NOTIFY btnDisableBorderColorChanged)
    Q_PROPERTY(QBrush btnFocusBorderColor READ btnFocusBorderColor WRITE setBtnFocusBorderColor NOTIFY btnFocusBorderColorChanged)

    Q_PROPERTY(QBrush  normalTransparentBC READ  normalTransparentBC WRITE  setNormalTransparentBC NOTIFY  normalTransparentBCChanged)
    Q_PROPERTY(QBrush clickedTransparentBC READ clickedTransparentBC WRITE setClickedTransparentBC NOTIFY clickedTransparentBCChanged)
    Q_PROPERTY(QBrush hoveredTransparentBC READ hoveredTransparentBC WRITE setHoveredTransparentBC NOTIFY hoveredTransparentBCChanged)
    Q_PROPERTY(QBrush disableTransparentBC READ disableTransparentBC WRITE setDisableTransparentBC NOTIFY disableTransparentBCChanged)

    Q_PROPERTY(QBrush  normalTransparentBorderColor READ  normalTransparentBorderColor WRITE  setNormalTransparentBorderColor NOTIFY  normalTransparentBorderColorChanged)
    Q_PROPERTY(QBrush clickedTransparentBorderColor READ clickedTransparentBorderColor WRITE setClickedTransparentBorderColor NOTIFY clickedTransparentBorderColorChanged)
    Q_PROPERTY(QBrush hoveredTransparentBorderColor READ hoveredTransparentBorderColor WRITE setHoveredTransparentBorderColor NOTIFY hoveredTransparentBorderColorChanged)
    Q_PROPERTY(QBrush disableTransparentBorderColor READ disableTransparentBorderColor WRITE setDisableTransparentBorderColor NOTIFY disableTransparentBorderColorChanged)

    Q_PROPERTY(QBrush btnNormalTransparentColor READ btnNormalTransparentColor WRITE setBtnNormalTransparentColor NOTIFY btnNormalTransparentColorChanged)
    Q_PROPERTY(QBrush btnHoverTransparentColor READ btnHoverTransparentColor WRITE setBtnHoverTransparentColor NOTIFY btnHoverTransparentColorChanged)
    Q_PROPERTY(QBrush btnClickTransparentColor READ btnClickTransparentColor WRITE setBtnClickTransparentColor NOTIFY btnClickTransparentColorChanged)
    Q_PROPERTY(QBrush btnDisableTransparentColor READ btnDisableTransparentColor WRITE setBtnDisableTransparentColor NOTIFY btnDisableTransparentColorChanged)

    Q_PROPERTY(QBrush btnNormalTransparentBorderColor READ btnNormalTransparentBorderColor WRITE setBtnNormalTransparentBorderColor NOTIFY btnNormalTransparentBorderColorChanged)
    Q_PROPERTY(QBrush btnHoverTransparentBorderColor READ btnHoverTransparentBorderColor WRITE setBtnHoverTransparentBorderColor NOTIFY btnHoverTransparentBorderColorChanged)
    Q_PROPERTY(QBrush btnClickTransparentBorderColor READ btnClickTransparentBorderColor WRITE setBtnClickTransparentBorderColor NOTIFY btnClickTransparentBorderColorChanged)
    Q_PROPERTY(QBrush btnDisableTransparentBorderColor READ btnDisableTransparentBorderColor WRITE setBtnDisableTransparentBorderColor NOTIFY btnDisableTransparentBorderColorChanged)

    Q_PROPERTY(int normalWidth READ normalWidth WRITE setNormalWidth NOTIFY normalWidthChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setNormalHeight NOTIFY normalHeightChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(int btnNormalWidth READ btnNormalWidth WRITE setBtnNormalWidth NOTIFY btnNormalWidthChanged)
    Q_PROPERTY(int btnNormalHeight READ btnNormalHeight WRITE setBtnNormalHeight NOTIFY btnNormalHeightChanged)
    Q_PROPERTY(int focusBorderWidth READ focusBorderWidth WRITE setFocusBorderWidth NOTIFY focusBorderWidthChanged)
    Q_PROPERTY(int normalBorderWidth READ normalBorderWidth WRITE setNormalBorderWidth NOTIFY normalBorderWidthChanged)

    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)

    Q_PROPERTY(int btnBorderWidth READ btnBorderWidth WRITE setBtnBorderWidth NOTIFY btnBorderWidthChanged)

public:
    explicit UKUISpinBox(QQuickItem *parent = nullptr);
    ~UKUISpinBox();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUISpinBox* qmlAttachedProperties(QObject* parent);

    const QBrush &normalColor() const;
    void setNormalColor(const QBrush &newNormalColor);

    const QBrush &disableColor() const;
    void setDisableColor(const QBrush &newDisableColor);

    const QBrush &hoverColor() const;
    void setHoverColor(const QBrush &newHoverColor);

    const QBrush &clickColor() const;
    void setClickColor(const QBrush &newClickColor);

    const QBrush &normalTextColor() const;
    void setNormalTextColor(const QBrush &newNormalTextColor);

    const QBrush &disableTextColor() const;
    void setDisableTextColor(const QBrush &newDisableTextColor);

    int normalWidth() const;
    void setNormalWidth(int newNormalWidth);

    int normalHeight() const;
    void setNormalHeight(int newNormalHeight);

    int radius() const;
    void setRadius(int newRadius);

    int btnNormalWidth() const;
    void setBtnNormalWidth(int newBtnNormalWidth);

    int btnNormalHeight() const;
    void setBtnNormalHeight(int newBtnNormalHeight);

    const QBrush &normalBorderColor() const;
    void setNormalBorderColor(const QBrush &newNormalBorderColor);

    const QBrush &hoverBorderColor() const;
    void setHoverBorderColor(const QBrush &newHoverBorderColor);

    const QBrush &clickBorderColor() const;
    void setClickBorderColor(const QBrush &newClickBorderColor);

    const QBrush &disableBorderColor() const;
    void setDisableBorderColor(const QBrush &newDisableBorderColor);

    const QBrush &focusBorderColor() const;
    void setFocusBorderColor(const QBrush &newFocusBorderColor);

    int focusBorderWidth() const;
    void setFocusBorderWidth(int newFocusBorderWidth);

    int normalBorderWidth() const;
    void setNormalBorderWidth(int newNormalBorderWidth);

    const QBrush &focusColor() const;
    void setFocusColor(const QBrush &newFocusColor);

    int padding() const;
    void setPadding(int newPadding);

    const QBrush &btnNormalColor() const;
    void setBtnNormalColor(const QBrush &newBtnNormalColor);

    const QBrush &btnHoverColor() const;
    void setBtnHoverColor(const QBrush &newBtnHoverColor);

    const QBrush &btnClickColor() const;
    void setBtnClickColor(const QBrush &newBtnClickColor);

    const QBrush &btnDisableColor() const;
    void setBtnDisableColor(const QBrush &newBtnDisableColor);

    const QBrush &btnFocusColor() const;
    void setBtnFocusColor(const QBrush &newBtnFocusColor);

    const QBrush &btnNormalBorderColor() const;
    void setBtnNormalBorderColor(const QBrush &newBtnNormalBorderColor);

    const QBrush &btnHoverBorderColor() const;
    void setBtnHoverBorderColor(const QBrush &newBtnHoverBorderColor);

    const QBrush &btnClickBorderColor() const;
    void setBtnClickBorderColor(const QBrush &newBtnClickBorderColor);

    const QBrush &btnDisableBorderColor() const;
    void setBtnDisableBorderColor(const QBrush &newBtnDisableBorderColor);

    const QBrush &btnFocusBorderColor() const;
    void setBtnFocusBorderColor(const QBrush &newBtnFocusBorderColor);

    int btnBorderWidth() const;
    void setBtnBorderWidth(int newBtnBorderWidth);

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

    const QBrush &btnNormalTransparentColor() const;
    void setBtnNormalTransparentColor(const QBrush &newBtnNormalTransparentColor);

    const QBrush &btnHoverTransparentColor() const;
    void setBtnHoverTransparentColor(const QBrush &newBtnHoverTransparentColor);

    const QBrush &btnClickTransparentColor() const;
    void setBtnClickTransparentColor(const QBrush &newBtnClickTransparentColor);

    const QBrush &btnDisableTransparentColor() const;
    void setBtnDisableTransparentColor(const QBrush &newBtnDisableTransparentColor);

    const QBrush &btnNormalTransparentBorderColor() const;
    void setBtnNormalTransparentBorderColor(const QBrush &newBtnNormalTransparentBorderColor);

    const QBrush &btnHoverTransparentBorderColor() const;
    void setBtnHoverTransparentBorderColor(const QBrush &newBtnHoverTransparentBorderColor);

    const QBrush &btnClickTransparentBorderColor() const;
    void setBtnClickTransparentBorderColor(const QBrush &newBtnClickTransparentBorderColor);

    const QBrush &btnDisableTransparentBorderColor() const;
    void setBtnDisableTransparentBorderColor(const QBrush &newBtnDisableTransparentBorderColor);

signals:
    void normalColorChanged();

    void linkColorChanged();

    void disableColorChanged();

    void hoverColorChanged();

    void clickColorChanged();

    void normalTextColorChanged();

    void disableTextColorChanged();

    void normalWidthChanged();

    void normalHeightChanged();

    void radiusChanged();

    void btnNormalWidthChanged();

    void btnNormalHeightChanged();

    void normalBorderColorChanged();

    void hoverColorBorderChanged();

    void clickColorBorderChanged();

    void disableBorderColorChanged();

    void focusBorderColorChanged();

    void focusBorderWidthChanged();

    void normalBorderWidthChanged();

    void focusColorChanged();

    void paddingChanged();

    void btnNormalColorChanged();

    void btnHoverColorChanged();

    void btnClickColorChanged();

    void btnDisableColorChanged();

    void btnFocusColorChanged();

    void btnNormalBorderColorChanged();

    void btnHoverBorderColorChanged();

    void btnClickBorderColorChanged();

    void btnDisableBorderColorChanged();

    void btnFocusBorderColorChanged();

    void btnBorderWidthChanged();

    void parametryChanged();

    void normalTransparentBCChanged();

    void clickedTransparentBCChanged();

    void hoveredTransparentBCChanged();

    void disableTransparentBCChanged();

    void normalTransparentBorderColorChanged();

    void clickedTransparentBorderColorChanged();

    void hoveredTransparentBorderColorChanged();

    void disableTransparentBorderColorChanged();

    void btnNormalTransparentColorChanged();

    void btnHoverTransparentColorChanged();

    void btnClickTransparentColorChanged();

    void btnDisableTransparentColorChanged();

    void btnNormalTransparentBorderColorChanged();

    void btnHoverTransparentBorderColorChanged();

    void btnClickTransparentBorderColorChanged();

    void btnDisableTransparentBorderColorChanged();

private:
    Q_INVOKABLE QBrush m_normalColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));
    Q_INVOKABLE QBrush m_disableColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    Q_INVOKABLE QBrush m_hoverColor;
    Q_INVOKABLE QBrush m_clickColor;
    Q_INVOKABLE QBrush m_normalTextColor;
    Q_INVOKABLE QBrush m_disableTextColor;
    int m_normalWidth;
    int m_normalHeight;
    int m_radius;
    int m_btnNormalWidth;
    int m_btnNormalHeight;
    Q_INVOKABLE QBrush m_normalBorderColor;
    Q_INVOKABLE QBrush m_hoverBorderColor;
    Q_INVOKABLE QBrush m_clickBorderColor;
    Q_INVOKABLE QBrush m_disableBorderColor;
    Q_INVOKABLE QBrush m_focusBorderColor;
    int m_focusBorderWidth;
    int m_normalBorderWidth;
    Q_INVOKABLE QBrush m_focusColor;
    int m_padding;
    Q_INVOKABLE QBrush m_btnNormalColor;
    Q_INVOKABLE QBrush m_btnHoverColor;
    Q_INVOKABLE QBrush m_btnClickColor;
    Q_INVOKABLE QBrush m_btnDisableColor;
    Q_INVOKABLE QBrush m_btnFocusColor;
    Q_INVOKABLE QBrush m_btnNormalBorderColor;
    Q_INVOKABLE QBrush m_btnHoverBorderColor;
    Q_INVOKABLE QBrush m_btnClickBorderColor;
    Q_INVOKABLE QBrush m_btnDisableBorderColor;
    Q_INVOKABLE QBrush m_btnFocusBorderColor;
    int m_btnBorderWidth;
    QBrush m_normalTransparentBC;
    QBrush m_clickedTransparentBC;
    QBrush m_hoveredTransparentBC;
    QBrush m_disableTransparentBC;
    QBrush m_normalTransparentBorderColor;
    QBrush m_clickedTransparentBorderColor;
    QBrush m_hoveredTransparentBorderColor;
    QBrush m_disableTransparentBorderColor;
    QBrush m_btnNormalTransparentColor;
    QBrush m_btnHoverTransparentColor;
    QBrush m_btnClickTransparentColor;
    QBrush m_btnDisableTransparentColor;
    QBrush m_btnNormalTransparentBorderColor;
    QBrush m_btnHoverTransparentBorderColor;
    QBrush m_btnClickTransparentBorderColor;
    QBrush m_btnDisableTransparentBorderColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUISpinBox, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUISPINBOX_H
