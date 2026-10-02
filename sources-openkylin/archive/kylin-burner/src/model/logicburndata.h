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


#ifndef LOGICBURNDATA_H
#define LOGICBURNDATA_H

#include "src/view/burndata.h"

#include <kysdk/applications/kmessagebox.h>

#include <QMutex>
#include <QAbstractItemModel>
#include <QIcon>

class DataFile;
class DataDirectory;
class Progress;
class LogicBurnData : public QAbstractItemModel
{
    Q_OBJECT
    friend class MainWindowTest;
    friend class BurnDataTest;
public:
    explicit LogicBurnData(QObject *parent = nullptr);

    // Header
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable
    bool setData(const QModelIndex &index, const QVariant &value,int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

public:
    bool isUp(QModelIndex);
    bool isFolder(QModelIndex);
    bool checkAdd(QModelIndex);
    bool checkAdd(QModelIndexList);
    bool checkDel(QModelIndexList);
    bool checkCls(QModelIndex);
    bool checkDefaultAddpage(QModelIndex);
    bool checkOpen(QModelIndexList);
    bool checkOpenLocal(QModelIndexList);
    bool checkProperties(QModelIndexList);
    bool checkSelectModelItem(QModelIndexList);
    bool checkSize();

    QString stringSize(unsigned long);
    int index() const { return m_iIndex; }
    unsigned int level(QModelIndex idx = QModelIndex());
    int filter(QModelIndex idx = QModelIndex());

    bool resetFileterConfigToModel();

    int doFilterHidden(QModelIndex idx = QModelIndex());
    int doRecoveryHidden(QList<void *>, QModelIndex);
    int doFilterBroken(QModelIndex idx = QModelIndex());
    int doRecoveryBroken(QList<void *>, QModelIndex);
    int doFilterRepleace(QModelIndex idx = QModelIndex());
    int doRecoveryRepleace(QList<void *>,QModelIndex);
    void fixFileDstPathByIndex(QString newPath, QModelIndex idx); /* 修改文件(夹)的目标路径(拷贝到刻录的指定路径) */

    int findTheRootIndexofFile(DataFile* inputFile);

    int recoverHiddenFilesByDeviceIndex(QList<void*> dataList,int deviceIndex);
    int recoverBrokenFilesByDeviceIndex(QList<void*> dataList,int deviceIndex);
    int recoverReplaceLinkFilesByDeviceIndex(QList<void*> dataList,int deviceIndex);
    int getDeviceCount();
    bool updateListOrder();  // 保持m_oBurners列表顺序与MainWindow的顺序一致

    int appendBurnDataUDF102();
    int formatUDF102();

    QModelIndex modelIndexForDataFile(DataFile *file) const;

public slots:
    void changeIndex(int);

    int addDatasToIndex(QStringList, QModelIndex);
    int deleteDatasFromIndex(QModelIndexList, QModelIndex);

    int clearDatasUnderIndex(QModelIndex);
    int clearDatasUnderIndexd(QModelIndex);
    bool duplicatedName(QString, QModelIndex);
    void addEmptyFolder(QString, QModelIndex);
    void deleteRoot(int);
    void slotRestoreAllFiles(QList<void *> hiddens, QList<void *> brokens, QList<void *> replaces);
    void stopLoadingData();

    static QUrl convertTrashUrlToFileUrl(const QUrl& trashUrl);
    QModelIndex modelIndexOfDirectory(DataDirectory *dir) const;

private slots:
    void slotsSetting();
    void slotsCreateISO(void *inputCDrom);
    void slotsCreateISO();
    void slotsBurnData();
    void slotsBurnUDFData();
    void slotsEmptyMedium(void *);
    void slotsUpdateCDROM();
    void slotsDoFilterHidden(void *);
    void slotsDoFilterBroken(void *);
    void slotsDoRepleace(void *);
    void slotsUpdateSize(unsigned long);
    bool slotsBurnFileByEngine();

public slots:
    void slotsAsynDeleteRoot(int idx);

private:
    int addDiskData(QStringList, void *);
    int calcFileCount(QStringList intputList);
    int calcFileCountInDir(QString inputDirPath);
    int addDirectoy(QString, void *, bool disk = false);
    int addFile(QString, void *, bool isDisk = false);
    bool checkTempPathAvailabeSize();
    int createIsoByCommand(QString volumeName, QString destPath);
    void progressWidgetDelayResponeTimer(int ms = 300);

private:
    int           m_iIndex;
    bool          m_addAction_canceled;
    bool          m_bAppRunning;
    QStringList   m_oHHeaders;
    QStringList   m_oVHeaders;

    QList<void *> m_oBurners;
    bool          m_isContinue;
    QList<QMutex*> m_mutex_list;
    DataDirectory*     m_current_item;
    int           m_file_device_index;
    Progress*     m_progressDlg;

public:
    QList<void *> m_oRoots;

Q_SIGNALS:
    void setting(void *);
    void rootIndex(QModelIndex);
    void sizeChanged(unsigned long);
    void rootIndexChanged(QModelIndex);

    // progress
    void title(QString);
    void setRange(int, int);
    void setValue(int, QIcon icon = QIcon(), QString str = QString());
    void addValue(int, QIcon icon = QIcon(), QString str = QString());

    void defaultAddPageEnabled(bool);
    void addEnabled(bool);
    void delEnabled(bool);
    void clsEnabled(bool);
    void folderEnabled(bool);

    void updateDisk(int, void *);

    // filter
    void newFilterNode(QIcon, QString);

    void hiddenFileFiltered(int, void *);
    void brokenLinkfileFiltered(int, void *);
    void replaceLinkFileFiltered(int, void *);
};

#endif // LOGICBURNDATA_H
