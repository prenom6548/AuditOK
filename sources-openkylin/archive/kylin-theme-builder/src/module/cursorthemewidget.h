/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef CURSORTHEMEWIDGET_H
#define CURSORTHEMEWIDGET_H

#include "cursorthemefeature.h"
#include "historywidget.h"
#include "basewidget.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QScrollArea>
#include <QLabel>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStackedWidget>
#include <QObject>
#include <QWidget>

class CursorThemeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CursorThemeWidget(QWidget *parent = nullptr);
    QMap<QString, QString>* getCursorMap();
    QMap<QString, QString> *getTimeCursorMap();
    void eidtInitWidget(const HistoryInfo &InfoData);
    void refresh();
    static bool g_themeChange;
    QMap<QString, bool> *m_themeChangeMap;
    QMap<QString, bool> *m_cursorPresenceMap;

    void useMaximumScale();
    void useRestoreScale();
    void setBuildBtnText(bool isglobal);
signals:
    void wallpaperupdate(const QString& filePath);
    void newCursorMap(QMap<QString, QString> *cursormap);
    void newTimeCursorMap(QMap<QString, QString> *timecursormap);
    void cursorthemeChange(QMap<QString, bool> *cursorthemeChangeMap);
    void build();
private:
    void initPreviewWidget();
    void initMaps();
    void initTimeMaps();
    void initRightWidget();
    void initBuildWidget();
    void initFormatWidget();
    void setHistoryCursor(QString type);
    void updateCustomPathMap(QMap<QString, QString>*historyMap,QString type);
    void setHistoryTimeCursor();
    QStringList getWidgetNamesFromFilesInDirectory(const QString& directoryPath);

    QMap<QString, QString> *getResourcesAppIconMap(QString Type);

    QHBoxLayout *m_cursorthemelayout;
    QWidget *m_rightwidget;
    QVBoxLayout *m_rightwidgetlayout;

    QMap<QString, CursorEditWidget*> *m_pathWidgetMap;
    QMap<QString, CursorEditWidget*> *m_timepathWidgetMap;
    QVBoxLayout *m_previewlayout;
    QString m_coverfilepath = ":/resource/background/background-glazing.png";

    QMap<QString, QString> *m_iconpathmap;
    QMap<QString, QString> *m_historyiconpathmap;
    QMap<QString, QString> *m_customiconpathmap;

    QMap<QString, QString> *m_timeiconpathmap;
    QMap<QString, QString> *m_historytimeiconpathmap;
    QMap<QString, QString> *m_timecustomiconpathmap;

    BasePreviewWidget *m_previewwidget;
    CursorImageWidget *m_preview;
    CursorImageWidget *m_preview2;
    QWidget *m_editwidget;
    QWidget *m_buildwidget;
    QWidget *m_formatwidget;
    HistoryInfo m_info;
    BatchImportDialog *m_batchimportdlg;
    QScrollArea *m_scrollArea;
    QPushButton *m_buildButton;
};

#endif // CURSORTHEMEWIDGET_H
