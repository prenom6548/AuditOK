#ifndef TOUCHSCREENWIDGET_H
#define TOUCHSCREENWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include "common.h"
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/fixlabel.h>
#include "kysdk/applications/klineframe.h"

class FixPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FixPushButton(QWidget *parent = 0);

public:
    void setButtonText(QString text);
    QString getText();

private Q_SLOTS:
    void changedLabelSlot();
private:
    QString m_str;

};

class TouchScreenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchScreenWidget(QWidget *parent = nullptr);

    void resetTouchList(const QList<Device>& devices);
    void resetOutputList(const QList<Device>& devices);
    Device currentTouchDevice() const;
    Device currentOutputDevice() const;

signals:
    void currentOutputChanged(Device device);
    void currentTouchChanged(Device device);
    void mapButtonClicked();
    void calibratButtonClicked();

private:
    void initConnection();
    void initUi();

    void setScreenFrame();
    void setTouchscreenFrame();
    void setDeviceInfoFrame();
    void setSettingFrame();
    void setCalibrationFrame();

    void resetFrameSize();

    QList<Device> m_touchDevices;
    QList<Device> m_outputDevices;

    QVBoxLayout *m_Vlayout;

    TitleLabel* m_titleLabel;

    QFrame* m_hotspotFrame;
    QFrame* m_screenFrame;
    QFrame* m_touchScreenFrame;
    QFrame* m_deviceInfoFrame;
    QFrame* m_settingFrame;
    QFrame* m_calibrationFrame;

    FixLabel* m_monitorLabel;
    FixLabel* m_touchLabel;
    FixLabel* m_deviceLabel;
    QLabel* m_touchNameContent;
    FixLabel* m_settingLabel;
    FixLabel* m_settingWarningLabel;
    FixLabel* m_calibrationLabel;

    QComboBox* m_monitorCombo;
    QComboBox* m_touchscreenCombo;

    FixPushButton* m_mapButton;
    FixPushButton* m_calibrationButton;

    kdk::KHLineFrame* m_screenAndTouchscreenLine;
    kdk::KHLineFrame* m_touchscreenAndDeviceinfoLine;
    kdk::KHLineFrame* m_deviceinfoAndSettingLine;
    kdk::KHLineFrame* m_settingAndCalibrationLine;
};

#endif // TOUCHSCREENWIDGET_H
