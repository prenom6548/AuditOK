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


#ifndef DATADIRECTORY_H
#define DATADIRECTORY_H

#include <QPointer>

#include "datafile.h"
#include "src/view/inputchange.h"

class DataDirectory : public DataFile
{
    Q_OBJECT
public:
    explicit DataDirectory(QString path, QObject *parent = nullptr, bool fromDisk = false);
    ~DataDirectory();
public:
    DataFile *at(int);
    int findIndex(DataFile *);
    int findIndex(DataDirectory *);

    bool take(DataFile *);
    void append(DataFile *inputFile);

    int count() const;
    int counts() const { return m_oChildren.size(); }

    unsigned long total() const { return m_u64Dirs + m_u64Files; }
    unsigned long nSize() const { return m_u64NewDataSize; }
    bool isRoot() const { return m_bIsRoot; }
    bool canClean() const;
    bool duplicate(QString) const;
    void rename(QString, bool add = false);
    void updateChild(QString, QString);
    void clearChildren();
    void clear();
    void clean();
    void copy();
    void *findChildByDisplay(QString, bool child = false);
    static void setFilterHidden(bool filter);

    /* 重命名后修改文件、目录(下所有子文件)被缓存的目的路径 */
    void setFileDestination(QString name);
    void setAllChildFileDestinationByPath(QString fatherPath);

    bool filterHidden() const {
        // 优先使用节点特定的过滤规则，如果没有设置则使用全局规则
        return m_bNodeFilterHidden.isValid() ? m_bNodeFilterHidden.toBool() : g_bFilterHidden;
    }
    void setNodeFilterHidden(bool filter) {
        m_bNodeFilterHidden = filter;
    }
    static void setFilterBrokenSymbolLinks(bool filter);
    bool filterBrokenSymbolLinks() const {
        // 优先使用节点特定的过滤规则，如果没有设置则使用全局规则
        return m_bNodeFilterBroken.isValid() ? m_bNodeFilterBroken.toBool() : g_bFilterBroken;
    }
    void setNodeFilterBrokenSymbolLinks(bool filter) {
        m_bNodeFilterBroken = filter;
    }
    static void setRepleaceSymbolLinks(bool filter);
    bool repleaceSymbolLinks() const {
        // 优先使用节点特定的过滤规则，如果没有设置则使用全局规则
        return m_bNodeRepleace.isValid() ? m_bNodeRepleace.toBool() : g_bRepleace;
    }
    void setNodeRepleaceSymbolLinks(bool filter) {
        m_bNodeRepleace = filter;
    }

    // 节点特定配置的保存和加载
    void loadNodeFilterConfig();
    QString getNodeConfigKey() const;
    void execHidden(bool isEnable = true);
    void execBroken(bool b = true);
    void execRepleace(bool b = true);
    void turnViewFilterFileToHidden();

    int  CountOneLevelNotDiskFile();
    bool isGreateThan2GByNewAddSingleFile(); // 是否有新加入的文件>2G

public slots:
    void doSize(unsigned long, bool);
    void doDirectory(bool, DataFile *);
    void doFile(bool, DataFile *);
    void appendOrDeleteSubfile(DataFile *, bool);
    void closeInputChangeDlg();
    bool isRenameDlgShown();

private:
    QPointer<InputChange>       m_inputchange_dlg;    //界面
    bool                        m_bIsRoot;       //是否为根节点
    unsigned long               m_u64Dirs;       //当前文件夹下包含文件夹数量
    unsigned long               m_u64Files;      //当前文件夹下包含文件数量
    unsigned long               m_u64NewDataSize;//当前文件夹下新添加文件大小
    QList<DataFile *>           m_oChildren;     //当前文件夹下子文件
    QHash<QString, DataFile *>  m_oHashChildren; //当前文件夹下唯一子文件
    static bool g_bFilterHidden;                 //是否执行隐藏文件过滤
    static bool g_bFilterBroken;                 //是否执行损坏连接文件过滤
    static bool g_bRepleace;                     //是否执行连接文件替换
public:
    // 节点特定的过滤规则（QVariant 用于区分是否已设置）
    QVariant m_bNodeFilterHidden;                //节点特定的隐藏文件过滤
    QVariant m_bNodeFilterBroken;                //节点特定的损坏链接过滤
    QVariant m_bNodeRepleace;                    //节点特定的链接替换
Q_SIGNALS:
    void updateSize(unsigned long);
    void folderHidden(bool);
    void folderBroken(bool);
    void  folderReplaced(bool);
};

#endif // DATADIRECTORY_H
