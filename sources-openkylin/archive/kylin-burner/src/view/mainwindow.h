/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "windowtitle.h"

#include <QMainWindow>
#include <QMutex>
#include <QButtonGroup>
#include <QStandardItem>
#include <QByteArray>
#include <QGSettings/QGSettings>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QPushButton>
#include <map>
#include <QRect>

#include "knavigationbar.h"
#include <windowmanager/windowmanager.h>
#include <kysdk/applications/klabel.h>

using namespace kdk;

const QByteArray TRANSPARENT_SCHEMAS = "org.ukui.control-center.personalise";

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class UkuiWindowHelper;
class DataCDROM;

class MyAlignRightLabel : public QLabel{
    Q_OBJECT
public:
    MyAlignRightLabel(QWidget* parent = nullptr) : QLabel(parent){
        setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        QString originalText = text();
        if (originalText.isEmpty()) {
            QLabel::paintEvent(event);
            return;
        }
        
        // 计算省略文本
        QFontMetrics fm = fontMetrics();
        QRect rect = contentsRect();
        QString elidedText = fm.elidedText(originalText, Qt::ElideRight, rect.width());
        
        if (elidedText != originalText) {
            setToolTip(originalText);
        } else {
            setToolTip("");
        }
        
        blockSignals(true);
        setText(elidedText);
        QLabel::paintEvent(event);
        setText(originalText);
        blockSignals(false);
    }
};

class MainWindow : public QMainWindow
{
    friend class MainWindowTest;
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.burner.manager")

Q_SIGNALS:
    void busness(int);
    void position(QPoint);
    void browse(QString);
    void delData(int);

public:
    MainWindow(QStringList args, QWidget *parent = nullptr);
    static MainWindow* Instance(QStringList args = QStringList(), QWidget *parent = nullptr);
    ~MainWindow();
    bool currentDataRomIsUDF();
    int getCountDevice(){ return m_oCDROMS.size();}
    QString getDeviceByIndex(int index);
    WindowId getWinId() { return m_lastestWindowId; }

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    int slotsReShowMainWindow(QStringList args);
    bool isBusyDevice(QString device);
    void slotChangeUIFontSize(int fontSize);
    void slotChangeToImageBurnPage(QString imagePath);
    void slotChangeWindowState();
    void slotChangeStackWindowIndex(QModelIndex index);
    void slotSwitchBurnFormat(DataCDROM* cdrom);

private slots:
    void slotsBurnData();
    void slotsBurnImage();
    void slotsCopyDisc();
    void slotsAdjustBurnData(int idx);
    void slotsBusness(bool);
    void slotsInitCreateISO(void *);
    void slotsNoMediun(void *);
    void slotsRemoveCDROM(void *);
    void slotsBrowseOrSetting(bool);
    void slotsUpdateDisk(int, void *);
    void slotsChangeName(int);
    void slotsChangeBtnBusnessName();
    void uiModeChange(bool);
    void slotsCloseApp();
    void slotsChangeCurrentIndex(int);
    void slotShowTileFlyout(QRect rect);
    void slotHideTileFlyout();

private:
    bool started();
    void uiAdjust();
    void initTransparent();
    void calculatorTime(int current);
    void resetText(QPushButton* item);
    void setAccessibleName();

private:
    QButtonGroup   *m_oGroup;
    QList<void *>   m_oCDROMS;
    QGSettings     *m_transparentSettings;
    double          m_transparentValue;
    QStandardItem  *m_burnDataItem;
    QStandardItem  *m_burnImageItem;
    QStandardItem  *m_copyImageItem;
    KNavigationBar *m_NavBar;
    UkuiWindowHelper *m_windowHelper;
    QDBusMessage          msg;
    static MainWindow     *m_mainWindow;
    static QMutex          m_oMutex;
    bool m_isTabletMode;
    static bool firstAddFlag;
    int m_current_page = 0;
    QString m_lastestWindowId;
    QString m_mainWindowId;
    QTimer          *m_maxBtnHoverDelayTimer;
    QRect           m_tileFlyoutRect;

    std::map<int, qint64> m_mapStartTime = {
        {0, 0},
        {1, 0},
        {2, 0}
    };

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
