#include "clipboardmodel.h"
#include <QVariant>
#include <QDebug>

Clipboardmodel::Clipboardmodel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int Clipboardmodel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

//QVariant Clipboardmodel::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid())
//        return QVariant();

//    clipboardData *item = m_items.at(index.row());

//    switch (role) {
//    case IdRole:
//        return item->id();
//    case HasImageRole:
//        return item->hasImage();
//    case SourceTextRole:
//        return item->sourceText();
//    case ImageUrlRole:
//        return item->imageUrl();
//    case SourceTimeRole:
//        return QVariant::fromValue(item->sourceTime());
//    case IsCollectedRole:
//        return item->isCollected();
//    default:
//        return QVariant();
//    }
//}


QVariant Clipboardmodel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    clipboardData *item = m_items.at(index.row());

    switch (role) {
    case ClipboardDataRole:
        return QVariant::fromValue(m_items.at(index.row()));;
    default:
        return QVariant();
    }
}

void Clipboardmodel::appendData(clipboardData *item)
{
    //if (!item || m_items.contains(item))
    beginInsertRows(QModelIndex(), 0, 0);// 通知视图即将在索引0处插入新行
    m_items.prepend(item);
    endInsertRows();// 通知视图新行已插入
}

//QHash<int, QByteArray> Clipboardmodel::roleNames() const
//{
//    QHash<int, QByteArray> roles;
//    roles[IdRole] = "id";
//    roles[HasImageRole] = "hasImage";
//    roles[SourceTextRole] = "sourceText";
//    roles[ImageUrlRole] = "imageUrl";
//    roles[SourceTimeRole] = "sourceTime";
//    roles[IsCollectedRole] = "isCollected";
//    return roles;
//}

QHash<int, QByteArray> Clipboardmodel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ClipboardDataRole] = "ClipboardData";
    return roles;
}

void Clipboardmodel::removeData(clipboardData *cData)
{
    int rowToRemove = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i] == cData) {
            rowToRemove = i;
            break;
        }
    }

    if (rowToRemove == -1) {
        qDebug() << "Data not found.";
    }

    // 准备移除一行，父节点为根节点
    beginRemoveRows(QModelIndex(), rowToRemove, rowToRemove);

    // 执行实际的数据删除逻辑
    m_items.removeAt(rowToRemove);

    // 完成移除操作
    endRemoveRows();
}

void Clipboardmodel::setMode(Clipboardmodel::Mode mo)
{
    this->mode = mo;
}

Clipboardmodel::Mode Clipboardmodel::getMode()
{
    return this->mode;
}

clipboardData* Clipboardmodel::getClipboardData(const QString &id)
{
    int j = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i]->id() == id) {
            j = i;
            break;
        }
    }

    clipboardData* data;
    if (j >= 0) data = m_items[j];
    return data;
}

clipboardData* Clipboardmodel::getFirstData()
{
    if (m_items.size() > 0)
        return m_items[0];
    else {
        qWarning()<<"m_items 中没有数据";
        return new clipboardData;
    }
}
