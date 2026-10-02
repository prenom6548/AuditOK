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

#ifndef MEMORYINFO_H
#define MEMORYINFO_H

#include <QWidget>
#include <kinfolistitem.h>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QEvent>
#include <map>
#include <vector> 
#include <QMouseEvent>

#include "../../src/kajsondef.h"
#include "kainfotitle.h"
#include "kabaseinfopage.h"
#include "kalabel.h"

enum MMI_INDEX{
    MEMORYINFO_TITLE,
    MEMORYINFO_INVALID,
    MEMORYINFO_SLOT,
    MEMORYINFO_NAME,
    MEMORYINFO_FREQ,
    MEMORYINFO_BUSWIDTH,
    MEMORYINFO_TOTALCAPACITY,
    MEMORYINFO_USEDCAPACITY,
    MEMORYINFO_SERIALNUM,
    MEMORYINFO_MANUFACTURER,
    MEMORYINFO_DATAWIDTH,
    MEMORYINFO_TYPE,
    MEMORYINFO_SPEED,
    MEMORYINFO_CONFIGSPEED,
    MEMORYINFO_PIPE,
    MEMORYINFO_ARRAYHANDLE,
    MEMORYINFO_PARTNUMBER,
    MEMORYINFO_PHYSICALID,
    MEMORYINFO_MODEL
};

class MemoryInfo : public KABaseInfoPage
{
    Q_OBJECT
public:
    explicit MemoryInfo(QWidget *parent = nullptr);

    void initUI();

    void initConnections();
    void updateInfoItems(QString strInfoJson);

public slots:
    void onUpdateInfo(QString strInfoJson) override;
    void onRefreshInfo() override;

protected:
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    void addInfoItems(unsigned uGroup, unsigned uId, QString strTitle, QString strContent, bool bMulti);
    void clearInfoItems();
    void chkNoDevice();

private:
    QString m_strInfoJson;
    QString m_strPageInfo;
    QString m_strCopyInfo;
    KALabel* m_selectedLabel;
    std::vector<QString> vecIndex;
    std::vector<std::vector<QString>> vecItems;
    std::map<QString, QString> mapPageInfo;
    std::vector<std::map<QString, QString>> vecMapItems;

    QVBoxLayout *m_mainLayout = nullptr;
    QFrame *m_listFrame = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QScrollArea *m_scrollFrame = nullptr;
    QMenu *m_rkeyMenu = nullptr;

    QMap<unsigned,QMap<unsigned, QWidget*>> m_mapInfoItems;
};

#endif // MEMORYINFO_H
