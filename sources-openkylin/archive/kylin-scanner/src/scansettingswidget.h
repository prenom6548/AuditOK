/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#ifndef SCANSETTINGSWIDGET_H
#define SCANSETTINGSWIDGET_H

#define UKUI_THEME_GSETTING_PATH "org.ukui.style"

#include "include/common.h"
#include "saneobject.h"
#include "globalsignal.h"
#include "sendmail.h"

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QColor>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QSpacerItem>
#include <QSize>
#include <QFontMetrics>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QPdfWriter>
#include <QImage>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include "runningdialog.h"
#include "custom_push_button.h"
#include "waittingdialog.h"
#include "deviceFinder.h"
#include "newdevicelistpage.h"

class ScanSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScanSettingsWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

    /// update scan settings throught sane api
    void updateScanButtonSettings();
    void updateDeviceSettings();
    void updatePageNumberSettings();
    void updateTypeSettings();
    void updateColorSettings();
    void updateResolutionSettings(bool DetectedDevices);
    void updateSizeSettings();
    void updateFormatSettings();
    void updateSaveNameTextSettings();
    void updateSaveDirectorySettings();
    void updateSendMailSettings();
    void updateSaveAsSettings();
    void updateSaveAsTextStore();
    void updateSaveAsTextRecover();

    void updateSettingsForSwitchDevices();
    void updateSettingsStatusForStartScan();
    void updateSettingsStatusForEndScan(int saneStatus);

    int U(const char *str);
    int toUnicode(QString str);
    void setPdfSize(QPdfWriter *pdfWriter, QString size);
    void saveToPdf(QImage img, QString pathname);


    void setLabelAttributes(QLabel *label, const QString &text, int labelHeight = ScanSettingsLabelElideWidth);
    void setSaveButtonAttributes(QPushButton *button, const QString &text, int elideWidth);
    void setSaveButtonLabelAttributes(QLabel *label, const QString &text, int elideWidth);
    void setComboboxAttributes(QComboBox *combobox, QStringList strList);
    void setNameEditTooltip();
    void warnMsg(QString msg);


protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

public slots:
    void rotateChangedSlot(bool isPCMode);
    void updateSettingsForDetectDevices();
    void selectSaveDirectorySlot();

    void deviceCurrentTextChangedSlot(QString text);
    void pageNumberCurrentTextChangedSlot(QString text);
    void timeCurrentTextChangedSlot(QString text);
    void typeCurrentTextChangedSlot(QString text);
    void colorCurrentTextChangedSlot(QString text);
    void resolutionCurrentTextChangedSlot(QString text);
    void sizeCurrentTextChangedSlot(QString text);
    void formatCurrentTextChangedSlot(QString text);
    void nameCurrentTextChangedSlot(QString text);

    void sendMailButtonClickedSlot();
    void saveAsButtonClickedSlot(bool exitApp = false);

    void scanButtonClickedSlot();

    void fontSizeChanged();
    void fontSizeChangedSlot();
    void setDeviceBoxDisableSlot();
    void showWaittingDialogSlot();
private:
    RunningDialog *m_runningDialog = nullptr;
    WaittingDialog *m_waittingDialog = nullptr;

    QString currentSaveDirectory;
    QString currentSaveAsDirectory;

    QPushButton *m_scanButton = nullptr;
    QLabel *scanButtonLeftLabel = nullptr;
    QLabel *scanButtonRightLabel = nullptr;
    QHBoxLayout *scanButtonHLayout = nullptr;

    QHBoxLayout *scanButtonCrapLayout = nullptr;

    QLabel *m_deviceSettingsLabel = nullptr;

    QLabel *m_deviceLabel = nullptr;
    QComboBox *m_deviceComboBox = nullptr;
    kdk::KPushButton *m_deviceAddButton = nullptr;
    QHBoxLayout *m_deviceHLayout = nullptr;
    QWidget *m_deviceWidget = nullptr;

    QLabel *m_pageNumberLabel = nullptr;
    QComboBox *m_pageNumberComboBox = nullptr;

    QLabel *m_typeLabel = nullptr;
    QComboBox *m_typeComboBox = nullptr;

    QLabel *m_colorLabel = nullptr;
    QComboBox *m_colorComboBox = nullptr;

    QLabel *m_resolutionLabel = nullptr;
    QComboBox *m_resolutionComboBox = nullptr;

    QLabel *m_fileSettingsLabel = nullptr;

    QLabel *m_sizeLabel = nullptr;
    QComboBox *m_sizeComboBox = nullptr;

    QLabel *m_formatLabel = nullptr;
    QComboBox *m_formatComboBox = nullptr;

    QLabel *m_saveNameLabel = nullptr;
    QLineEdit *m_saveNameEdit = nullptr;

    QLabel *m_saveDirectoryLabel = nullptr;
    QLabel *m_saveDirectoryButtonLabel = nullptr;
    QHBoxLayout *m_saveDirectoryButtonLayout = nullptr;
    QLineEdit *m_saveDirectoryButton = nullptr;

    QFormLayout *m_settingsFormLayout = nullptr;

    CustomPushButton *m_sendMailButton = nullptr;
    CustomPushButton *m_SaveAsButton = nullptr;
    QHBoxLayout *m_buttonsHLayout = nullptr;

    QVBoxLayout *m_mainVLayout = nullptr;

    QGSettings *m_themeData = nullptr;
    QGSettings *m_data = nullptr;

    SendMailDialog *dialog = nullptr;
    deviceFinder *m_devFinder = nullptr;
    QList<DeviceInformation> m_noDriverDevices;
    newDeviceListPage *m_deviceListPage = nullptr;

    bool isDarkTheme();
    bool showRunningDialog();

public slots:
    void setScanIconDisable();
    void showNewDeviceListPageSuccessSlot();
    void showNewDeviceListPageFailSlot();
    void setAddDevStatusSlot(bool status);
    void updateSettingBtnSlotForInstallDriver();
};

#endif // SCANSETTINGSWIDGET_H
