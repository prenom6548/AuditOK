#ifndef DISKITEMWIDGET_H
#define DISKITEMWIDGET_H

#include <QObject>
#include <QWidget>

class ChartViewWidget;

class DiskItemWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitleStyle {
        TITLE_HORIZONTAL = 0,// 单个模式
        TITLE_VERTICAL = 1   // 多个模式
    };

    explicit DiskItemWidget(QWidget *parent = nullptr);
    ~DiskItemWidget();

signals:
    void clicked(const QString &deviceName);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

public slots:
    void fontChanged(const QFont &font);
    void activeItemWidget(bool isShow);

public:
    void updateData(const QString &deviceName);
    void setMode(int mode);
    bool isActiveItem();

private:
    void updateWidgetGeometry();

private:
    int m_mode = TITLE_HORIZONTAL;
    QColor m_readColor {"#3790FA"};
    QColor m_writeColor {"#F1852B"};
    ChartViewWidget *m_pDiskChartWidget;

    QFont m_font;
    QString m_deviceName;
    QList<qreal> m_listWriteSpeed;
    QList<qreal> m_listReadSpeed;
    bool m_isActive = false;
};

#endif // DISKITEMWIDGET_H
