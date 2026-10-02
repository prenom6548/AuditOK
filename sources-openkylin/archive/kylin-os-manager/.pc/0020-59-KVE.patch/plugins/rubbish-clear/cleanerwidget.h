/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef CLEANERWIDGET_H
#define CLEANERWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include "cleandetailveiw.h"
#include "clearmainwidget.h"
#include "dbus/dataworker.h"

namespace KylinRubbishClear
{

class CleanerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CleanerWidget(QWidget *parent = 0);
    ~CleanerWidget();
    // void setToolKits(Toolkits* kits) { toolKits = kits;}
    void resetSkin(QString skin);

    void paintEvent(QPaintEvent *);

public Q_SLOTS:
    void displayDetailPage();
    void displayMainPage();
    void onInitDataFinished(); //表明dbus初始化完成

Q_SIGNALS:
    void transCleanSignal();
    void transScanSignal();
    void tranActionAnimaitonSignal();
    void tranScanOverSignal(bool status);

    void tellCleanerDetailData(const QStringList &data);
    void tellCleanerDetailStatus(const QString &status);

    void startScanSystem(QMap<QString, QVariant> itemsMap);
    void startCleanSystem(QMap<QString, QVariant> itemsMap);

    void startOneKeyScan(const QStringList &categorys);
    void isScanning(QString msg);
    void finishScanWork(QString msg);
    void tellScanResult(QString flag, QString msg);
    void startOneKeyClean();

    void finishCleanWorkMain(QString msg);

    void tellCleanerMainData(const QStringList &data);
    void tellCleanerMainStatus(const QString &status, const QString &domain);
    void sendCleanErrorSignal(const QString &status);

    void policykitCleanSignal(bool status);
    void sendCleanOverSignal();

private:
    QString desktop = "";
    QStackedWidget *statked_widget;
    ClearMainWidget *main_widget = nullptr;
    CleandetailVeiw *detailview = nullptr;

    DataWorker *m_dataWorker = nullptr;
    int themeColor = 1; //默认1是深色，0是浅色

    void startDbusDaemon();

    void initThemeGetting();
    void changeThemeColor(int color);

    int mFontSize = 0;
    void changeSystemFontSize(int size);
};

} // namespace KylinRubbishClear


#endif // CLEANERWIDGET_H
