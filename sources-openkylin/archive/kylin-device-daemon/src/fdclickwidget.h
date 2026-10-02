/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef __FDCLICKWIDGET_H__
#define __FDCLICKWIDGET_H__
#include <QWidget>
#include <QProcess>
#include <QDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <qgsettings.h>
#include <QPropertyAnimation>
#include <QTimer>

#include "ejectInterface.h"
#include "clickLabel.h"
#include "UnionVariable.h"
#include "interactivedialog.h"
#include "gpartedinterface.h"
#include "fdframe.h"

typedef struct _EjectDeviceInfo_s
{
    void* pVoid = nullptr;
    QString strDriveId = "";
    QString strDriveName = "";
    QString strVolumeId = "";
    QString strVolumeName = "";
    QString strMountId = "";
    QString strMountUri = "";
    unsigned uFlag = 0;
}EjectDeviceInfo;

class MainWindow;
class FDClickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FDClickWidget(FDFrame *parent = nullptr,
                          unsigned diskNo = 0,
                          QString strDriveId = "",
                          QString strVolumeId = "",
                          QString strMountId = "",
                          QString driveName = "",
                          QString volumeName = "",
                          quint64 capacityDis = 0,
                          QString strMountUri = "",
                          int uVolumeSum = 0);
    ~FDClickWidget();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    QIcon imgIcon;
    unsigned m_uDiskNo;
    QString m_driveName;
    QString m_volumeName;
    quint64 m_capacityDis;
    QString m_mountUri;
    QString m_driveId;
    QString m_volumeId;
    QString m_mountId;
    MainWindow *m_mainwindow;
    QPoint mousePos;
    QLabel *image_show_label = nullptr;
    QLabel *m_driveName_label;
    ClickLabel *m_nameDis1_label;
    QLabel *m_capacityDis1_label;
    QWidget *disWidgetNumOne;
    QWidget *driveWidget;
    QVBoxLayout *main_V_BoxLayout = nullptr;
    bool m_mousePressFlag = false;

    QGSettings *fontSettings = nullptr;
    QGSettings *qtSettings = nullptr;

    int fontSize;
    QString currentThemeMode;
    QString m_strCapacityDis;

    FDFrame *m_frame;
    int m_width = 320;
    bool m_letfToRight = true;

public:
    ClickLabel *m_eject_button = nullptr;
    interactiveDialog *chooseDialog = nullptr;
    gpartedInterface *gpartedface = nullptr;
    bool ifSucess = false;
    FDClickWidget * m_fristClickWidget = nullptr;
Q_SIGNALS:
    void clicked();
    void clickedEject(EjectDeviceInfo eDeviceInfo);
    void noDeviceSig();
    void themeFontChange(qreal lfFontSize);
    void hoverEjectBotton();
    void leaveEjectBotton();

private Q_SLOTS:
    void on_volume_clicked();
    void switchWidgetClicked();
    void onThemeFontChange(qreal lfFontSize);
    void on_hideinterface();

private:
    QString size_human(qlonglong capacity);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
    void openFirstVolume();
protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent * event);
public:
    void initFontSize();
    void initThemeMode();
};

#endif // __FDCLICKWIDGET_H__
