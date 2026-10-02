#ifndef REMOTEWIDGET_H
#define REMOTEWIDGET_H

#include <QLineEdit>
#include <QTableView>
#include <QStandardItemModel>
#include <QModelIndexList>
#include "custom_push_button.h"
#include "custom_trans_widget.h"
#include <QAction>

class RemoteWidget : public CustomTransWidget
{
    Q_OBJECT
private:
    CustomPushButton *searchButton;
    QLineEdit *inputIP;
    QTableView *view;
    QStandardItemModel *model;
    QModelIndexList selectList;
    QAction *actionAdd;
    QAction *actionPrint;
    void initUi();
    void initConnect();
    void doWork();
    void ShowContextMenu(const QPoint &pos);
    void triggerMenu(QAction *action);
public:
    explicit RemoteWidget(QWidget *parent = nullptr);
    ~RemoteWidget();
};

#endif // REMOTEWIDGET_H