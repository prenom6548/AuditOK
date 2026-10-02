#ifndef CPUHISTORYWIDGET_H
#define CPUHISTORYWIDGET_H

#include <QWidget>
#include <qgsettings.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>
#include <QProgressBar>

#include "../controls/kchartview.h"
#include <kysdk/applications/klabel.h>
using namespace kdk;

class CpuHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CpuHistoryWidget(QWidget *parent = nullptr);
    virtual ~CpuHistoryWidget();

    QString getCpuOccupancy();

    void setClickedFlags(bool clicked);
public slots:
    void onUpdateData(QVector<double> usage);
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
    QProgressBar *m_progressBar = nullptr;
    KLabel *m_labelTitle = nullptr;
    KLabel *m_labelValue = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QHBoxLayout *m_progLayout = nullptr;
    QHBoxLayout *m_chartLayout = nullptr;

    QGSettings *m_styleSettings = nullptr;
    float m_curFontSize;
    bool m_clicked = false;
    bool m_hover = false;
    qreal transparent = 0.4;
};

#endif // CPUHISTORYWIDGET_H
