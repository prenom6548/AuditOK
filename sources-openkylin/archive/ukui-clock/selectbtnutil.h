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
#ifndef SELECTBTNUTIL_H
#define SELECTBTNUTIL_H

#include <QObject>
#include <QList>
#include <QSqlTableModel>
#include <QMap>
#include "kylinqfiledialog.h"
class SelectBtnUtil : public QObject
{
    Q_OBJECT
public:
    explicit SelectBtnUtil(QObject *parent = nullptr);
    ~SelectBtnUtil();
    int getBellListSize();
    int getBellIndexById(QString id);
    QString getBellIdByIndex(int index);
    QString getBellNameById(QString id);
    QString getBellPathById(QString id);
    QString getDefaultBellId();
    QString getBellIdByNameEn(QString nameEn);
    QString checkBellExists(QString filePath);
    void refreshBellData();
    static QString getDefaultBellStr();
    static QString getDefaultBellEn();
    static QList<QString> * getDefaultBellList();
    static QList<QString> * getDefaultBellTrList();
    QString copyAudioFile(QString from);
    QString getTargetPath(QString from);
    bool copyFile(QString from ,QString to);
    QString getFileNameFromPath(QString path);
    QString getFileNameWithoutSuffix(QString FileName);
    QString saveToBellTable(QString filePath);

    QString openAudioFileDialog(QWidget * parent);

    QSqlQuery getBellkByPK(QString id);
    QSqlQuery getBellkBynameEn(QString nameEn);
    QList<QString> *getAllBellItem() const;

    int getCurrentLocale() const;

    bool mkdir(QString path);
signals:

private:
    QSqlTableModel * m_bellQueryModel = nullptr;
    QMap<QString,int> * idIndexMap = nullptr;
    QMap<int,QString> * indexIdMap = nullptr;
    QString m_defaultBellId = "";
    QList<QString> * allBellItem = nullptr;
};

#endif // SELECTBTNUTIL_H
