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

#ifndef UKUIRADIOBUTTON_H
#define UKUIRADIOBUTTON_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIRadioButton : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(double leftRightMargin READ leftRightMargin WRITE setLeftRightMargin NOTIFY leftRightMarginChanged)
    Q_PROPERTY(double upDownMargin READ upDownMargin WRITE setUpDownMargin NOTIFY upDownMarginChanged)
    Q_PROPERTY(double space READ space WRITE setSpace NOTIFY spaceChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)


    Q_PROPERTY(QBrush disableTextColor READ disableTextColor WRITE setDisableTextColor NOTIFY disableTextColorChanged)
    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY normalTextColorChanged)

    Q_PROPERTY(QBrush normalIndicatorColor READ normalIndicatorColor WRITE setNormalIndicatorColor NOTIFY normalIndicatorColorChanged)
    Q_PROPERTY(QBrush hoverIndicatorColor READ hoverIndicatorColor WRITE setHoverIndicatorColor NOTIFY hoverIndicatorColorChanged)
    Q_PROPERTY(QBrush clickIndicatorColor READ clickIndicatorColor WRITE setClickIndicatorColor NOTIFY clickIndicatorColorChanged)
    Q_PROPERTY(QBrush disableIndicatorColor READ disableIndicatorColor WRITE setDisableIndicatorColor NOTIFY disableIndicatorColorChanged)
    Q_PROPERTY(QBrush normalIndicatorBorderColor READ normalIndicatorBorderColor WRITE setNormalIndicatorBorderColor NOTIFY normalIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush hoverIndicatorBorderColor READ hoverIndicatorBorderColor WRITE setHoverIndicatorBorderColor NOTIFY hoverIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush clickIndicatorBorderColor READ clickIndicatorBorderColor WRITE setClickIndicatorBorderColor NOTIFY clickIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush disableIndicatorBorderColor READ disableIndicatorBorderColor WRITE setDisableIndicatorBorderColor NOTIFY disableIndicatorBorderColorChanged)

    Q_PROPERTY(QBrush checked_normalIndicatorColor        READ checked_normalIndicatorColor        WRITE setChecked_NormalIndicatorColor        NOTIFY checked_normalIndicatorColorChanged)
    Q_PROPERTY(QBrush checked_hoverIndicatorColor         READ checked_hoverIndicatorColor         WRITE setChecked_HoverIndicatorColor         NOTIFY checked_hoverIndicatorColorChanged)
    Q_PROPERTY(QBrush checked_clickIndicatorColor         READ checked_clickIndicatorColor         WRITE setChecked_ClickIndicatorColor         NOTIFY checked_clickIndicatorColorChanged)
    Q_PROPERTY(QBrush checked_disableIndicatorColor       READ checked_disableIndicatorColor       WRITE setChecked_DisableIndicatorColor       NOTIFY checked_disableIndicatorColorChanged)
    Q_PROPERTY(QBrush checked_normalIndicatorBorderColor  READ checked_normalIndicatorBorderColor  WRITE setChecked_NormalIndicatorBorderColor  NOTIFY checked_normalIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush checked_hoverIndicatorBorderColor   READ checked_hoverIndicatorBorderColor   WRITE setChecked_HoverIndicatorBorderColor   NOTIFY checked_hoverIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush checked_clickIndicatorBorderColor   READ checked_clickIndicatorBorderColor   WRITE setChecked_ClickIndicatorBorderColor   NOTIFY checked_clickIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush checked_disableIndicatorBorderColor READ checked_disableIndicatorBorderColor WRITE setChecked_DisableIndicatorBorderColor NOTIFY checked_disableIndicatorBorderColorChanged)

    Q_PROPERTY(QBrush checked_normalChildrenColor        READ checked_normalChildrenColor        WRITE setChecked_NormalChildrenColor        NOTIFY checked_normalChildrenColorChanged)
    Q_PROPERTY(QBrush checked_hoverChildrenColor         READ checked_hoverChildrenColor         WRITE setChecked_HoverChildrenColor         NOTIFY checked_hoverChildrenColorChanged)
    Q_PROPERTY(QBrush checked_clickChildrenColor         READ checked_clickChildrenColor         WRITE setChecked_ClickChildrenColor         NOTIFY checked_clickChildrenColorChanged)
    Q_PROPERTY(QBrush checked_disableChildrenColor       READ checked_disableChildrenColor       WRITE setChecked_DisableChildrenColor       NOTIFY checked_disableChildrenColorChanged)
    Q_PROPERTY(QBrush checked_normalChildrenBorderColor  READ checked_normalChildrenBorderColor  WRITE setChecked_NormalChildrenBorderColor  NOTIFY checked_normalChildrenBorderColorChanged)
    Q_PROPERTY(QBrush checked_hoverChildrenBorderColor   READ checked_hoverChildrenBorderColor   WRITE setChecked_HoverChildrenBorderColor   NOTIFY checked_hoverChildrenBorderColorChanged)
    Q_PROPERTY(QBrush checked_clickChildrenBorderColor   READ checked_clickChildrenBorderColor   WRITE setChecked_ClickChildrenBorderColor   NOTIFY checked_clickChildrenBorderColorChanged)
    Q_PROPERTY(QBrush checked_disableChildrenBorderColor READ checked_disableChildrenBorderColor WRITE setChecked_DisableChildrenBorderColor NOTIFY checked_disableChildrenBorderColorChanged)

    Q_PROPERTY(QBrush normalTransparentIndicatorColor        READ normalTransparentIndicatorColor        WRITE setNormalTransparentIndicatorColor        NOTIFY normalTransparentIndicatorColorChanged)
    Q_PROPERTY(QBrush hoverTransparentIndicatorColor         READ hoverTransparentIndicatorColor         WRITE setHoverTransparentIndicatorColor         NOTIFY hoverTransparentIndicatorColorChanged)
    Q_PROPERTY(QBrush clickTransparentIndicatorColor         READ clickTransparentIndicatorColor         WRITE setClickTransparentIndicatorColor         NOTIFY clickTransparentIndicatorColorChanged)
    Q_PROPERTY(QBrush disableTransparentIndicatorColor       READ disableTransparentIndicatorColor       WRITE setDisableTransparentIndicatorColor       NOTIFY disableTransparentIndicatorColorChanged)
    Q_PROPERTY(QBrush normalTransparentIndicatorBorderColor  READ normalTransparentIndicatorBorderColor  WRITE setNormalTransparentIndicatorBorderColor  NOTIFY normalTransparentIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush hoverTransparentIndicatorBorderColor   READ hoverTransparentIndicatorBorderColor   WRITE setHoverTransparentIndicatorBorderColor   NOTIFY hoverTransparentIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush clickTransparentIndicatorBorderColor   READ clickTransparentIndicatorBorderColor   WRITE setClickTransparentIndicatorBorderColor   NOTIFY clickTransparentIndicatorBorderColorChanged)
    Q_PROPERTY(QBrush disableTransparentIndicatorBorderColor READ disableTransparentIndicatorBorderColor WRITE setDisableTransparentIndicatorBorderColor NOTIFY disableTransparentIndicatorBorderColorChanged)


    Q_PROPERTY(int indicatorWidth READ indicatorWidth WRITE setIndicatorWidth NOTIFY indicatorWidthChanged)
    Q_PROPERTY(int childrenWidth READ childrenWidth WRITE setChildrenWidth NOTIFY childrenWidthChanged)


public:
    explicit UKUIRadioButton(QQuickItem *parent = nullptr);
    ~UKUIRadioButton();

    static UKUIRadioButton* qmlAttachedProperties(QObject* parent);

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);
    double leftRightMargin() const;
    void setLeftRightMargin(double newLeftRightMargin);

    double upDownMargin() const;
    void setUpDownMargin(double newUpDownMargin);

    double space() const;
    void setSpace(double newSpace);

    const QBrush &disableTextColor() const;
    void setDisableTextColor(const QBrush &newDisableTextColor);

    const QBrush &normalTextColor() const;
    void setNormalTextColor(const QBrush &newNormalTextColor);

    const QBrush &normalIndicatorColor() const;
    void setNormalIndicatorColor(const QBrush &newNormalIndicatorColor);

    const QBrush &hoverIndicatorColor() const;
    void setHoverIndicatorColor(const QBrush &newHoverIndicatorColor);

    const QBrush &clickIndicatorColor() const;
    void setClickIndicatorColor(const QBrush &newClickIndicatorColor);

    const QBrush &disableIndicatorColor() const;
    void setDisableIndicatorColor(const QBrush &newDisableIndicatorColor);

    const QBrush &normalIndicatorBorderColor() const;
    void setNormalIndicatorBorderColor(const QBrush &newNormalIndicatorBorderColor);

    const QBrush &hoverIndicatorBorderColor() const;
    void setHoverIndicatorBorderColor(const QBrush &newHoverIndicatorBorderColor);

    const QBrush &clickIndicatorBorderColor() const;
    void setClickIndicatorBorderColor(const QBrush &newClickIndicatorBorderColor);

    const QBrush &disableIndicatorBorderColor() const;
    void setDisableIndicatorBorderColor(const QBrush &newDisableIndicatorBorderColor);

    const QBrush &checked_normalIndicatorColor() const;
    void setChecked_NormalIndicatorColor(const QBrush &newChecked_normalIndicatorColor);

    const QBrush &checked_hoverIndicatorColor() const;
    void setChecked_HoverIndicatorColor(const QBrush &newChecked_hoverIndicatorColor);

    const QBrush &checked_clickIndicatorColor() const;
    void setChecked_ClickIndicatorColor(const QBrush &newChecked_clickIndicatorColor);

    const QBrush &checked_disableIndicatorColor() const;
    void setChecked_DisableIndicatorColor(const QBrush &newChecked_disableIndicatorColor);

    const QBrush &checked_normalIndicatorBorderColor() const;
    void setChecked_NormalIndicatorBorderColor(const QBrush &newChecked_normalIndicatorBorderColor);

    const QBrush &checked_hoverIndicatorBorderColor() const;
    void setChecked_HoverIndicatorBorderColor(const QBrush &newChecked_hoverIndicatorBorderColor);

    const QBrush &checked_clickIndicatorBorderColor() const;
    void setChecked_ClickIndicatorBorderColor(const QBrush &newChecked_clickIndicatorBorderColor);

    const QBrush &checked_disableIndicatorBorderColor() const;
    void setChecked_DisableIndicatorBorderColor(const QBrush &newChecked_disableIndicatorBorderColor);

    const QBrush &checked_normalChildrenColor() const;
    void setChecked_NormalChildrenColor(const QBrush &newChecked_normalChildrenColor);

    const QBrush &checked_hoverChildrenColor() const;
    void setChecked_HoverChildrenColor(const QBrush &newChecked_hoverChildrenColor);

    const QBrush &checked_clickChildrenColor() const;
    void setChecked_ClickChildrenColor(const QBrush &newChecked_clickChildrenColor);

    const QBrush &checked_disableChildrenColor() const;
    void setChecked_DisableChildrenColor(const QBrush &newChecked_disableChildrenColor);

    const QBrush &checked_normalChildrenBorderColor() const;
    void setChecked_NormalChildrenBorderColor(const QBrush &newChecked_normalChildrenBorderColor);

    const QBrush &checked_hoverChildrenBorderColor() const;
    void setChecked_HoverChildrenBorderColor(const QBrush &newChecked_hoverChildrenBorderColor);

    const QBrush &checked_clickChildrenBorderColor() const;
    void setChecked_ClickChildrenBorderColor(const QBrush &newChecked_clickChildrenBorderColor);

    const QBrush &checked_disableChildrenBorderColor() const;
    void setChecked_DisableChildrenBorderColor(const QBrush &newChecked_disableChildrenBorderColor);

    int borderWidth() const;
    void setBorderWidth(int newBorderWidth);

    int indicatorWidth() const;
    void setIndicatorWidth(int newIndicatorWidth);

    int childrenWidth() const;
    void setChildrenWidth(int newChildrenWidth);

    const QBrush &normalTransparentIndicatorColor() const;
    void setNormalTransparentIndicatorColor(const QBrush &newNormalTransparentIndicatorColor);

    const QBrush &hoverTransparentIndicatorColor() const;
    void setHoverTransparentIndicatorColor(const QBrush &newHoverTransparentIndicatorColor);

    const QBrush &clickTransparentIndicatorColor() const;
    void setClickTransparentIndicatorColor(const QBrush &newClickTransparentIndicatorColor);

    const QBrush &disableTransparentIndicatorColor() const;
    void setDisableTransparentIndicatorColor(const QBrush &newDisableTransparentIndicatorColor);

    const QBrush &normalTransparentIndicatorBorderColor() const;
    void setNormalTransparentIndicatorBorderColor(const QBrush &newNormalTransparentIndicatorBorderColor);

    const QBrush &hoverTransparentIndicatorBorderColor() const;
    void setHoverTransparentIndicatorBorderColor(const QBrush &newHoverTransparentIndicatorBorderColor);

    const QBrush &clickTransparentIndicatorBorderColor() const;
    void setClickTransparentIndicatorBorderColor(const QBrush &newClickTransparentIndicatorBorderColor);

    const QBrush &disableTransparentIndicatorBorderColor() const;
    void setDisableTransparentIndicatorBorderColor(const QBrush &newDisableTransparentIndicatorBorderColor);

signals:

    void leftRightMarginChanged();

    void upDownMarginChanged();

    void spaceChanged();

    void disableTextColorChanged();

    void normalTextColorChanged();

    void normalIndicatorColorChanged();

    void hoverIndicatorColorChanged();

    void clickIndicatorColorChanged();

    void disableIndicatorColorChanged();

    void normalIndicatorBorderColorChanged();

    void hoverIndicatorBorderColorChanged();

    void clickIndicatorBorderColorChanged();

    void disableIndicatorBorderColorChanged();

    void checked_normalIndicatorColorChanged();

    void checked_hoverIndicatorColorChanged();

    void checked_clickIndicatorColorChanged();

    void checked_disableIndicatorColorChanged();

    void checked_normalIndicatorBorderColorChanged();

    void checked_hoverIndicatorBorderColorChanged();

    void checked_clickIndicatorBorderColorChanged();

    void checked_disableIndicatorBorderColorChanged();

    void checked_normalChildrenColorChanged();

    void checked_hoverChildrenColorChanged();

    void checked_clickChildrenColorChanged();

    void checked_disableChildrenColorChanged();

    void checked_normalChildrenBorderColorChanged();

    void checked_hoverChildrenBorderColorChanged();

    void checked_clickChildrenBorderColorChanged();

    void checked_disableChildrenBorderColorChanged();

    void borderWidthChanged();

    void indicatorWidthChanged();

    void childrenWidthChanged();

    void parametryChanged();

    void normalTransparentIndicatorColorChanged();

    void hoverTransparentIndicatorColorChanged();

    void clickTransparentIndicatorColorChanged();

    void disableTransparentIndicatorColorChanged();

    void normalTransparentIndicatorBorderColorChanged();

    void hoverTransparentIndicatorBorderColorChanged();

    void clickTransparentIndicatorBorderColorChanged();

    void disableTransparentIndicatorBorderColorChanged();

private:
    double m_leftRightMargin;
    double m_upDownMargin;
    double m_space;
    Q_INVOKABLE QBrush m_disableTextColor;
    Q_INVOKABLE QBrush m_normalTextColor;
    Q_INVOKABLE QBrush m_normalIndicatorColor;
    Q_INVOKABLE QBrush m_hoverIndicatorColor;
    Q_INVOKABLE QBrush m_clickIndicatorColor;
    Q_INVOKABLE QBrush m_disableIndicatorColor;
    Q_INVOKABLE QBrush m_normalIndicatorBorderColor;
    Q_INVOKABLE QBrush m_hoverIndicatorBorderColor;
    Q_INVOKABLE QBrush m_clickIndicatorBorderColor;
    Q_INVOKABLE QBrush m_disableIndicatorBorderColor;
    Q_INVOKABLE QBrush m_checked_normalIndicatorColor;
    Q_INVOKABLE QBrush m_checked_hoverIndicatorColor;
    Q_INVOKABLE QBrush m_checked_clickIndicatorColor;
    Q_INVOKABLE QBrush m_checked_disableIndicatorColor;
    Q_INVOKABLE QBrush m_checked_normalIndicatorBorderColor;
    Q_INVOKABLE QBrush m_checked_hoverIndicatorBorderColor;
    Q_INVOKABLE QBrush m_checked_clickIndicatorBorderColor;
    Q_INVOKABLE QBrush m_checked_disableIndicatorBorderColor;
    Q_INVOKABLE QBrush m_checked_normalChildrenColor;
    Q_INVOKABLE QBrush m_checked_hoverChildrenColor;
    Q_INVOKABLE QBrush m_checked_clickChildrenColor;
    Q_INVOKABLE QBrush m_checked_disableChildrenColor;
    Q_INVOKABLE QBrush m_checked_normalChildrenBorderColor;
    Q_INVOKABLE QBrush m_checked_hoverChildrenBorderColor;
    Q_INVOKABLE QBrush m_checked_clickChildrenBorderColor;
    Q_INVOKABLE QBrush m_checked_disableChildrenBorderColor;
    int m_borderWidth;
    int m_indicatorWidth;
    int m_childrenWidth;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_normalTransparentIndicatorColor;
    QBrush m_hoverTransparentIndicatorColor;
    QBrush m_clickTransparentIndicatorColor;
    QBrush m_disableTransparentIndicatorColor;
    QBrush m_normalTransparentIndicatorBorderColor;
    QBrush m_hoverTransparentIndicatorBorderColor;
    QBrush m_clickTransparentIndicatorBorderColor;
    QBrush m_disableTransparentIndicatorBorderColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIRadioButton, QML_HAS_ATTACHED_PROPERTIES)

//Q_DECLARE_METATYPE(UKUIQQC2Style::UKUIButton);
//Q_DECLARE_METATYPE(std::shared_ptr<UKUIQQC2Style::UKUIButton>)

#endif // UKUIRADIOBUTTON_H
