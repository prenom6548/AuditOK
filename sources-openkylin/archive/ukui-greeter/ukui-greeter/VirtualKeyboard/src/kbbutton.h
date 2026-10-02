/*
 * Copyright (C) 2018 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef KBBUTTON_H
#define KBBUTTON_H

#include <QPushButton>

class KBButton : public QPushButton
{
    Q_OBJECT
public:
    enum BORDER_RADIUS{
        BORDER_RADIUS_NONE,
        BORDER_RADIUS_LT = 1,
        BORDER_RADIUS_TR = 2,
        BORDER_RADIUS_RB = 4,
        BORDER_RADIUS_LB = 8,
        BORDER_RADIUS_ALL = 0xF,
    };
    KBButton(QWidget *parent = nullptr);
    virtual ~KBButton();

    void setCharId(QChar charId);
    void updateStyleSheet(QString clrNormal, QString clrHover, QString clrChecked, QString clrBoard, QString clrBoardPress, QString clrFontPress, int radius = BORDER_RADIUS_ALL, bool is_lock = false);
    void setShiftState(bool isShift);
    void setCapsStatus(bool isCaps);
    void setCtrlState(bool isCtrl);
    void setAltState(bool isAlt);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void clicked(QChar charId);

private:
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor);

private:
    int m_nKeyId = -1;
    bool m_isShift = false;
    bool m_isCtrl = false;
    bool m_isAlt = false;
    bool m_isCaps = false;
    QChar m_charId = QChar::Null;
    QString m_clrBoard;
    QString m_clrNormal;
    QString m_clrBoardPress;
    QString m_clrHover;
    QString m_clrChecked;
    QFont sysFont;
};

#endif // KBBUTTON_H
