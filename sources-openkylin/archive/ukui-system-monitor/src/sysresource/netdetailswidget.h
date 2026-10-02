#ifndef NETDETAILSWIDGET_H
#define NETDETAILSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>

#include "../controls/kchartview.h"
#include "basedetailviewwidget.h"
#include "detaillisttable.h"
#include "netdetailsmodel.h"
#include "chartviewscrollarea.h"

class NetDetailsWidget : public BaseDetailViewWidget
{
    Q_OBJECT
public:
    explicit NetDetailsWidget(QWidget *parent = nullptr);
    ~NetDetailsWidget();

private:
    void initUI();

private:
    ChartViewScrollArea *mNetGraphScrollArea = nullptr;
    QHBoxLayout *mListHLayout = nullptr;

    DetailListTable *mDetailList = nullptr;
    NetDetailsModel *mDetailModel = nullptr;
    QString mCurrentNetName;

public slots:
    void onUpdateNetChartView();
    void onUpdateData();

signals:

};

#endif // NETDETAILSWIDGET_H
