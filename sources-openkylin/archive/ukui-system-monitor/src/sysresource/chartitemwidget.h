#ifndef CHARTITEMWIDGET_H
#define CHARTITEMWIDGET_H

#include <QWidget>
#include <memory>

#include "../src/controls/kchartview.h"

typedef struct NetifInfo
{
    float receiveBytes;
    float sendBytes;
    QString netName;
} NetifInfo;

class ChartItemWidget : public QWidget
{
    Q_OBJECT
public:
    enum ChartViewTypes {
        CPU_CHART,      //处理器
        MEM_CHART,      //内存
        NET_CHART,      //网络
        BLOCK_CHART     //磁盘
    };

    enum TitleStyle {
        TITLE_HORIZONTAL = 0,// 单个模式
        TITLE_VERTICAL = 1   // 多个模式
    };

    explicit ChartItemWidget(ChartViewTypes chartType, QWidget *parent = nullptr, const QString &mac = "");

public:
    void updateActiveStatus(bool active);
    void setMode(int mode);
    bool isActiveItem();
    void setData1Color(const QColor &color); // 添加曲线1颜色
    void setData2Color(const QColor &color); // 添加曲线2颜色
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(const QString &mac);

public slots:
    void fontChanged(const QFont &font);
    void updateData(const NetifInfo &netifInfo);
    void updateData(const QString &deviceName);

    void updateCpuData(const QString &deviceName, qreal value);

    void updateChartDate(qreal sendRate, qreal recvRate);
    void updateChartDate(qreal sendRate);
private:
    void updateWidgetGeometry();

private:
    KChartView *m_pChartView = nullptr;
    ChartViewTypes m_chartType = ChartViewTypes::CPU_CHART;
    int m_mode = TITLE_HORIZONTAL;
    QColor m_data1Color;
    QColor m_data2Color;
    QString m_data1Title = "";
    QString m_data2Title = "";

    QString m_chartName = ""; // 曲线图名称
    QString m_recv_bps; // 接受速率
    QString m_sent_bps; // 发送速率

    QFont m_font;
    bool m_isActive = false;  // 是否被点击

    QList<qreal> m_listData1;
    QList<qreal> m_listData2;
    qreal m_maxData1 = 1;
    qreal m_maxData2 = 1;
    qreal m_maxData;
};

#endif // CHARTITEMWIDGET_H

