/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DESKTOPITEMPROXYMODEL_H
#define DESKTOPITEMPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "bw-list-info.h"
#include "peony-json-operation.h"
class DesktopOrganizationManager;
class DesktopWindowManager;
namespace Peony {
class DesktopItemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum SortType {
        FileName,
        ModifiedDate,
        FileType,
        FileSize,
        Other
    };
    enum OriganizationType {
        None = -1,
        OrgByApplycation,
        OrgByDir,
        OrgByFile,
        Others
    };
    explicit DesktopItemProxyModel(QObject *parent = nullptr);
    ~DesktopItemProxyModel();

    Q_INVOKABLE int count() {
        return rowCount();
    }
    Q_INVOKABLE QString uriAt(int row) const;
    Q_INVOKABLE int indexOfUri(const QString &uri) const;
    Q_INVOKABLE QStringList allUris() const;
    Q_INVOKABLE QVariantMap itemDataAt(int row) const;

    void setSortType(int type) {
        m_sort_type = type;
    }
    int getSortType() {
        return m_sort_type;
    }

    void setSortOrder(int sortOrder) {
        m_sort_order = sortOrder;
    }

    int getSortOrder() {
        return m_sort_order;
    }

    void setShowHidden(bool showHidden);
    int updateBlackAndWriteLists();
    QString getBlackAndWhiteModel();
    QSet<QString> getBWListInfo();
    bool getBlackAndWhiteListExist(QString name);

    void updateFilter();

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

    void setId(int newId); // screen id
    Q_INVOKABLE int id() const;
    QModelIndexList getAllFileIndexes();

    void setDesktopUseAutoLayout(bool desktopUseAutoLayout);
    bool getDesktopUseAutoLayout() const;

    QStringList getSortedUris() const;
    void setSortedUris(const QStringList &sortedUris);

    void clearExpectedItemPosForCurrentView();
    void syncBlackAndWhiteData();
    void syncControlDeliverData();
    void setInstanceId(int newInstanceId);
    int instanceId() const;
    int origanizationType() const;
    void setOriganizationType(int type);
    bool checkoriganization(QString uri) const;

Q_SIGNALS:
    void showHiddenFile();
    void requestSyncSortedUrisWithDelay(int msec = 1000);
    void datechange(int i, const QVariant &value, int role);
    void layoutUpdate();
    void showStandardFile();
    void viewUpdated();
    void drop(const QStringList &selectedUris, Qt::DropAction action, int row, const QPoint &offPos);

    void instanceIdChanged();
    void origanizationTypeChanged();
    void idChanged();

public Q_SLOTS:
    Q_INVOKABLE void setdata(int i, const QVariant &value, int role = Qt::EditRole);
    Q_INVOKABLE void updateIconPositions(const QStringList &mimeData, Qt::DropAction action, int row, const QPoint &offPos, bool onItem);
    Q_INVOKABLE void addFile(const QStringList &mimeData, Qt::DropAction action, int row, bool isSearch);
    Q_INVOKABLE void invalidateModel();
    Q_INVOKABLE QVariantMap updateIconPositionsByOrgtool(const QStringList &mimeData, Qt::DropAction action, int row, bool onItem);
    Q_INVOKABLE void updateIconPositionsByOtherView(const QStringList &mimeData/*, Qt::DropAction action, int row, bool onItem*/);
    // 和desktop organization manager交互
    void onRequestSort(int instanceId, int type, int order);
    void onRequestDisbandPartation(int instanceId);
    void onRequestUpdateOrganizationView(int instanceId);

private:
    void handleHiddenPreferenceChange(const QString& key);

    int m_sort_type = Other;
    bool m_show_hidden;
    int m_sort_order = 0;
    BWListInfo    *m_bwListInfo;
    PeonyJsonOperation *m_jsonOp;
    int m_id = 0;
    bool dropSucess = true;
    bool m_desktopUseAutoLayout = false;
    QStringList m_sortedUris;
    int m_instanceId = -1;
    int m_origanizationType = -1;
    Q_PROPERTY(int instanceId READ instanceId WRITE setInstanceId NOTIFY instanceIdChanged FINAL)
    Q_PROPERTY(int screenId READ id WRITE setId NOTIFY idChanged FINAL) //避免和qml的id属性命名冲突
    Q_PROPERTY(int origanizationType READ origanizationType WRITE setOriganizationType NOTIFY origanizationTypeChanged FINAL)

};

}

#endif // DESKTOPITEMPROXYMODEL_H
