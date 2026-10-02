/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
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

#ifndef CUSTOMTOOLBUTTON_H
#define CUSTOMTOOLBUTTON_H

#include <QToolButton>
#include <QPainter>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLabel>
#include <QtWidgets>
#include <QEvent>
#include <QGSettings>

const QString IS_SELECT = "IS_SELECT";
class CustomToolButton : public QToolButton
{
     Q_OBJECT

public:
    CustomToolButton(QWidget *parent = 0);
    ~CustomToolButton();

    void initGsettings();
    void initLayout();
    void setLablePixmap(QString img);
    void setLableText(QString text);
    QString getLableText();
    void setLabelFont(const QFont &font);
    int textLabelWidth() const;
    void setSelectedAndHoverdStyle();
    void selectChanged();
    void defaultStyle();
    void requestCustomContextMenu(const QPoint &pos);

    QString pluginBtnHoverColor(QString styleName, bool hoverFlag);

    QAction *deleteAct = nullptr;
    QAction *renameAct = nullptr;
    QString buttonListName = "";

Q_SIGNALS:
    void playall(QString text);
    void renamePlayList(QString text);
    void removePlayList(QString text);
    void selectButtonChanged(QString text);
    void sigShowMenu();

private:
    bool m_isEnter = false;
    QGSettings *qtSettings = nullptr;

    QLabel *m_iconLable = nullptr;
    QLabel *m_textLabel = nullptr;
    QHBoxLayout *m_hboxLayout = nullptr;

    QString hoverColor;
    QString clickColor;
};

#endif
