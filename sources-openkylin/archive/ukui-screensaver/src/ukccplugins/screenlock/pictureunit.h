/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef PICTUREUNIT_H
#define PICTUREUNIT_H

#include <QLabel>
#include <QMouseEvent>
#include <QEvent>

class PictureUnit : public QLabel
{
    Q_OBJECT

public:
    explicit PictureUnit(QWidget *parent = nullptr);
    ~PictureUnit();

public:
    void setFilenameText(QString fn);
    QString filenameText();
    void changeClickedFlag(bool flag);
    bool getClickedFlag();
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
public:
    QString _filename;
    QString clickedStyleSheet;
protected:
    void mouseReleaseEvent(QMouseEvent * e);
    bool eventFilter(QObject *w, QEvent *e);

private:
    bool clickedFlag;
    QString hoverStyleSheet;
    bool isTouchControl;
    QPointF touchBeginPoint;
    QPointF touchEndPoint;

Q_SIGNALS:
    void clicked(QString filename);

};

#endif // PICTUREUNIT_H
