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

#ifndef UKUISLIDER_H
#define UKUISLIDER_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUISlider : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int normalWidth READ normalWidth WRITE setNormalWidth NOTIFY normalWidthChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setNormalHeight NOTIFY normalHeightChanged)
    Q_PROPERTY(int grooveHeight READ grooveHeight WRITE setGrooveHeight NOTIFY grooveHeightChanged)
    Q_PROPERTY(int handleHeight READ handleHeight WRITE setHandleHeight NOTIFY handleHeightChanged)
    Q_PROPERTY(int handleHoverHeight READ handleHoverHeight WRITE setHandleHoverHeight NOTIFY handleHoverHeightChanged)

    Q_PROPERTY(int focusBorderWidth READ focusBorderWidth WRITE setFocusBorderWidth NOTIFY focusBorderWidthChanged)
    Q_PROPERTY(QBrush focusBorderColor READ focusBorderColor WRITE setFocusBorderColor NOTIFY focusBorderColorChanged)

    Q_PROPERTY(QBrush normalGrooveColor READ normalGrooveColor WRITE setNormalGrooveColor NOTIFY normalGrooveColorChanged)
    Q_PROPERTY(QBrush disableGrooveColor READ disableGrooveColor WRITE setDisableGrooveColor NOTIFY disableGrooveColorChanged)
    Q_PROPERTY(QBrush normalGrooveBorderColor READ normalGrooveBorderColor WRITE setNormalGrooveBorderColor NOTIFY normalGrooveBorderColorChanged)
    Q_PROPERTY(QBrush disableGrooveBorderColor READ disableGrooveBorderColor WRITE setDisableGrooveBorderColor NOTIFY disableGrooveBorderColorChanged)
    Q_PROPERTY(int grooveBorderWidth READ grooveBorderWidth WRITE setGrooveBorderWidth NOTIFY grooveBorderWidthChanged)

    Q_PROPERTY(QBrush normalUnGrooveColor READ normalUnGrooveColor WRITE setNormalUnGrooveColor NOTIFY normalUnGrooveColorChanged)
    Q_PROPERTY(QBrush disableUnGrooveColor READ disableUnGrooveColor WRITE setDisableUnGrooveColor NOTIFY disableUnGrooveColorChanged)
    Q_PROPERTY(QBrush normalUnGrooveBorderColor READ normalUnGrooveBorderColor WRITE setNormalUnGrooveBorderColor NOTIFY normalUnGrooveBorderColorChanged)
    Q_PROPERTY(QBrush disableUnGrooveBorderColor READ disableUnGrooveBorderColor WRITE setDisableUnGrooveBorderColor NOTIFY disableUnGrooveBorderColorChanged)


    Q_PROPERTY(QBrush normalHandleColor READ normalHandleColor WRITE setNormalHandleColor NOTIFY normalHandleColorChanged)
    Q_PROPERTY(QBrush hoverHandleColor READ hoverHandleColor WRITE setHoverHandleColor NOTIFY hoverHandleColorChanged)
    Q_PROPERTY(QBrush clickHandleColor READ clickHandleColor WRITE setClickHandleColor NOTIFY clickHandleColorChanged)
    Q_PROPERTY(QBrush disableHandleColor READ disableHandleColor WRITE setDisableHandleColor NOTIFY DisableHandleColorChanged)
    Q_PROPERTY(QBrush normalHandleBorderColor READ normalHandleBorderColor WRITE setNormalHandleBorderColor NOTIFY normalHandleBorderColorChanged)
    Q_PROPERTY(QBrush hoverHandleBorderColor READ hoverHandleBorderColor WRITE setHoverHandleBorderColor NOTIFY hoverHandleBorderColorChanged)
    Q_PROPERTY(QBrush clickHandleBorderColor READ clickHandleBorderColor WRITE setClickHandleBorderColor NOTIFY clickHandleBorderColorChanged)
    Q_PROPERTY(QBrush disableHandleBorderColor READ disableHandleBorderColor WRITE setDisableHandleBorderColor NOTIFY disableHandleBorderColorChanged)
    Q_PROPERTY(int handleBorderWidth READ handleBorderWidth WRITE setHandleBorderWidth NOTIFY handleBorderWidthChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)

    Q_PROPERTY(int touchNormalWidth READ touchNormalWidth WRITE setTouchNormalWidth NOTIFY touchNormalWidthChanged)
    Q_PROPERTY(int touchNormalHeight READ touchNormalHeight WRITE setTouchNormalHeight NOTIFY touchNormalHeightChanged)
    Q_PROPERTY(int touchGrooveHeight READ touchGrooveHeight WRITE setTouchGrooveHeight NOTIFY touchGrooveHeightChanged)
    Q_PROPERTY(int touchHandleHeight READ touchHandleHeight WRITE settouchHandleHeight NOTIFY touchHandleHeightChanged)
    Q_PROPERTY(int touchHandleWidth READ touchHandleWidth WRITE setTouchHandleWidth NOTIFY touchHandleWidthChanged)
    Q_PROPERTY(QBrush touchHandleColor READ touchHandleColor WRITE setTouchHandleColor NOTIFY touchHandleColorChanged)

    Q_PROPERTY(QBrush normalTransparentUnGrooveColor READ normalTransparentUnGrooveColor WRITE setNormalTransparentUnGrooveColor NOTIFY normalTransparentUnGrooveColorChanged)
    Q_PROPERTY(QBrush disableTransparentUnGrooveColor READ disableTransparentUnGrooveColor WRITE setDisableTransparentUnGrooveColor NOTIFY disableTransparentUnGrooveColorChanged)
    Q_PROPERTY(QBrush normalTransparentUnGrooveBorderColor READ normalTransparentUnGrooveBorderColor WRITE setNormalTransparentUnGrooveBorderColor NOTIFY normalTransparentUnGrooveBorderColorChanged)
    Q_PROPERTY(QBrush disableTransparentUnGrooveBorderColor READ disableTransparentUnGrooveBorderColor WRITE setDisableTransparentUnGrooveBorderColor NOTIFY disableTransparentUnGrooveBorderColorChanged)

    Q_PROPERTY(QBrush touchHandleBaseColor READ touchHandleBaseColor WRITE setTouchHandleBaseColor NOTIFY touchHandleBaseColorChanged)


public:
    explicit UKUISlider(QQuickItem *parent = nullptr);
    ~UKUISlider();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUISlider* qmlAttachedProperties(QObject* parent);

    int normalWidth() const;
    void setNormalWidth(int newNormalWidth);

    int normalHeight() const;
    void setNormalHeight(int newNormalHeight);

    int grooveHeight() const;
    void setGrooveHeight(int newGrooveHeight);

    int handleHeight() const;
    void setHandleHeight(int newHandleHeight);

    int focusBorderWidth() const;
    void setFocusBorderWidth(int newFocusBorderWidth);

    const QBrush &focusBorderColor() const;
    void setFocusBorderColor(const QBrush &newFocusBorderColor);

    const QBrush &normalGrooveColor() const;
    void setNormalGrooveColor(const QBrush &newNormalGrooveColor);

    const QBrush &disableGrooveColor() const;
    void setDisableGrooveColor(const QBrush &newDisableGrooveColor);

    const QBrush &normalHandleColor() const;
    void setNormalHandleColor(const QBrush &newNormalHandleColor);

    const QBrush &disableHandleColor() const;
    void setDisableHandleColor(const QBrush &newDisableHandleColor);

    const QBrush &normalGrooveBorderColor() const;
    void setNormalGrooveBorderColor(const QBrush &newNormalGrooveBorderColor);

    const QBrush &disableGrooveBorderColor() const;
    void setDisableGrooveBorderColor(const QBrush &newDisableGrooveBorderColor);

    int grooveBorderWidth() const;
    void setGrooveBorderWidth(int newGrooveBorderWidth);

    const QBrush &normalHandleBorderColor() const;
    void setNormalHandleBorderColor(const QBrush &newNormalHandleBorderColor);

    const QBrush &disableHandleBorderColor() const;
    void setDisableHandleBorderColor(const QBrush &newDisableHandleBorderColor);

    int handleBorderWidth() const;
    void setHandleBorderWidth(int newHandleBorderWidth);

    const QBrush &normalUnGrooveColor() const;
    void setNormalUnGrooveColor(const QBrush &newNormalUnGrooveColor);

    const QBrush &disableUnGrooveColor() const;
    void setDisableUnGrooveColor(const QBrush &newDisableUnGrooveColor);

    const QBrush &normalUnGrooveBorderColor() const;
    void setNormalUnGrooveBorderColor(const QBrush &newNormalUnGrooveBorderColor);

    const QBrush &disableUnGrooveBorderColor() const;
    void setDisableUnGrooveBorderColor(const QBrush &newDisableUnGrooveBorderColor);

    const QBrush &hoverHandleColor() const;
    void setHoverHandleColor(const QBrush &newHoverHandleColor);

    const QBrush &clickHandleColor() const;
    void setClickHandleColor(const QBrush &newClickHandleColor);

    const QBrush &hoverHandleBorderColor() const;
    void setHoverHandleBorderColor(const QBrush &newHoverHandleBorderColor);

    const QBrush &clickHandleBorderColor() const;
    void setClickHandleBorderColor(const QBrush &newClickHandleBorderColor);

    int padding() const;
    void setPadding(int newPadding);

    int touchNormalWidth() const;
    void setTouchNormalWidth(int newTouchNormalWidth);

    int touchNormalHeight() const;
    void setTouchNormalHeight(int newTouchNormalHeight);

    int touchGrooveHeight() const;
    void setTouchGrooveHeight(int newTouchGrooveHeight);

    int touchHandleHeight() const;
    void settouchHandleHeight(int newTouchHandleHeight);

    int touchHandleWidth() const;
    void setTouchHandleWidth(int newTouchHandleWidth);

    const QBrush &touchHandleColor() const;
    void setTouchHandleColor(const QBrush &newTouchHandleColor);

    const QBrush &normalTransparentUnGrooveColor() const;
    void setNormalTransparentUnGrooveColor(const QBrush &newNormalTransparentUnGrooveColor);

    const QBrush &disableTransparentUnGrooveColor() const;
    void setDisableTransparentUnGrooveColor(const QBrush &newDisableTransparentUnGrooveColor);

    const QBrush &normalTransparentUnGrooveBorderColor() const;
    void setNormalTransparentUnGrooveBorderColor(const QBrush &newNormalTransparentUnGrooveBorderColor);

    const QBrush &disableTransparentUnGrooveBorderColor() const;
    void setDisableTransparentUnGrooveBorderColor(const QBrush &newDisableTransparentUnGrooveBorderColor);

    int handleHoverHeight() const;
    void setHandleHoverHeight(int newHandleHoverHeight);

    const QBrush &touchHandleBaseColor() const;
    void setTouchHandleBaseColor(const QBrush &newTouchHandleBaseColor);

signals:
    void normalWidthChanged();

    void normalHeightChanged();

    void grooveHeightChanged();

    void handleHeightChanged();

    void focusBorderWidthChanged();

    void focusBorderColorChanged();

    void normalGrooveColorChanged();

    void disableGrooveColorChanged();

    void normalHandleColorChanged();

    void DisableHandleColorChanged();

    void normalGrooveBorderColorChanged();

    void disableGrooveBorderColorChanged();

    void grooveBorderWidthChanged();

    void normalHandleBorderColorChanged();

    void disableHandleBorderColorChanged();

    void handleBorderWidthChanged();

    void normalUnGrooveColorChanged();

    void disableUnGrooveColorChanged();

    void normalUnGrooveBorderColorChanged();

    void disableUnGrooveBorderColorChanged();

    void hoverHandleColorChanged();

    void clickHandleColorChanged();

    void hoverHandleBorderColorChanged();

    void clickHandleBorderColorChanged();

    void parametryChanged();

    void paddingChanged();

    void touchNormalWidthChanged();

    void touchNormalHeightChanged();

    void touchGrooveHeightChanged();

    void touchHandleHeightChanged();

    void touchHandleWidthChanged();

    void touchHandleColorChanged();

    void normalTransparentUnGrooveColorChanged();

    void disableTransparentUnGrooveColorChanged();

    void normalTransparentUnGrooveBorderColorChanged();

    void disableTransparentUnGrooveBorderColorChanged();

    void handleHoverHeightChanged();

    void touchHandleBaseColorChanged();

private:
    int m_normalWidth;
    int m_normalHeight;
    int m_grooveHeight;
    int m_handleHeight;
    int m_focusBorderWidth;
    Q_INVOKABLE QBrush m_focusBorderColor;
    Q_INVOKABLE QBrush m_normalGrooveColor;
    Q_INVOKABLE QBrush m_disableGrooveColor;
    Q_INVOKABLE QBrush m_normalHandleColor;
    Q_INVOKABLE QBrush m_disableHandleColor;
    Q_INVOKABLE QBrush m_normalGrooveBorderColor;
    Q_INVOKABLE QBrush m_disableGrooveBorderColor;
    int m_grooveBorderWidth;
    Q_INVOKABLE QBrush m_normalHandleBorderColor;
    Q_INVOKABLE QBrush m_disableHandleBorderColor;
    int m_handleBorderWidth;
    Q_INVOKABLE QBrush m_normalUnGrooveColor;
    Q_INVOKABLE QBrush m_disableUnGrooveColor;
    Q_INVOKABLE QBrush m_normalUnGrooveBorderColor;
    Q_INVOKABLE QBrush m_disableUnGrooveBorderColor;
    Q_INVOKABLE QBrush m_hoverHandleColor;
    Q_INVOKABLE QBrush m_clickHandleColor;
    Q_INVOKABLE QBrush m_hoverHandleBorderColor;
    Q_INVOKABLE QBrush m_clickHandleBorderColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    int m_padding;
    int m_touchNormalWidth;
    int m_touchNormalHeight;
    int m_touchGrooveHeight;
    int m_touchHandleHeight;
    int m_touchHandleWidth;
    QBrush m_touchHandleColor;
    QBrush m_normalTransparentUnGrooveColor;
    QBrush m_disableTransparentUnGrooveColor;
    QBrush m_normalTransparentUnGrooveBorderColor;
    QBrush m_disableTransparentUnGrooveBorderColor;
    int m_handleHoverHeight;
    QBrush m_touchHandleBaseColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUISlider, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUISLIDER_H
