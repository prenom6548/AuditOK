/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * 
**/
#include "treemodel.h"
#include <QToolTip>
#include <QDebug>
#include <pwd.h>
#include <unistd.h>

TreeModel::TreeModel(int uid, BioType type, QObject *parent)
    : QAbstractItemModel(parent),
      rootItem(nullptr),
      uid_(uid),
      type_(type)
{
    QString typeText = EnumToString::transferBioType(type_) + tr("Name");
    if(isAdmin(uid))
        rootItem = new TreeItem({tr("index"), tr("username"), typeText});
    else
        rootItem = new TreeItem({tr("index"), typeText});
    serviceInterface = new QDBusInterface(DBUS_SERVICE,
                                          DBUS_PATH,
                                          DBUS_INTERFACE
                                          , QDBusConnection::systemBus());

}

TreeModel::~TreeModel()
{
    if(rootItem) {
        rootItem->cleanChildren();
        delete rootItem;
    }
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

QHash<int, QByteArray> TreeModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[IndexRole] = "index";
    roles[UidRole] = "uid";

    return roles;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
//    if(role != Qt::DisplayRole && role != Qt::EditRole)
//        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole:
        return item->data(index.column());
    case Qt::UserRole:
        return item->getIndex();
    case UidRole:
        return item->getUid();
    case NameRole:
        if(isAdmin(uid_))
            return item->data(2);
        else
            return item->data(1);
    case Qt::SizeHintRole:
        // 返回单元格尺寸
        return QSize(40, 40);
    case Qt::ToolTipRole:
        QString name=index.data(Qt::DisplayRole).toString();
        QPoint point = QCursor::pos();
        //QToolTip::showText(point, name);
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if(0 == index.column())
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    if(1 == index.column())
    {
        return Qt::ItemIsEnabled /*| Qt::ItemIsEditable*/ | Qt::ItemIsSelectable;
    }

    return Qt::NoItemFlags;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;
//    QString text = value.toString();
//    QStringList names = getAllFeaturelist(getuid());
//    if(text.isEmpty()) {
//        QTimer::singleShot(0, this, [&, this](){
//            Q_EMIT setFMessage(tr("Empty feature name"));
//        });
//        return false;
//    }else if(text.length() > 64){
//        QTimer::singleShot(0, this, [&, this](){
//            Q_EMIT setFMessage(tr("feature name is too long"));
//        });
//        return false;
//    } else if(names.contains(text)) {
//        QTimer::singleShot(0, this, [&, this](){
//            Q_EMIT setFMessage(tr("Duplicate feature name"));
//        });
//        return false;
//    } else if(text.startsWith(" ") || text.endsWith(" ")){
//        QTimer::singleShot(0, this, [&, this](){
//            Q_EMIT setFMessage(tr("Please do not use spaces as the beginning or end of the feature name"));
//        });
//        return false;
//    }
//    int column = index.column();
//    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
//    item->setData(column, value);
//    Q_EMIT dataChanged(index, value);
    return true;
}

void TreeModel::setMySort()
{
//    // 关键：发送模型layout即将改变信号
//    emit layoutAboutToBeChanged();

    // 排序
    int provinceCount = rootItem->childCount();
    if (provinceCount > 1)
        rootItem->sort();

//    // 关键：发送模型layout已改变信号
//    emit layoutChanged();
}

void TreeModel::setupTestData()
{
    std::shared_ptr<TreeItem> rootItem = std::make_shared<TreeItem>(QList<QVariant>{"Root", "Root", "Root"});
    std::shared_ptr<TreeItem> user1 = std::make_shared<TreeItem>(QList<QVariant>{"1", "kylin", "左拇指"}, rootItem.get());
    user1->setIndex(1);
    std::shared_ptr<TreeItem> feature12 = std::make_shared<TreeItem>(QList<QVariant>{"1", "kylin", "右拇指"}, user1.get());
    feature12->setIndex(2);
    std::shared_ptr<TreeItem> feature13 = std::make_shared<TreeItem>(QList<QVariant>{"", "", "左食指"}, user1.get());
    feature13->setIndex(3);
    user1->appendChild(feature12.get());
    user1->appendChild(feature13.get());
    std::shared_ptr<TreeItem> user2 = std::make_shared<TreeItem>(QList<QVariant>{"2", "nackee", "中指"}, rootItem.get());
    user2->setIndex(1);
    std::shared_ptr<TreeItem> user3 = std::make_shared<TreeItem>(QList<QVariant>{"3", "yh", "大拇指"}, rootItem.get());
    user3->setIndex(1);
    std::shared_ptr<TreeItem> feature32 = std::make_shared<TreeItem>(QList<QVariant>{"", "", "无名指"}, user3.get());
    feature32->setIndex(2);

    user3->appendChild(feature32.get());
    rootItem->appendChild(user1.get());
    rootItem->appendChild(user2.get());
    rootItem->appendChild(user3.get());
}

QString getUserName(uid_t uid)
{
    struct passwd *pwd = getpwuid(uid);
    return QString(pwd->pw_name);
}

void TreeModel::setModelData(const QList<FeatureInfo *> &featureInfoList)
{
    if(featureInfoList.size() <= 0)
        return;

    rootItem->cleanChildren();
    //TreeItem存放的内容为四列：特征的索引， 显示的序列号， 用户名， 特征名称

    if(isAdmin(uid_)) {
        parentItems.clear();
        for(int i = 0; i < featureInfoList.size(); i++) {
            FeatureInfo *featureInfo = featureInfoList[i];
            if(parentItems.contains(featureInfo->uid)) {
                std::shared_ptr<TreeItem> childItem = std::make_shared<TreeItem>(QList<QVariant>{"",
                                                                                                 "",
                                                                                                 featureInfo->index_name},
                                                                                 parentItems[featureInfo->uid],
                                                                                 featureInfo->uid,
                                                                                 featureInfo->index);
                parentItems[featureInfo->uid]->appendChild(childItem.get());
            } else {
                QString index = QString::number(parentItems.size() + 1);
                QString userName = getUserName(featureInfo->uid);
                std::shared_ptr<TreeItem> parentItem = std::make_shared<TreeItem>(QList<QVariant>{index,
                                                                                                  userName,
                                                                                                  featureInfo->index_name},
                                                                                  rootItem,
                                                                                  featureInfo->uid,
                                                                                  featureInfo->index);
                rootItem->appendChild(parentItem.get());
                parentItems[featureInfo->uid] = parentItem.get();
            }
        }
    } else {
        for(int i = 0; i < featureInfoList.size(); i++) {
            FeatureInfo *featureInfo = featureInfoList[i];
            TreeItem *childItem = new TreeItem({QString::number(i+1),
                                                featureInfo->index_name},
                                               rootItem,
                                               featureInfo->uid,
                                               featureInfo->index);
            rootItem->appendChild(childItem);
        }
    }
}

TreeItem *TreeModel::createItem(int serialNum,
                                const FeatureInfo *featureInfo,
                                int type)
{
    TreeItem *item = nullptr;

    int uid = featureInfo->uid;
    switch(type){
    case NORMAL:
        item = new TreeItem({QString::number(serialNum),
                             featureInfo->index_name},
                            rootItem,
                            uid,
                            featureInfo->index);
//        qDebug()<<"index_name="<<featureInfo->index_name;
//        qDebug()<<"index="<<featureInfo->index;
        break;
    case ADMIN_PARENT:
        item = new TreeItem({QString::number(serialNum),
                             getUserName(uid),
                             featureInfo->index_name},
                            rootItem,
                            uid,
                            featureInfo->index);
        break;;
    case ADMIN_CHILD:
        item =  new TreeItem({"",
                              "",
                              featureInfo->index_name},
                             parentItems[uid],
                             uid,
                             featureInfo->index);
        break;
    }

    return item;
}

void TreeModel::appendData(const FeatureInfo *featureInfo)
{
    if(isAdmin(uid_)) {
        //先判断该用户是否已经存在录入的特征
        int uid = featureInfo->uid;
        QString userName = getUserName(uid);

        if(parentItems.contains(uid)) {

            int row = -1;
            for(int i = 0; i < rowCount(); i++) {
                if(index(i, 1).data().toString() == userName) {
                    row = i;
                }
            }

            if(row >= 0) {  //已经有录入的特征
                QModelIndex parent = index(row, 0);
                int childCount = rowCount(parent);
                beginInsertRows(parent, childCount, childCount);
                TreeItem *childItem = createItem(row, featureInfo, ADMIN_CHILD);
                parentItems[uid]->appendChild(childItem);

                endInsertRows();
            }
        } else {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());

            TreeItem *parentItem = createItem(rowCount() + 1, featureInfo, ADMIN_PARENT);
            rootItem->appendChild(parentItem);
            parentItems[uid] = parentItem;

            endInsertRows();
        }
    } else {
        if(parentItems.isEmpty())
            parentItems[uid_] = rootItem;

        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        TreeItem *childItem = createItem(rowCount()+1,  featureInfo, NORMAL);
        rootItem->appendChild(childItem);

        endInsertRows();
    }
}

void TreeModel::insertData(const FeatureInfo *featureInfo)
{
    if(isAdmin(uid_)){

        int uid = featureInfo->uid;
        QString userName = getUserName(uid);

        if(parentItems.contains(uid)) {

            int row = -1;
            for(int i = 0; i < rowCount(); i++) {
                if(index(i, 1).data().toString() == userName) {
                    row = i;
                }
            }
            TreeItem *parentItem = parentItems[uid];
            QModelIndex parent = index(row, 0);
            if(parentItem->getIndex() > featureInfo->index) {   //成为父节点
                std::shared_ptr<TreeItem> childItem = std::make_shared<TreeItem>(QList<QVariant>{"", "", parentItem->data(2)},
                                                                                 parentItem,
                                                                                 parentItem->getUid(),
                                                                                 parentItem->getIndex());
                parentItem->setData(0, row + 1);
                parentItem->setData(1, userName);
                parentItem->setData(2, featureInfo->index_name);
                parentItem->setIndex(featureInfo->index);
                parentItem->setUid(featureInfo->uid);

                beginInsertRows(parent, 0, 0);
                parentItem->insertChild(0, childItem.get());
                endInsertRows();
            } else {    //成为子节点
                int pos = findInsertPosition(featureInfo, parentItem);
                TreeItem *item = createItem(pos, featureInfo, ADMIN_CHILD);
                beginInsertRows(parent, pos, pos);
                parentItem->insertChild(pos, item);
                endInsertRows();
            }
        } else {
            appendData(featureInfo);
        }
    } else {
        if(parentItems.isEmpty())
            parentItems[uid_] = rootItem;

        int pos = findInsertPosition(featureInfo, rootItem);
        TreeItem *childItem = createItem(pos+1, featureInfo, NORMAL);

        beginInsertRows(QModelIndex(), pos, pos);
        rootItem->insertChild(pos, childItem);

        endInsertRows();

    }
    //updateSerialNum();
}

int TreeModel::findInsertPosition(const FeatureInfo *featureInfo, TreeItem *parentItem)
{
    qDebug() << featureInfo->index_name;
    for(int i = 0; i < parentItem->childCount(); i++) {
        TreeItem *child = parentItem->child(i);
        qDebug() << child->getIndex() << featureInfo->index;
        if(child->getIndex() < featureInfo->index)
            continue;
        else
            return i;
    }
    return parentItem->childCount() + 1;
}


bool TreeModel::removeRow(int row, const QModelIndex &parent, bool recursive)
{
    bool ret;
    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    if(recursive)
        parentItems.remove(parentItem->child(row)->getUid());

    beginRemoveRows(parent, row, row);
    ret = parentItem->removeChild(row, recursive);
    endRemoveRows();

    updateSerialNum();

    return ret;
}

void TreeModel::removeAll()
{
    int count = rowCount(QModelIndex());

    beginRemoveRows(QModelIndex(), 0, count - 1);
    rootItem->cleanChildren();
    endRemoveRows();

    parentItems.clear();
}

void TreeModel::updateSerialNum()
{
    //更新序列号
    for(int i = 0; i < rootItem->childCount(); i++) {
        TreeItem *item = rootItem->child(i);
        item->setData(0, QString::number(i+1));
    }
}

/*!
 * \brief TreeModel::freeIndex
 * \return
 * 查找出一个空闲的索引
 */
int TreeModel::freeIndex()
{
    TreeItem *parentItem;
    QList<int> usedIndexList;
    if(isAdmin(uid_)) {
        if(!parentItems.contains(uid_)) {
            return 1;
        }
        parentItem = parentItems[uid_];
        usedIndexList.append(parentItem->getIndex());
    } else {
        parentItem = rootItem;
    }

    for(int i = 0; i < parentItem->childCount(); i++) {
        usedIndexList.append(parentItem->child(i)->getIndex());
    }
    qSort(usedIndexList);
    qDebug() << "used index: " << usedIndexList;

    int i = 0;
    for(auto index : usedIndexList) {
        if(index - i > 1)
            break;
        i = index;
    }

    return i+1;
}

/**
 * @brief 判断用户是否有指定的特征名的特征
 * @param 用户id
 * @param 待查找的特征名
 * @return
 */
bool TreeModel::hasFeature(int uid, const QString &featureName)
{
    if(!parentItems.contains(uid))
        return false;

    TreeItem *parent = parentItems[uid];
    if(isAdmin(uid)) {
        if(parent->data(2).toString() == featureName)
            return true;
    } 

    for(int i = 0; i < parent->childCount(); i++)
    {
        TreeItem *child = parent->child(i);
        if(isAdmin(uid)) {
            if(child->data(2).toString() == featureName)
                return true;
        } else {
            if(child->data(1).toString() == featureName)
                return true;
        }
    }
    return false;
}
