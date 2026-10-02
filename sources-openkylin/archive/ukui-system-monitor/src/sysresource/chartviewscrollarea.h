#ifndef CHARTVIEWSCROLLAREA_H
#define CHARTVIEWSCROLLAREA_H

#include <QScrollArea>
#include <QMap>
#include <QMenu>

#include "chartitemwidget.h"

class ChartViewScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit ChartViewScrollArea(ChartItemWidget::ChartViewTypes types, QWidget *parent = nullptr);
    void updateWidgetGeometry();
    QString getCurrentChartName();

signals:
    void chartItemClicked(const QString &name);

private:
    void initUI();
    void showItemOne();
    void showItemTwo();
    void showItemLgTwo(int count);

protected:
    void resizeEvent(QResizeEvent *event);

public slots:
    void fontChanged(const QFont &font);
    void onSetItemActiveStatus(const QString &mac);
    void onUpdateNetChartData();
    void onUpdateDiskChartData();
    void onUpdateCpuChartData(QVector<double> usage);

private:
    bool m_initStatus = false;
    QWidget *m_pCentralWidget;
    QString m_currentChartName = "";
    QStringList chartList;
    QStringList preChartList;
    QList<QString> insertionOrder;
    QMultiMap<QString, ChartItemWidget *> m_mapItemView;
    QMenu *m_pMenu = nullptr;
    bool showAllCpuUsage = true;
    ChartItemWidget::ChartViewTypes m_chartType;
};

#endif // CHARTVIEWSCROLLAREA_H
