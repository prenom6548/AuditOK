#ifndef UIHISTORYFEEDBACK_H
#define UIHISTORYFEEDBACK_H

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include "paginationwid.h"
#include "feedbackmanager.h"
#include "../constant.h"

class UiHistoryFeedback : public QWidget
{
    Q_OBJECT
public:
    UiHistoryFeedback(QWidget *parent = nullptr);

public slots:
    void indexChanged(QString name);
    void historyInfo(const QList<HistoryInfo> &infolist);

private:
    enum IconType { Loading = 0, Error, Empty } m_iconType = Loading;
    void setIconPix();
    void initTableUI();
    void initErrorUI();
    void setTextToolTips(QTreeWidgetItem *item, int indexCol);
    QWidget *m_tableWidget = nullptr;
    QWidget *m_otherWidget = nullptr;
    QLabel *m_labelText = nullptr;
    QPushButton *m_labelIcon = nullptr;
    QPushButton *m_retryBtn = nullptr;
    QTreeWidget *m_table = nullptr;
    PaginationWid *m_pageWidget = nullptr;
    int m_PageIndex = 1;
    QString m_loadingText;
    QString m_emptyText;
    QString m_errorText;
    QString m_objectName = NAME_HISTORY_FEEDBACK;
    InternalMode m_internalMode = DefaultMode;
};

#endif // UIHISTORYFEEDBACK_H
