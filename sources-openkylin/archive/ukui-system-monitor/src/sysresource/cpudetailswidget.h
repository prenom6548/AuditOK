#ifndef CPUDETAILSWIDGET_H
#define CPUDETAILSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>

#include "../controls/kchartview.h"
#include "basedetailviewwidget.h"
#include "detaillisttable.h"
#include "cpudetailsmodel.h"
#include "chartviewscrollarea.h"

class CpuDetailsWidget : public BaseDetailViewWidget
{
    Q_OBJECT
public:
    CpuDetailsWidget(QWidget *parent = nullptr);
    ~CpuDetailsWidget();

private:
    void initUI();

private:
    ChartViewScrollArea *m_pCpuGraphScrollArea = nullptr;
    QVBoxLayout *mCpuGraphVLayout = nullptr;
    QHBoxLayout *mListHLayout = nullptr;

    KChartView *mCpuChartView = nullptr;

    DetailListTable *mDetailList = nullptr;
    CpuDetailsModel *mDetailModel = nullptr;

public slots:
    void onUpdateCpuChartView(QVector<double> usage);
    void onUpdateData();
signals:

};

#endif // CPUDETAILSWIDGET_H
