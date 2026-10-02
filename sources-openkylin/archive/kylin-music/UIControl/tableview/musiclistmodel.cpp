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

#include "musiclistmodel.h"
#include <QStandardItem>
#include <QVariant>
#include <QFile>

MusicListModel::MusicListModel(QObject* parent) : QObject(parent)
{
    titleList.append(tr("Song"));
    titleList.append(tr("Singer"));
    titleList.append(tr("Album"));
    titleList.append(tr("Time"));

    m_model.setHorizontalHeaderLabels(titleList); //使用list设置水平标题标签。如有必要，将列数增加到标签的大小
    m_model.horizontalHeaderItem(titleList.length()-1)->setTextAlignment(Qt::AlignRight);
    m_model.horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    m_model.horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
    m_model.horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
//    m_model.horizontalHeaderItem(2)->setTextAlignment(Qt::AlignRight);
//    m_model.item(0,0)->setTextAlignment(Qt::AlignLeft | Qt::AlignRight);
}


/**
 * @brief MusicListModel::add 增加音乐列表信息
 * @param info 具体的每项音乐列表信息
 * @return
 */
bool MusicListModel::add(musicDataStruct info)
{
    QStandardItem* root = m_model.invisibleRootItem();
    QStandardItem* item0 = new QStandardItem(); // 第一列
    QStandardItem* item1 = new QStandardItem(); // 第二列
    QStandardItem* item2 = new QStandardItem(); // 第三列
    QStandardItem* item3 = new QStandardItem(); // 第四列
//    QStandardItem* item4 = new QStandardItem();
//    QStandardItem* item5 = new QStandardItem();
//    QStandardItem* item6 = new QStandardItem();

    bool ret = true;
    if( (root != NULL) && (item0 != NULL) && (item1 != NULL) && (item2 != NULL) ) {
        // 每项元素增加提示
        item0->setData(info.title, Qt::ToolTipRole);
        item1->setData(info.singer, Qt::ToolTipRole);
        item2->setData(info.album, Qt::ToolTipRole);
        item3->setData(info.time, Qt::ToolTipRole);

        item0->setData(info.title, Qt::DisplayRole);
        item1->setData(info.singer, Qt::DisplayRole);
        item2->setData(info.album, Qt::DisplayRole);
        item3->setData(info.time, Qt::DisplayRole);

        // 每项元素文本对齐
        item0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item3->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // 使用这种方式和 `getItem()` 结合通过右键显示每行列表信息
        item0->setData(info.filepath, Qt::DecorationRole);
        item1->setData(info.filetype, Qt::DecorationRole);
        item2->setData(info.size, Qt::DecorationRole);


        // 每项元素不可点击
        item0->setEditable(false);
        item1->setEditable(false);
        item2->setEditable(false);
        item3->setEditable(false);
//        item4->setEditable(false);
//        item5->setEditable(false);
//        item6->setEditable(false);

        int newRow = count();

        root->setChild(newRow, 0, item0);
        root->setChild(newRow, 1, item1);
        root->setChild(newRow, 2, item2);
        root->setChild(newRow, 3, item3);
//        root->setChild(newRow, 4, item4);
//        root->setChild(newRow, 5, item5);
//        root->setChild(newRow, 6, item6);

        resList.append(info);
    } else {
        ret = false;
    }

    return ret;
}


/**
 * @brief MusicListModel::add 增加音乐列表信息
 * @param list 音乐列表信息，通过QList容器存储
 * @return
 */
bool MusicListModel::add(QList<musicDataStruct> list)
{
    bool ret = true;

    for(int i=0; i<list.count(); i++) {
        ret = ret && add(list[i]);
    }

    return ret;
}

bool MusicListModel::remove(int i)
{
    bool ret = true;

    if( (0 <= i) && (i < count()) ) {
        m_model.removeRow(i);
    } else {
        ret = false;
    }

    return ret;
}

void MusicListModel::clear()
{
    m_model.clear();
    resList.clear();
}


/**
 * @brief MusicListModel::setView 将模型Model和视图View结合起来
 * 经验三(Tip3)： 里氏替换原则（基类和子类之间的关系）
 *
 * 通过基类参数来指向子类，那么这个传入参数可以是继承于基类的多层级的子类，
 * 当子类不断修改，如修改为继承基类的其他继承类，可以不修改当前函数，以此降低setView函数的多次修改次数。
 * 也就是一个数据源多个视图，这多个视图，只需要间接继承于同一个基类就行。
 *
 * 例如，有以下派生关系：
 *
 *                      - -> QTableView
 * QAbstractItemView -> |
 *                      - -> QTreeView -> CustomTreeView -> 产生变量： m_view_tableOne (1)
 *                                                      |
 *                                                       -> 产生变量： m_view_tableHistory (2)
 *
 * 则，给函数的传入参数可以是间接继承于 QAbstractItemView 的 m_view_tableone 和 m_view_tableHistory 等变量。
 *
 * @param view 基类QAbstractItemView参数指向子类
 */
void MusicListModel::setView(QAbstractItemView &view)
{
    view.setModel(&m_model);
}


/**
 * @brief MusicListModel::getItem 获取当前行的音乐列表数据
 * 用于 "右键-》文件信息" 显示
 *
 * @param i 当前行索引
 * @return 获取的当前行音乐列表数据
 */
musicDataStruct MusicListModel::getItem(int i)
{
    musicDataStruct ret;

    if( (0 <= i) && (i < count()) ) {
        QModelIndex index0 = m_model.index(i, 0, QModelIndex());
        QModelIndex index1 = m_model.index(i, 1, QModelIndex());
        QModelIndex index2 = m_model.index(i, 2, QModelIndex());
        QModelIndex index3 = m_model.index(i, 3, QModelIndex());

        QVariant v0 = index0.data(Qt::DisplayRole);
        QVariant v1 = index1.data(Qt::DisplayRole);
        QVariant v2 = index2.data(Qt::DisplayRole);
        QVariant v3 = index3.data(Qt::DisplayRole);
        QVariant v4 = index0.data(Qt::DecorationRole);
        QVariant v5 = index1.data(Qt::DecorationRole);
        QVariant v6 = index2.data(Qt::DecorationRole);


        ret.title = v0.toString();
        ret.singer = v1.toString();
        ret.album = v2.toString();
        ret.time = v3.toString();
        ret.filepath = v4.toString();
        ret.filetype = v5.toString();
        ret.size = v6.toString();
    }
    return ret;
}


/**
 * @brief MusicListModel::getPathList 从数据库中获取要播放的歌单列表
 * @param listName 歌单列表名称
 * @return  获取的歌单列表
 */
QStringList MusicListModel::getPathList(QString listName)
{
    QStringList pathList;
    QList<musicDataStruct> musicInfoList;

    int ret = g_db->getSongInfoListFromDB(musicInfoList,listName);

    foreach (const musicDataStruct date, musicInfoList) {
#if 0
        QFile file(date.filepath);
        if (file.exists()) {
            // 当歌曲存在才往歌曲列表中添加，这种情况会将不存在的歌曲都不显示，与设计不符
            pathList.append(date.filepath);
        }
#endif
        pathList.append(date.filepath);
    }

    return pathList;
}

void MusicListModel::getMusicDateList(QList<musicDataStruct> list)
{
    list = resList;
}

int MusicListModel::count()
{
    return m_model.rowCount();
}
