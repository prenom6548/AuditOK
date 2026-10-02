#ifndef INFORMATIONCLASSITEM_H
#define INFORMATIONCLASSITEM_H
#include "feedbackglobal.h"
#include <QDateTime>
class InformationItem;
typedef QList<InformationItem *> InformationItemList;
class InformationClassItem
{
public:
    InformationClassItem(const QString &name, bool select = true);
    ~InformationClassItem();
    QString getItemName() const;
    QString getItemNameShow() const;
    bool isSelect() const;
    void setSelect(bool c);
    int Detailed() const;
    void setDetailed(int d);
    void setItemNameShow(const QString &str);
    InformationItemList *children();

private:
    QString m_itemName;
    QString m_itemNameShow;
    bool m_select;
    int m_detailed = 1;
    InformationItemList *m_children;
};

struct ChooseClassItem
{
    int code;
    QString one;
    QString oneCN;
    QStringList two;
    QStringList twoCN;
    QString guide;
};

struct HistoryInfo
{
    QString id;
    QDateTime creatTime;
    QString type;
    QString message;
    QString solution;
    QString progress;
};

struct ProjectInfo
{
    QString zentaoBuildId;
    QString zentaoBuildName;
    QString productId;
    QString productName;
    QString projectId;
    QString executionId;
    QString executionName;
    QMap<QString, QString> modules;
    QMap<QString, QString> branchs;
};

enum FeedBackInternalVerifyRes { HasData = 0, LoginError, Nodata, ConnectFaild };

enum FeedBackFinishType {
    DefaultFinishType = 0,
    Success,
    Cancel,
    Oversize,
    UploadFail,
    Refuse,
    ExportLogSuccess,    // 用于仅生成日志功能，日志导出成功
};

typedef QList<ChooseClassItem> ChooseClassList;
typedef QList<InformationClassItem *> InformationClassItemList;

#endif
