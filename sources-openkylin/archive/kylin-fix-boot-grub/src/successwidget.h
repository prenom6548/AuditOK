#ifndef SUCCESSWIDGET_H
#define SUCCESSWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QProcess>
#include <QDebug>
#include <QPainterPath>
class successWidget : public QWidget
{
    Q_OBJECT
public:
    successWidget(QWidget *parent = nullptr);
    ~successWidget();

    void initUI();

public:
    QWidget         *m_pIconLabelWidget = nullptr;
    QHBoxLayout     *m_pHboxIconLabelLayout = nullptr;
    QLabel          *m_pSuccessIconLabel = nullptr;

    QWidget         *m_pTextLabelWidget = nullptr;
    QHBoxLayout     *m_pHboxTextLabelWidget = nullptr;
    QLabel          *m_pSuccessTextLabel = nullptr;
    QVBoxLayout     *m_pVboxLayout = nullptr;

    QTimer          *m_pTime = nullptr;
private slots:
    void timerRebootSlots();

};

#endif // SUCCESSWIDGET_H
