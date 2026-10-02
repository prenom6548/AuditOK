#ifndef CHARTVIEWWIDGET_H
#define CHARTVIEWWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QPainterPath>
#include <QPainter>

#include "../controls/kchartview.h"
#include "commoninfo.h"

class ChartViewWidget : public QWidget
{
    Q_OBJECT
public:
    enum ChartViewTypes {
        MEM_CHART,      //内存
        NET_CHART,      //网络
        BLOCK_CHART     //磁盘
    };
    enum TitleStyle {
        TITLE_HORIZONTAL = 0,// 单个模式
        TITLE_VERTICAL = 1   // 多个模式
    };
    explicit ChartViewWidget(ChartViewWidget::ChartViewTypes types, QWidget *parent = nullptr);
    ~ChartViewWidget();

    void setMode(int mode);
    void setActive(bool active);
    QString getNCName();


    void setAxisTitle(const QString &text);

    void setData1Color(const QColor &color);

    void setData2Color(const QColor &color);

private:
    int m_mode = TITLE_HORIZONTAL;
    bool m_isActive = false;
    QFont m_font;
    QString m_ifname;   // 网卡名称
    QString m_recv_bps; // 接受速率
    QString m_sent_bps; // 发送速率
    QColor m_data1Color = {"#B37FEB"};
    QColor m_data2Color = {"#FF7875"};
    QString m_axisTitle = {"100%"};
    KChartView *netChartView = nullptr;

    QList<qreal> m_listData1;
    QList<qreal> m_listData2;
    qreal m_maxData1 = 1;
    qreal m_maxData2 = 1;
    qreal m_maxData;

    ChartViewTypes m_viewType = ChartViewTypes::MEM_CHART;

private:
    void updateWidgetGeometry();
    void drawBackPixmap();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(const QString &mac);

public slots:
    void updateChartDate(qreal sendRate, qreal recvRate);
};

#endif // CHARTVIEWWIDGET_H
