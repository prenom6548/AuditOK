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


#ifndef DATAFILE_H
#define DATAFILE_H

#include <QObject>
#include <QIcon>
#include <QModelIndex>

#define NAME_LENGTH 103

class DataFile : public QObject
{
    Q_OBJECT
public:
    explicit DataFile(QString path, QObject *parent = nullptr, bool fromDisk = false);
    ~DataFile();

    bool isDirectory() const { return m_bIsDirectory; }
    bool isDisk() const { return m_bIsDisk; }
    QIcon   icon() const { return m_oIcon; }
    unsigned long size() const { return m_u64Size; }

    QString stringSize();

    QString display() const { return m_oDisplayName; }
    QString source() const { return m_oSource; }
    QString destination() const { return m_oDestination; }
    void setDestination(const QString& destination) { m_oDestination = destination;}
    DataFile *linkTarget() { return m_oLinkTarget; }
    void setLinkTarget(DataFile *t) { m_oLinkTarget = t; }
    QString type() const { return m_oType; }
    unsigned int level() const { return m_u32Level; }
    bool attrHidden() const { return m_bAttrHidden; }
    bool attrSymbolLink() const { return m_bAttrLink; }
    void rename(QString, bool add = false);
    void copy();
    void filter();
    void changeParent(DataFile *inputParent);
    bool valid() const { return m_bIsValid; }
    void setValid(bool b) { m_bIsValid = b; }
    void setModeIndex(QModelIndex idx) {m_oModelIndex = idx;}
    QModelIndex getModelIndex()  {return m_oModelIndex;}

public slots:
    void onUpdate(QString);
    void slotsDoHidde(bool);
    void slotsDoBroken(bool);
    void slotsDoRepleace(bool);

public:
    static bool isCopySuccess;

protected:
    unsigned int     m_u32Level;    //当前文件在整个数据集中的级别
    bool             m_bIsDirectory;//当前文件是否为文件夹
    bool             m_bAttrHidden; //当前文件是否有隐藏属性
    bool             m_bAttrLink;   //当前文件是否为连接文件
    bool             m_bIsDisk;     //当前文件来源是否为光盘
    bool             m_bIsValid;
    unsigned long    m_u64Size;     //当前文件大小(字节单位)
    QString          m_oDisplayName;//当前文件展示名称
    QString          m_oSource;     //当前文件源
    QString          m_oDestination;//当前文件将被缓存的目的路径
    QString          m_oType;       //当前文件类别
    QString          m_oCreateTime; //当前文件创建时间
    QString          m_oAddTime;    //当前文件添加到刻录内时间
    DataFile        *m_oLinkTarget; //当前文件连接的目标文件
    QIcon            m_oIcon;       //当前文件类别对应主题内图标
    QModelIndex      m_oModelIndex; //记录当前文件在TreeView中的ModelIndex

Q_SIGNALS:
    // logic
    void sizeChanged(unsigned long, bool);
    void fileChanged(bool, DataFile *file);
    void directoryChanged(bool, DataFile *dir);

    // child
    void subFileChanged(DataFile *, bool);

    // copy
    void copied(QString, QString);
    void update(QString);

    // filter
    void hidden(bool flag = true);
    void broken(bool flag = true);
    void repleace(bool flag = true);
    void doHidden(void *);
    void doBroken(void *);
    void doRepleace(void *);
};

#endif // DATAFILE_H
