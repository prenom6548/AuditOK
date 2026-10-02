/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include "picturetowhite.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QCheckBox>
#include <QMenu>
#include <QProcess>
#include <QTextCodec>
#include <QtXml/QDomDocument>
#include <qxmlstream.h>
#include <QMessageBox>
#include <QMenu>
#include <QDebug>
#include <QFileInfo>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDBusInterface>
#include <QDBusReply>
#include <QPainter>
#include <QTranslator>
#include <QStackedWidget>
#include <QToolButton>

#include <kmenubutton.h>
#include <windowmanager/windowmanager.h>

using namespace kdk;

#define ORG_UKUI_STYLE_FONT            "org.ukui.style"
#define GSETTING_FONT_KEY              "systemFontSize"

class titleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit titleWidget(QWidget *parent = nullptr);


    QHBoxLayout    *m_pTitleLayout;

    QLabel         *m_pLogoLabel;
    QLabel         *m_pNameLabel;

    QPushButton    *m_pClostBtn;
    QPushButton    *m_pMinBtn;
    QToolButton    *m_pSetBtn;


    QGSettings     *m_pGsettings;
    QGSettings     *m_pGsettingFontSize;

    QMenu* m_menu;

    QFont getSystemFont();

    void focuschangeEvent(bool);

private:
    bool selfHasFocus = false;
};

#endif // TITLEWIDGET_H
