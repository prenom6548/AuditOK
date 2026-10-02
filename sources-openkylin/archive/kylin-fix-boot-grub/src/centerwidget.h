#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QDebug>
#include <unistd.h>
#include <QPainterPath>
#include "selectwidgetentry.h"

class centerWidget : public QWidget
{
    Q_OBJECT
public:
    centerWidget(QWidget *parent = nullptr);
    ~centerWidget();
    void initMenberVar();
    void initListWidget();
    void initTitleLable();
    void initButton();
    void initListWidgetEntry();
    void getDiskInfo();

public:
    QVBoxLayout *m_pMainVboxLayout = nullptr;

    QWidget *m_pListContentWidget = nullptr;
    QHBoxLayout *m_pListContentLayout = nullptr;
    QListWidget *m_pListWidget = nullptr;

    QWidget     *m_pTitleWidget = nullptr;
    QHBoxLayout *m_pHboxTitleLayout = nullptr;
    QLabel      *m_pWidgetTitleLabel = nullptr;

    QWidget     *m_pButtonWidget = nullptr;
    QHBoxLayout *m_pHboxButtonLayout = nullptr;
    QPushButton *m_pconfirmButton = nullptr;
    QFrame      *m_pFrameLine = nullptr;

    QStringList m_DiskInfoList;

private:
    QString m_diskNameString = nullptr;

private slots:
    void updateDiskInfoEntry(QWidget *widget);
    void confirmButtonSlots();

signals:
    void execFixBashStatus(bool status);
    void execSwitchWidgetFixing(QString diskName);
};

#endif // CENTERWIDGET_H
