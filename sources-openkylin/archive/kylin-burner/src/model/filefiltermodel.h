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


#ifndef FILEFILTERMODEL_H
#define FILEFILTERMODEL_H

#include <QAbstractItemModel>

class DataDirectory;
class FileFilterModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class FileFilterModelTestFriend;
    friend class FileFilterModelTest;
Q_SIGNALS:
    void node(void*, QIcon, QString);
    void root(QModelIndex);
    void allFilesRestored(QList<void *> hiddens, QList<void *> brokens, QList<void *> replaces);
public:
    explicit FileFilterModel(QObject *parent = nullptr);
    static FileFilterModel *instance();

    // Header
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public:
    int filtered();
    void addFilterNode(void *inputDevice);
    void addFilterData(int, void *);

    void updateIndex(int idx)
    {
        m_iIndex = idx;
        root(createIndex(idx, 0, m_oRoots.at(idx)));
    }

    void restoreFiles();

    QList<void *> GetFilteredHiddenFiles(int, int);
    QList<void *> GetFilteredBadLinkFiles(int, unsigned int);
    QList<void *> GetFilteredReplacedLinkfiles(int, int);

public slots:
    void deleteFilterNode(int);
    DataDirectory* getCurrentRoot();


public:
    int            m_iIndex;
    QList<void *>  m_oRoots;
private:
    QStringList    m_oHeaders;
};

#endif // FILEFILTERMODEL_H
