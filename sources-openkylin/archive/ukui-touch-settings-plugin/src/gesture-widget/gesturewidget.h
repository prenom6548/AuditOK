/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GESTUREWIDGET_H
#define GESTUREWIDGET_H

#include <QPushButton>

class QLabel;
class QString;
class TextLabel;
class QHBoxLayout;
class QVBoxLayout;
class QGSettings;
class PlayIconLabel;

class GestureWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit GestureWidget(QWidget *parent = nullptr);
    ~GestureWidget() override = default;

    void setGestureDescription(const QString &text);
    void setGestureAction(const QString &text);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void focusOutEvent(QFocusEvent *e) override;

public slots:
    void updateLabelTextColor(bool checked);

signals:
    void getFocus();
    void loseFocus();

private:
    TextLabel* m_gestureDescription = nullptr;
    TextLabel* m_gestureAction = nullptr;
    PlayIconLabel* m_playIcon = nullptr;

    void updateActionMaxWidth();
    void initProperty();
    void initConnections();
    void initStyleConnection();
    void initUI();
    QHBoxLayout* createGestureNameLayout();
    QVBoxLayout* createMainLayout();
    void onStyleChanged();
    void onIconThemeChanged();
    QString getCurrentStyleName() const;
};

#endif // GESTUREWIDGET_H
