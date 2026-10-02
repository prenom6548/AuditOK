#ifndef OPERATIONLOG_H
#define OPERATIONLOG_H

#include <QStackedWidget>
#include <QTableWidget>
#include "../common/utils.h"
#include "../../common/dynamiccreator.h"
#include "../datetimeutils.h"

class OperationLog : public QStackedWidget
{
    Q_OBJECT
public:
    enum Column_Index {
        Backup_Name = 0,
        UUID,
        Operation,
        Operation_Time,
    };

    DECLARE_DYNCREATE_WITH_ONE_PARAM(OperationLog, QStackedWidget*, QWidget*)

public:
    explicit OperationLog(QWidget* parent = nullptr);
    virtual ~OperationLog();

private:
    void initHomePage();
    void initOperationLogs(const QList<BackupWrapper>& list);
    void setItem(int row, int column, const QString& text, int alignFlag = Qt::AlignLeft | Qt::AlignVCenter);
    QString castTypeToString(int type);
    void updateTimeVolumn();

    QTableWidget *m_tableWidget;
    DateTimeUtils m_dateUtils;
};

#endif // OPERATIONLOG_H
