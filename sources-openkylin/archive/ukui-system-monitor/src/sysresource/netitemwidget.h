#ifndef NETITEMWIDGET_H
#define NETITEMWIDGET_H

#include <QWidget>
#include <memory>

typedef struct NetifInfo
{
    float receiveBytes;
    float sendBytes;
    QString netName;
} NetifInfo;

class ChartViewWidget;
class NetItemWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitleStyle {
        TITLE_HORIZONTAL = 0,// 单个模式
        TITLE_VERTICAL = 1   // 多个模式
    };

    explicit  NetItemWidget(QWidget *parent = nullptr, const QString &mac = "");

public:
    void updateActiveStatus(bool active);
    void setMode(int mode);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(const QString &mac);

public slots:
    void fontChanged(const QFont &font);
    void updateData(const NetifInfo &netifInfo);

private:
    void updateWidgetGeometry();

private:
    int m_mode = TITLE_HORIZONTAL;
    QColor m_recvColor {"#B37FEB"};
    QColor m_sentColor {"#FF7875"};

    QString m_ifname;   // 网卡名称
    QString m_recv_bps; // 接受速率
    QString m_sent_bps; // 发送速率
    ChartViewWidget *m_ChartWidget;

    QFont m_font;
    bool m_isActive = false;  // 是否被点击
};

#endif // NETITEMWIDGET_H

