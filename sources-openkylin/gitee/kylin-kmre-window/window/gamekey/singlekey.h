/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SINGLE_KEY_H
#define SINGLE_KEY_H

#include "basekey.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class KmreWindow;

class SingleKey : public BaseKey
{
    Q_OBJECT
public:
    explicit SingleKey(int idx, KmreWindow* window);
    ~SingleKey();
    
    void enableEdit(bool enable) Q_DECL_OVERRIDE;
    void updateSize(bool update_panel = true) Q_DECL_OVERRIDE;
    void updatePos() Q_DECL_OVERRIDE;

    void setKeyString(QString character);
    void getEventXY(double &x, double &y);
    void setEventXY(double x, double y);
    QString getKeyString() { return m_keyString; }
    void updateStoredKeyString(){m_storedKeyString = m_keyString;}
    static QString genKeyString(QKeyEvent *k);

    int getIndex(){return m_index;}
    int mouseOrderNumber = 0; //对应鼠标multitouch的序号

signals:
    void deleteGameKey(int idx);

protected:
    bool eventFilter(QObject *obj, QEvent *evt) Q_DECL_OVERRIDE;

private:
    int m_index;
    QLineEdit *m_inputEdit = nullptr;
    QPushButton *m_bgBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    OverlayButton *m_overlayBtn = nullptr;

    QString m_keyString;
    QString m_storedKeyString;
    Qt::KeyboardModifier m_keyModifier;
    bool m_isLongWidthCurrently;
    bool m_isWidthSwitched;
    
    double event_coordinate_x;// 中心点
    double event_coordinate_y;// 中心点
    double m_short_width_scale_ratio;
    double m_long_width_scale_ratio;
    double m_height_scale_ratio;

    static QString composeKeyString(Qt::KeyboardModifiers modifier, QString key);
    static QString convertNativeScanCodeToString(quint32 keyCode);
    double getWidthScaleRatio();
    QPoint getCoordnate(QPoint pos);
    void tempUpdate();
};

#endif // SINGLE_KEY_H
