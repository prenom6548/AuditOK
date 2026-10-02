#ifndef CLIPBOARDMODEL_H
#define CLIPBOARDMODEL_H
#include <QAbstractListModel>
#include "clipboarddata.h"
class Clipboardmodel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Mode {
        Text, /**> 文本 */
        Image, /**> 图片 */
        File, /**> 文件 */
        Link /**> 链接 */
    };
    Q_ENUM(Mode)

//    enum RoleNames {
//        IdRole = Qt::UserRole + 1,
//        HasImageRole,
//        ImageUrlRole,
//        SourceTextRole,
//        SourceTimeRole,
//        IsCollectedRole
//    };

    enum RoleNames {
        ClipboardDataRole = Qt::UserRole + 1
    };

    explicit Clipboardmodel(QObject *parent = nullptr);

    // 继承的是抽象类，必须实现这些函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void removeData(clipboardData *cData);
    void moveDataToFirst(clipboardData *cData);
    void removeAllData();

    // 添加数据
    void appendData(clipboardData *item);
    void setMode(Clipboardmodel::Mode mo);
    Clipboardmodel::Mode getMode();
    int findItem(clipboardData *item);
    void updateItemImage();

    Q_INVOKABLE clipboardData* getClipboardData(const QString &id);
    Q_INVOKABLE int getModelCount();
    clipboardData* getFirstData();

    QHash<int, QByteArray> roleNames() const override;

private:
    QList<clipboardData*> m_items;
    Mode mode;
};
#endif // CLIPBOARDMODEL_H
