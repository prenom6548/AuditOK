/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DRIVEINFOPAGE_H
#define DRIVEINFOPAGE_H
#define BLANKNUM 10
#define BLANKNUM_EN 20

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QEvent>
#include <map>
#include <vector> 
#include <QMouseEvent>

#include "kdriveinfoitem.h"
#include "../../src/commondef.h"
#include "../../src/kajsondef.h"
#include "kdrivelabel.h"

enum DI_TYPE{
    DRIVEINFOTYPE_MOTHERBOCARD,     // 主板
    DRIVEINFOTYPE_GRPHICS,          // 显卡
    DRIVEINFOTYPE_NETWORK,          // 有线网卡
    DRIVEINFOTYPE_AUDIO,            // 声卡
    DRIVEINFOTYPE_WIFI,             // 无线网卡
    DRIVEINFOTYPE_BLUETOOTH,        // 蓝牙
    DRIVEINFOTYPE_OTHER,            // 其他
};

typedef struct _DriveInfo_s
{
    QString strName;
    QString strType;
    QString strVersion;
}DriveInfo;

class DriveInfoPage : public QWidget
{
    Q_OBJECT
public:
    explicit DriveInfoPage(QWidget *parent = nullptr);

    void initUI();

    void initConnections();
    void updateInfoItems(QString strInfoJson);
    int getBlankNum();

public slots:
    void onUpdateInfo(QString strInfoJson);
    void onCopyContent(QString strPageInfo);
    void onSelectAllLabel(KDriveLabel* selectedLabel);

protected:
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void clearInfoItems();
    void refreshInfoItems();
    DI_TYPE getDriveType(QString strType);

private:
    QString m_strInfoJson;
    QString m_strPageInfo;
    QString m_strCopyInfo;
    KDriveLabel* m_selectedLabel;
    std::vector<QString> vecIndex;
    std::map<QString, QString> mapPageInfo;

    QHBoxLayout *m_mainLayout = nullptr;
    QFrame *m_listFrame = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QScrollArea *m_scrollFrame = nullptr;

    QMap<unsigned, QList<KDriveInfoItem*>> m_mapItems;
    QMap<DI_TYPE, QList<DriveInfo>> m_mapDriveInfo;
    QMenu *m_rkeyMenu = nullptr;
};

#endif // DRIVEINFOPAGE_H
