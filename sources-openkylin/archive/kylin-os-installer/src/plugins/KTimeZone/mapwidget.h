#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QMap>
#include <QLabel>
#include <QComboBox>
#include <QMenu>
#include "timezonemodel.h"



class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    double changeCoordinatesTOdouble(QString str);
    TimeZoneStruct findNearestPoint(QPoint pos);
    void setZoneList(TimeZoneStructList list){list.clear(); m_TZLists = list;}

    QPoint getLocationPosition( double longitude, double latitude );
private:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);


public:
    QImage m_backgroundImage;//世界地图
    QImage m_pointpng;//悬浮在世界地图中，选中城市上方的蓝色小气球图形
    QList<QImage> m_timeZones;
    int m_curTimeZone;

    TimeZoneStructList m_TZLists;//全世界所有的重要城市列表
    TimeZoneStruct m_zoneItem;//代表当前选中的城市，m_pointpng将悬浮在m_backgroundImage图中此城市的正上方
    QMenu* m_pMenu;
    QString m_sel;

signals:
    void signalCurrentZoneChanged(TimeZoneStruct zoneItem);

public slots:
    void curSelectTimeZone(QAction *ac);
    void showLanguageSetZone(TimeZoneStruct zoneItem);
};

#endif // MAPWIDGET_H
