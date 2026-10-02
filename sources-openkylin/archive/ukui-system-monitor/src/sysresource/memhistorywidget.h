#ifndef MEMHISTORYWIDGET_H
#define MEMHISTORYWIDGET_H

#include <QWidget>
#include <qgsettings.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>
#include <QProgressBar>

#include "../controls/kchartview.h"
#include <kysdk/applications/klabel.h>
using namespace kdk;

class MemHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MemHistoryWidget(QWidget *parent = nullptr);
    virtual ~MemHistoryWidget();

    QString getMemOccupancy();

    void setClickedFlags(bool clicked);
public slots:
    void onUpdateMemData(qreal memUsed, qreal memTotal);
    void onUpdateSwapData(qreal swapUsed, qreal swapTotal);
signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initStyleTheme();
    void onThemeFontChange(float fFontSize);

private:
    KChartView *m_chartView = nullptr;
    QProgressBar *m_progressBarMem = nullptr;
    QProgressBar *m_progressBarSwap = nullptr;
    KLabel *m_labelMemTitle = nullptr;
    KLabel *m_labelMemValue = nullptr;
    KLabel *m_labelSwapTitle = nullptr;
    KLabel *m_labelSwapValue = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_memTitleLayout = nullptr;
    QHBoxLayout *m_memProgLayout = nullptr;
    QHBoxLayout *m_swapTitleLayout = nullptr;
    QHBoxLayout *m_swapProgLayout = nullptr;
    QHBoxLayout *m_chartLayout = nullptr;

    QGSettings *m_styleSettings = nullptr;
    float m_curFontSize;

    bool m_clicked = false;
    bool m_hover = false;
    qreal transparent = 0.4;
};

#endif // CPUHISTORYWIDGET_H
