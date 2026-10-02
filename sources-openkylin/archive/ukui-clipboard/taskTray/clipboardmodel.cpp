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

QVariant Clipboardmodel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    clipboardData *item = m_items.at(index.row());

    switch (role) {
    case ClipboardDataRole:
        return QVariant::fromValue(m_items.at(index.row()));
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
        qWarning() << "Data not found.";
        return;
    }

    // 准备移除一行，父节点为根节点
    beginRemoveRows(QModelIndex(), rowToRemove, rowToRemove);

    // 执行实际的数据删除逻辑
    m_items.removeAt(rowToRemove);

    // 完成移除操作
    endRemoveRows();
}

void Clipboardmodel::removeAllData()
{
    int num = m_items.size() - 1;

    beginRemoveRows(QModelIndex(), 0, num);

    m_items.clear();

    endRemoveRows();
}

void Clipboardmodel::moveDataToFirst(clipboardData *cData)
{
    int rowToMove = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i]->isSame(cData)) {
            rowToMove = i;
            break;
        }
    }

    if (rowToMove < 0 || rowToMove >= m_items.size()) {
        qWarning() << "Data not found.";
        return;
    } else if (rowToMove == 0) {
        m_items[rowToMove]->setSourceTime(QDateTime::currentDateTime());
        return;
    }

    m_items[rowToMove]->setSourceTime(QDateTime::currentDateTime());

    qDebug()<<beginMoveRows(QModelIndex(),rowToMove,rowToMove,QModelIndex(),0);

    m_items.move(rowToMove,0);

    endMoveRows();
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

    if (j >= 0) {
        return m_items[j];
    }
    return nullptr;
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

int Clipboardmodel::findItem(clipboardData *item)
{
    int rowItem = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i]->isSame(item)) {
            rowItem = i;
            break;
        }
    }
    return rowItem;
}

void Clipboardmodel::updateItemImage()
{
    for (int i = 0; i < m_items.size(); ++i) {
        m_items[i]->updateImage();
    }
}

int Clipboardmodel::getModelCount()
{
    return m_items.count();
}
