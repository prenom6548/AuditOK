/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
class Button : public QPushButton
{
    Q_OBJECT
public:
    explicit Button(const QString& text, const QString& img, QWidget *parent = nullptr);

    void setText(const QString& text);
    void setImage(const QString& image);
    void setStatus(bool state);
    void setMode(int mode);
    int getMode();
    QPixmap drawPixmap(const QPixmap &source);

    void setModeImage(const QString& modeImage);
private:
    void initButtonUi();
private:
    QHBoxLayout* m_mainLayout = nullptr;
    QLabel* m_imageLabel = nullptr;
    QLabel* m_textLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QString m_text;
    QString m_modeImage;
    int m_mode;

};

#endif // BUTTON_H
