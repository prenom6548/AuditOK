#ifndef DISKDETAILSWIDGET_H
#define DISKDETAILSWIDGET_H

#include <QWidget>

#include "basedetailviewwidget.h"
#include "detaillisttable.h"
#include "diskdetailsmodel.h"
#include "chartviewscrollarea.h"

class DiskDetailsWidget : public BaseDetailViewWidget
{
    Q_OBJECT
public:
    explicit DiskDetailsWidget(QWidget *parent = nullptr);

    ~DiskDetailsWidget();

private:
    void initUI();

private:
    ChartViewScrollArea *m_pDiskGraphScrollArea = nullptr;

    DetailListTable *m_pDetailList = nullptr;
    DiskDetailsModel *m_pDetailModel = nullptr;

    QString mCurrentDiskName;

public slots:
    void onUpdateDiskChartView();
    void onUpdateData();

signals:

};

#endif // DISKDETAILSWIDGET_H
