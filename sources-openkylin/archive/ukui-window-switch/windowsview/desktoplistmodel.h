/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */
#ifndef DESKTOPLISTMODEL_H
#define DESKTOPLISTMODEL_H

#include <QObject>
#include <QList>
#include <QRect>
#include <QVariantList>
#include <QString>
#include <QAbstractListModel>
#include <QMetaType>

class QQuickItem;
class DesktopManager;

class Desktop
{
    Q_GADGET
    Q_PROPERTY(QString uuid READ uuid)

public:
    enum DesktopType {
        WorkSpace = 0,
        NewDesktop
    };

    Desktop() = default;

    QString uuid() const { return m_uuid; }
    void setUuid(const QString uuid) {
        m_uuid = uuid;
    }

    DesktopType type() const { return m_type; };
    void setDesktopType(DesktopType type) {
        m_type = type;
    }

    QString customName() const { return m_customName; }
    void setCustomName(const QString name) {
        m_customName = name;
    }

private:
    QString m_uuid;
    QString m_customName;
    DesktopType m_type = DesktopType::WorkSpace;
};
Q_DECLARE_METATYPE(Desktop)

class DesktopListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString highlightDesktop READ getCurrentDesktop  WRITE setCurrentDesktop NOTIFY currentDesktopChanged)

public:
    enum Role {
        Id = Qt::UserRole + 1, /* desktop index, 从 1 开始计数，易于和界面显示对应 */
        Uuid, /* desktop uuid */
        CustomName, /* 用户自定义名称 */
        Name,
        ItemType /*DesktopType*/
    };
    Q_ENUM(Role)

    explicit DesktopListModel(QObject *parent = nullptr);
    ~DesktopListModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int,QByteArray> roleNames() const override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                              const QModelIndex &destinationParent, int destinationChild) override;

    Q_INVOKABLE void removeDesktop(const QString &desktop);
    Q_INVOKABLE void moveDesktop(const QString &srcDesktop, int destDesktopIndex);
    Q_INVOKABLE void appendDesktop();
    Q_INVOKABLE void setCurrentDesktop(const QString &desktop);
    Q_INVOKABLE QString getCurrentDesktop();
    Q_INVOKABLE QString getCurrentDesktopName();
    Q_INVOKABLE QString getOutputUuid(int outputIndex);
    Q_INVOKABLE bool isCurrentDesktopShowingDesktop();
    Q_INVOKABLE QRect getFullScreenGeometry(const QString &desktop, int screenIndex);
    // 工作区右键菜单
    Q_INVOKABLE void openDesktopMenu(const QString &desktop, QQuickItem *item);

private:
    bool rowIsValid(int row) const;
    void init();
    QStringList getDesktopNames();
    int getModelIndexByUuid(const QString &uuid);

    DesktopManager *m_desktopManager = nullptr;
    QList<Desktop> m_desktopList;
    /* 最大虚拟桌面数量为15: 15个工作区 或 14个工作区+新建工作区*/
    static constexpr int MAX_DESKTOP_COUNT = 15;

signals:
    void desktopAppended(const QString &desktop);
    // model最后一项的属性ItemType由“newDesktop”改为“workSpace”发送此信号
    void desktopNewToAppended(const QString &desktop);
    void desktopRemoved(const QString &desktop);
    void currentDesktopChanged();
    // 工作区重命名信号，激活编辑状态
    void renameDesktopSignal(QQuickItem *item);

private slots:
    void onDesktopAppend(const QString &desktop);
    void onDesktopRemoved(const QString &desktop);
    void onDesktopMoved(const QString &desktop, int destDesktopIndex);
};

#endif // DESKTOPLISTMODEL_H
