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
#ifndef ICONTHEMEWIDGET_H
#define ICONTHEMEWIDGET_H

#include "iconwidgetfeature.h"
#include "../fileProcess/filecheck.h"
#include "historywidget.h"
#include "basewidget.h"
#include "batchimportdialog.h"

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
#include <QTabBar>
#include <ktabbar.h>
#include <QScrollArea>

class IconThemeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IconThemeWidget(bool isHistory,QWidget *parent = nullptr);
    void initPreviewWidget();

    void initEditWidget();
    void initSystemEditWidget();
    void initPlaceandDeviceEditWidget();
    void initDeviceEditWidget();
    void initBatchEditWidget();

    void initRightWidget();

    QMap<QString, QString> *getResourcesAppIconMap(QString Type);

    void initMaps();
    void initSystemMaps();
    void initPlacesMaps();
    void initDevicesMaps();

    QMap<QString, QString>* getAppIconsMap();
    QMap<QString, QString>* getSystemIconsMap();
    QMap<QString, QString>* getPlaceIconsMap();
    QMap<QString, QString>* getDeviceIconsMap();

    void setHistoryAppIcon();
    void setHistorySystemIcon();
    void setHistoryPlaceIcon();
    void setHistoryDeviceIcon();

    void updateCustomIconPathMap(QMap<QString, QString>*historyMap);
    void updateCustomSyetemIconPathMap(QMap<QString, QString>*historyMap);
    void updateCustomPlaceIconPathMap(QMap<QString, QString>*historyMap);
    void updateCustomDeviceIconPathMap(QMap<QString, QString>*historyMap);

    void eidtInitWidget(const HistoryInfo &InfoData);
    void refresh();

    static bool g_themeChange;
    QMap<QString, bool> *m_themeChangeMap;
    QMap<QString, bool> *m_iconPresenceMap;
    void useMaximumScale();
    void useRestoreScale();
    void setBuildBtnText(bool isglobal);
signals:
    void wallpaperupdate(const QString& filePath);

    void newAppIconsMap(QMap<QString, QString> *appiconsmap);
    void newSystemIconsMap(QMap<QString, QString> *systemiconsmap);
    void newPlaceIconsMap(QMap<QString, QString> *placeiconsmap);
    void newDeviceIconsMap(QMap<QString, QString> *deviceiconsmap);

    void iconthemeChange(QMap<QString, bool> *iconthemeChangeMap);
    void batchIconUpdate(const QString& filePath);
    void build();
private:
    QStringList getWidgetNamesFromFilesInDirectory(const QString& directoryPath);

    QStackedWidget *m_themestackedwidget;
    QWidget *m_rightwidget;
    QVBoxLayout *m_rightwidgetlayout;
    QStackedWidget *m_stackedwidget;
    QWidget *m_batchwidget;

    kdk::KTabBar *m_tabbar;
    QPushButton *m_appiconbtn;
    QPushButton *m_systemiconbtn;

    QMap<QString, QString> *m_iconpathmap;
    QMap<QString, QString> *m_historyiconmap;
    QMap<QString, QString> *m_customiconpathmap;
    QMap<QString, IconEditWidget*> *m_pathWidgetMap;

    QMap<QString, QString> *m_systemiconpathmap;
    QMap<QString, QString> *m_historysystemiconmap;
    QMap<QString, QString> *m_systemcustomiconpathmap;
    QMap<QString, IconEditWidget*> *m_systempathWidgetMap;

    QMap<QString, QString> *m_placesiconpathmap;
    QMap<QString, QString> *m_historyplacesiconmap;
    QMap<QString, QString> *m_placescustomiconpathmap;
    QMap<QString, IconEditWidget*> *m_placespathWidgetMap;

    QMap<QString, QString> *m_devicesiconpathmap;
    QMap<QString, QString> *m_historydevicesiconmap;
    QMap<QString, QString> *m_devicescustomiconpathmap;
    QMap<QString, IconEditWidget*> *m_devicespathWidgetMap;

    BasePreviewWidget *m_previewwidget;
    ImageWidget *m_preview;
    ImageWidget *m_systempreview;
    ImageWidget *m_placepreview;

    QStackedWidget *m_previewstack;
    HistoryInfo m_info;
    QWidget *m_editwidget;
    QWidget *m_systemeditwidget;
    QWidget *m_placeeditwidget;

    QScrollArea *m_scrollArea;
    QScrollArea *m_systemscrollArea;
    QScrollArea *m_placescrollArea;
//    QScrollArea *m_systemscrollArea;
    BatchImportDialog *m_batchimportdlg;
    QPushButton*m_buildButton;
};

#endif // ICONTHEMEWIDGET_H
