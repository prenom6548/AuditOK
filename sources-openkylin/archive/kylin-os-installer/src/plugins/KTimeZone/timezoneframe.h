#ifndef TIMEZONEFRAME_H
#define TIMEZONEFRAME_H

#include <QObject>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "mapwidget.h"
#include "../PluginService/ui_unit/middleframemanager.h"
namespace KInstaller {
class TimeZoneFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit TimeZoneFrame(QWidget *parent = nullptr);
    ~TimeZoneFrame();
    void addStyleSheet();
    void initUI();
    void initAllConnect();
    void translateStr();

    void initComboxZone();

    QString readSettingIni();                   //读写systemsetting.ini文件
    void writeSettingIni();                     //读写systemsetting.ini文件
    void setBoardTabOrder();
    void setNextButtonName();

private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
public:
    QComboBox* m_comboxZone;
    QLabel* m_mainTitle;
    MapWidget* m_mapWid;//显示的世界地图，包括在上面选中某城市后出现的小点
    TimeZoneModel* m_zoneModel;
    TimeZoneStruct m_curzoneItem;
    bool m_firstfocusget=false;//首次焦点获取标志(首次获取时隐藏边框)
signals:
    void signalLanguageSetZone(TimeZoneStruct zoneItem);

public slots:
    void showCurrentZone(TimeZoneStruct zoneItem);
    void clickNextButton();
    void showCurrentZoneIndex(int index);


    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
};
}

#endif // TIMEZONEFRAME_H
