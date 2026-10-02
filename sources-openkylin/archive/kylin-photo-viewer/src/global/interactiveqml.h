/* Copyright (C) 2020, KylinSoft Co., Ltd.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef INTERACTIVEQML_H
#define INTERACTIVEQML_H

#include <QObject>
#include "global/variable.h"
#include <QDebug>
class InteractiveQml : public QObject
{
    Q_OBJECT
public:
    explicit InteractiveQml(QObject *parent = nullptr);
    Q_PROPERTY(int operateWay READ getOperateWay WRITE setOperateWay NOTIFY operateWayChanged)
    int getOperateWay()
    {
        return m_operateWay;
    }
    void setOperateWay(int way)
    {
        m_operateWay = way;
        Q_EMIT operateWayChanged(m_operateWay);
    }
    void setImageFormat(QString format)
    {
        m_imageFormat = format;
        Q_EMIT imageFormatChanged(m_imageFormat);
    }
    void setNavigatorState(bool state)
    {
        m_navigatorState = state;
        Q_EMIT navigatorStateChanged(m_navigatorState);
    }
    void setChangImageUseAnimation(bool isUse)
    {
        m_changImageUseAnimation = isUse;
        Q_EMIT changImageUseAnimationChanged(m_changImageUseAnimation);
    }
    static InteractiveQml *getInstance();

    void setPainterType(int type)
    {
        m_painterType = type;
        Q_EMIT painterTypeChanged(m_painterType);
    }
    void setPainterColor(QColor color)
    {
        m_painterColor = color;
        Q_EMIT painterColorChanged(color);
    }
    void setPainterThickness(int thickness)
    {
        m_painterThickness = thickness;
        Q_EMIT painterThicknessChanged(m_painterThickness);
    }
    void setPainterTextBold(bool isBold)
    {
        m_isBold = isBold;
        Q_EMIT textBoldChanged(m_isBold);
    }
    void setPainterTextDeleteLine(bool isDeleteLine)
    {
        m_isDeleteLine = isDeleteLine;
        Q_EMIT textDeleteLineChanged(m_isDeleteLine);
    }
    void setPainterTextUnderLine(bool isUnderLine)
    {
        m_isUnderLine = isUnderLine;
        Q_EMIT textUnderLineChanged(m_isUnderLine);
    }
    void setPainterTextItalics(bool isItalics)
    {
        m_isItalics = isItalics;
        Q_EMIT textItalicsChanged(m_isItalics);
    }
    void setPainterTextFontType(QString isFontType)
    {
        m_fontType = isFontType;
        Q_EMIT textFontTypeChanged(m_fontType);
    }
    void setPainterTextFontSize(int isFontSize)
    {
        m_fontSize = isFontSize;
        Q_EMIT textFontSizeChanged(m_fontSize);
    }


    int getPainterType()
    {
        return m_painterType;
    }
    QColor getPainterColor()
    {
        return m_painterColor;
    }
    int getPainterThickness()
    {
        return m_painterThickness;
    }
    bool getPainterTextBold()
    {
        return m_isBold;
    }
    bool getPainterTextDeleteLine()
    {
        return m_isDeleteLine;
    }
    bool getPainterTextUnderLine()
    {
        return m_isUnderLine;
    }
    bool getPainterTextItalics()
    {
        return m_isItalics;
    }
    QString getPainterTextFontType()
    {
        return m_fontType;
    }
    int getPainterTextFontSize()
    {
        return m_fontSize;
    }

    void setChangeImageBtnVisible(bool isvisible)
    {
        m_ChangeImageBtnVisible = isvisible;
        Q_EMIT changeImageBtnVisible(isvisible);
    }

    void setBackBtnPos(int backBtnPos)
    {
        m_backBtnPos;
        Q_EMIT backBtnPosChanged(backBtnPos);
    }

    void setChangeImageBtnBg(QString theme)
    {
        m_theme = theme;
        Q_EMIT ChangeImageBtnBg(theme);
    }

private:
    int m_operateWay = 0;
    QString m_imageFormat = "";
    bool m_navigatorState = false;
    bool m_changImageUseAnimation = false;
    int m_painterType = -1;                    //记录画笔类型
    int m_painterThickness = 12;               //记录画笔粗细
    QColor m_painterColor = QColor(255, 0, 0); //记录画笔颜色
    bool m_isBold = false;                     //加粗
    bool m_isDeleteLine = false;               //删除
    bool m_isUnderLine = false;                //下划线
    bool m_isItalics = false;                  //斜体
    QString m_fontType = "";                   //字体
    int m_fontSize = 12;                       //字号
    static InteractiveQml *m_interactionQml;   //单例指针

    int m_backBtnPos;
    bool m_ChangeImageBtnVisible = false;
    QString m_theme = "";

Q_SIGNALS:

    void operateWayChanged(QVariant);
    void imageFormatChanged(QVariant);
    void navigatorStateChanged(QVariant);
    void changImageUseAnimationChanged(QVariant);
    void backBtnPosChanged(QVariant);
    void ChangeImageBtnBg(QVariant);
    void changeImageBtnVisible(QVariant);

    // sign
    void painterTypeChanged(QVariant);
    void painterThicknessChanged(QVariant);
    void painterColorChanged(QVariant);
    void textBoldChanged(QVariant);
    void textDeleteLineChanged(QVariant);
    void textUnderLineChanged(QVariant);
    void textItalicsChanged(QVariant);
    void textFontTypeChanged(QVariant);
    void textFontSizeChanged(QVariant);
};

#endif // INTERACTIVE_H
