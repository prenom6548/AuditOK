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
#include "scansettingswidget.h"
#include "include/common.h"

#include <QDebug>

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidgetList>
#include <QApplication>
#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QDateTime>
#include <buriedpoint.hpp>
#include <kysdk/applications/gsettingmonitor.h>
#include <QPalette>
#include "include/theme.h"
#include "thumbnailwidget.h"
#include "newdevicelistpage.h"
#include <libkydatacollect.h>

#define KYLINSCANNER "org.kylin-scanner-data.settings"
inline int realLength(const char* str);
std::string truncateToWidth(const std::string& utf8Str, int maxWidth);

ScanSettingsWidget::ScanSettingsWidget(QWidget *parent) :
    QWidget(parent),
    m_scanButton(new QPushButton()),
    scanButtonLeftLabel(new QLabel(m_scanButton)),
    scanButtonRightLabel(new QLabel(m_scanButton)),
    scanButtonHLayout(new QHBoxLayout(m_scanButton)),
    m_deviceSettingsLabel(new QLabel()),
    m_deviceLabel(new QLabel()),
    m_deviceComboBox(new QComboBox()),
    m_deviceAddButton(new kdk::KPushButton()),
    m_deviceHLayout(new QHBoxLayout()),
    m_deviceWidget(new QWidget()),
    m_pageNumberLabel(new QLabel()),
    m_pageNumberComboBox(new QComboBox()),
    m_typeLabel(new QLabel()),
    m_typeComboBox(new QComboBox()),
    m_colorLabel(new QLabel()),
    m_colorComboBox(new QComboBox()),
    m_resolutionLabel(new QLabel()),
    m_resolutionComboBox(new QComboBox()),
    m_fileSettingsLabel(new QLabel()),
    m_sizeLabel(new QLabel()),
    m_sizeComboBox(new QComboBox()),
    m_formatLabel(new QLabel()),
    m_formatComboBox(new QComboBox()),
    m_saveNameLabel(new QLabel()),
    m_saveNameEdit(new QLineEdit()),
    m_saveDirectoryLabel(new QLabel()),
    m_saveDirectoryButtonLabel(new QLabel()),
    m_saveDirectoryButtonLayout(new QHBoxLayout()),
    m_saveDirectoryButton(new QLineEdit()),
    m_settingsFormLayout(new QFormLayout()),
    m_sendMailButton(new CustomPushButton()),
    m_SaveAsButton(new CustomPushButton()),
    m_buttonsHLayout(new QHBoxLayout()),
    m_mainVLayout(new QVBoxLayout(this)),
    dialog(new SendMailDialog(this))
{
    m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    if(QGSettings::isSchemaInstalled(KYLINSCANNER)){
        m_data = new QGSettings(KYLINSCANNER);
    }

    setupGui();
    initConnect();

}

void ScanSettingsWidget::setScanIconDisable()
{
    m_scanButton->setEnabled(false);
    scanButtonRightLabel->setStyleSheet("color:grey;");
    m_deviceComboBox->setEnabled(false);
    m_pageNumberComboBox->setEnabled(false);
    m_typeComboBox->setEnabled(false);
    m_colorComboBox->setEnabled(false);
    m_resolutionComboBox->setEnabled(false);
    m_sizeComboBox->setEnabled(false);
    m_formatComboBox->setEnabled(false);
}

void ScanSettingsWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0);
    QStyleOption opt;
    opt.init(this);

    QColor mainColor;
    mainColor = opt.palette.color(QPalette::Base);

    p.fillPath(rectPath,QBrush(mainColor));
    p.end();
}

void ScanSettingsWidget::selectSaveDirectorySlot()
{
    if (currentSaveDirectory.isEmpty()) {
        QString scannerPicturePath = g_config_signal->m_scannerImagePath;
        currentSaveDirectory = scannerPicturePath ;
    }

    QString midPath = currentSaveDirectory;
    QString dlgTitle = tr("Select a directory");
    QString selectedDir = QFileDialog::getExistingDirectory(this, dlgTitle, currentSaveDirectory, QFileDialog::ShowDirsOnly);
    qDebug() << "selected directory: " << selectedDir;

    if (!selectedDir.isEmpty()) {
        QFileInfo file(selectedDir);
        if (file.permission(QFileDevice::WriteUser) && selectedDir.contains(QDir::homePath())) {
            qDebug() << "The user could read and write " << selectedDir;

            currentSaveDirectory = selectedDir;
            setSaveButtonLabelAttributes(m_saveDirectoryButtonLabel, currentSaveDirectory, ScanSettingsButtonElideWidth);
        } else {
            qDebug() << "The user can't read and write " << selectedDir;

            QString msg = tr("Currently user has no permission to modify directory ") + selectedDir;
            warnMsg(msg);
        }
    }
    g_sane_object->userInfo.saveDirectory = currentSaveDirectory;

    qDebug() << "saveDirectory = " << currentSaveAsDirectory;
}

void ScanSettingsWidget::deviceCurrentTextChangedSlot(QString text)
{
    if(text == ""){
        return;
    }
    qDebug() << "Sane device name: " << g_sane_object->userInfo.name << "current deviece name: " << m_deviceComboBox->currentText();

    if (g_sane_object->userInfo.name.isEmpty()) {
        g_sane_object->userInfo.name = text;

        qDebug() << "We do not need open device again.";
        return;
    } else {
        qDebug() << "Device changed, therefore, we need open device again.";
        g_sane_object->userInfo.name = text;
    }

    int curTextLen = m_deviceComboBox->currentText().length();

    if (curTextLen >= 20){
        m_deviceComboBox->setToolTip(m_deviceComboBox->currentText());
    } else {
        m_deviceComboBox->setToolTip("");
    }

    // index is chosen by user manually, or default(0)
    int index = m_deviceComboBox->currentIndex();
    if (index == -1) {
        index = 0;
    }

    g_sane_object->userInfo.deviceNameIndex = index;

    if(g_sane_object->getSaneHaveHandle()){
        g_sane_object->saneClose();
    }

    // while switch scan device, we should open the scan device to get some parameters
    g_sane_object->openDeviceIndex = index;
    g_sane_object->openSaneDeviceForPage(index);

    updateSettingsForSwitchDevices();
}

/**
 * @brief ScanSettingsWidget::pageNumberCurrentTextChangedSlot
 * @param text
 *
 */
void ScanSettingsWidget::pageNumberCurrentTextChangedSlot(QString text)
{
    g_sane_object->m_ParametersHaveSeted = false;
    if (text.compare(QApplication::tr("Multiple"), Qt::CaseInsensitive) == 0 || text.compare("Multiple", Qt::CaseInsensitive) == 0) {
        if(m_typeComboBox->currentText().compare(QApplication::tr("Flatbed"), Qt::CaseInsensitive) == 0 || m_typeComboBox->currentText().compare("Flatbed", Qt::CaseInsensitive) == 0){
            QString msg = tr("Flatbed scan mode not support multiple scan.");
            g_user_signal->warnMsg(msg);
            m_pageNumberComboBox->setCurrentText(tr("Single"));
            return;
        }
        g_sane_object->userInfo.pageNumber = "Multiple";

        // Avoid SANE_STATUS_NO_DOC Error to set time not enable.
        g_sane_object->setSaneStatus(true);

        QMap<QString, QString> buried_data;
        buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerMultiPageScan)));
        buried_data.insert("action", "multipage scan");
        buried_data.insert("function", "in scansettingswidget.cpp function pageNumberCurrentTextChangedSlot()");
        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "multi_page_scan", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);
    } else {
        g_sane_object->userInfo.pageNumber = "Single";

        QMap<QString, QString> buried_data;
        buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerSinglePageScan)));
        buried_data.insert("action", "SinglePage scan");
        buried_data.insert("function", "in scansettingswidget.cpp function pageNumberCurrentTextChangedSlot()");

        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "single_page_scan", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);
    }
}

void ScanSettingsWidget::timeCurrentTextChangedSlot(QString text)
{
    g_sane_object->userInfo.time = text;

    qDebug() << "userInfo.time = " << text;
}

void ScanSettingsWidget::typeCurrentTextChangedSlot(QString text)
{
    if(QString::compare(text, tr("Flatbed"), Qt::CaseInsensitive) == 0){
        m_pageNumberComboBox->setCurrentText(tr("Single"));
    }

    g_sane_object->m_ParametersHaveSeted = false;
    g_sane_object->userInfo.type = text;
    qDebug() << "userInfo.type = " << text;
}

void ScanSettingsWidget::colorCurrentTextChangedSlot(QString text)
{
    g_sane_object->m_ParametersHaveSeted = false;
    if (0 == QString::compare(text, tr("Color"), Qt::CaseInsensitive)) {
        g_sane_object->userInfo.color = "Color";
    } else if(0 == QString::compare(text, tr("Lineart"), Qt::CaseInsensitive)) {
        g_sane_object->userInfo.color = "Lineart";
    } else {
        g_sane_object->userInfo.color = "Gray";
    }

    qDebug() << "userInfo.color = " << text;
}

void ScanSettingsWidget::resolutionCurrentTextChangedSlot(QString text)
{
    g_sane_object->m_ParametersHaveSeted = false;
    if ((0 == text.compare(tr("4800 dpi"), Qt::CaseInsensitive))
            || (0 == text.compare(tr("2400 dpi"), Qt::CaseInsensitive))
            || (0 == text.compare(tr("1200 dpi"), Qt::CaseInsensitive))) {

        QString msg = tr("This resolution will take a long time to scan, please choose carelly.");
        QMessageBox::information(this, tr("Alert"), msg);
//        warnMsg(msg);
    }

    g_sane_object->userInfo.resolution = text;

    qDebug() << "userInfo.resolution = " << text;
}

void ScanSettingsWidget::sizeCurrentTextChangedSlot(QString text)
{
    g_sane_object->m_ParametersHaveSeted = false;
    g_sane_object->userInfo.size = text;
    qDebug() << "userInfo.size = " << text;
}

void ScanSettingsWidget::formatCurrentTextChangedSlot(QString text)
{
    g_sane_object->m_ParametersHaveSeted = false;
    g_sane_object->userInfo.format = text;
    qDebug() << "userInfo.format = " << text;
}

void ScanSettingsWidget::nameCurrentTextChangedSlot(QString text)
{
    QString msg;
    if (m_saveNameEdit->text().contains(QChar('/'), Qt::CaseInsensitive)) {
        msg = tr("cannot contain '/' character.");
        warnMsg(msg);     
        m_saveNameEdit->setText(m_saveNameEdit->text().remove("/"));
    }
    if (m_saveNameEdit->text().startsWith(QChar('.'), Qt::CaseInsensitive)) {
        msg = tr("cannot save as hidden file.");
        warnMsg(msg);
        m_saveNameEdit->cursorBackward(true, 1);
        m_saveNameEdit->del();
    }

    QString saveNameEditStr = m_saveNameEdit->text();
    long length = realLength(saveNameEditStr.toUtf8().data()); // 当前内容长度
    if (length > 120) {
        saveNameEditStr = QString::fromStdString(truncateToWidth(saveNameEditStr.toUtf8().data(), 120));
        m_saveNameEdit->setText(saveNameEditStr);
    }

    if(g_sane_object->userInfo.loadName == ""){
        g_sane_object->userInfo.loadName = m_saveNameEdit->text();
    }

    g_sane_object->userInfo.saveName = m_saveNameEdit->text();
    qDebug() << "saveName = " << g_sane_object->userInfo.saveName;
    qDebug() << "loadName = " << g_sane_object->userInfo.loadName;

    setNameEditTooltip();

}

void ScanSettingsWidget::sendMailButtonClickedSlot()
{  
    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerSendMail)));
    buried_data.insert("action", "Send Mail");
    buried_data.insert("function", "in scansettingswidget.cpp function sendMailButtonClickedSlot()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "send_mail", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);

    int retDialog;

    AppList *maillist = getAppIdList(MailType);

    if (! maillist) {

        QWidget *widget = nullptr;
        QWidgetList widgetList = QApplication::allWidgets();
        for (int i=0; i<widgetList.length(); ++i) {
            if (widgetList.at(i)->objectName() == "MainWindow") {
                widget = widgetList.at(i);
            }
        }

        NoMailDialog *dialog = new NoMailDialog(widget);
        dialog->setAttribute(Qt::WA_DeleteOnClose);


        if (widget) {
            QRect rect = widget->geometry();
            int x = rect.x() + rect.width()/2 - dialog->width()/2;
            int y = rect.y() + rect.height()/2 - dialog->height()/2;
            dialog->move(x,y);
        }


    } else {
        qDebug() << "Get mail client list success.";

        dialog->setMailSelectComboboxItems();

        dialog->exec();
    }

    g_user_signal->sendMailButtonClicked();
}

void ScanSettingsWidget::saveAsButtonClickedSlot(bool exitApp)
{
    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerSaveAs)));
    buried_data.insert("action", "save in user designed path");
    buried_data.insert("function", "in scansettingswidget.cpp function saveAsButtonClickedSlot()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "scanner_save_as", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);

    QString filename = m_saveNameEdit->text();
    QString fileFormat = m_formatComboBox->currentText();
    QString filepath = currentSaveDirectory;
    //构造文件类型的filter
    QString filter;
    if (g_sane_object->ocrFlag == 1) {
        filter = QLatin1String("*.txt");
    } else {
        if (fileFormat == "jpg") {
            filter = QLatin1String("*.jpg;;*.png;;*.pdf;;*.bmp;;*.tiff;;*.ofd");
        } else if (fileFormat == "png") {
            filter = QLatin1String("*.png;;*.jpg;;*.pdf;;*.bmp;;*.tiff;;*.ofd");
        } else if (fileFormat == "pdf") {
            filter = QLatin1String("*.pdf;;*.jpg;;*.png;;*.bmp;;*.tiff;;*.ofd");
        } else if (fileFormat == "bmp"){
            filter = QLatin1String("*.bmp;;*.jpg;;*.png;;*.pdf;;*.tiff;;*.ofd");
        } else if (fileFormat == "tiff"){
            filter = QLatin1String("*.tiff;;*.bmp;;*.jpg;;*.png;;*.pdf;;*.ofd");
        } else{
            filter = QLatin1String("*.ofd;;*.jpg;;*.png;;*.pdf;;*.bmp;;*.tiff");
        }
    }
    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QString fileType;
    //构造打开路径
    QString openFileName;
    if(filepath.endsWith("/")){
        openFileName = filepath + filename;
    }else{
        openFileName = filepath + "/" + filename;
    }
    QString path = fileDialog.getSaveFileName(this, tr("Save As"), openFileName, filter, &fileType);
    fileType = fileType.right(fileType.length() - 1);
    int pos = path.lastIndexOf("/");
    int length = path.length();
    //文件名的安全判断
    QString path1 = path;
    QString fileNameOp = path1.right(length - pos - 1);
    if (fileNameOp.isNull()){
        return;
    }
    if (fileNameOp.startsWith(QChar('.'), Qt::CaseInsensitive)) {
        QString msg = tr("cannot save as hidden file.");
        warnMsg(msg);
        return;
    }
    //路径名的安全判断
    QString path2 = path;
    QString pathNameOp = path2.left(pos);
    QDir dirVertify(pathNameOp);
    QString str = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if(dirVertify.exists()){
        if(dirVertify.path().contains(str,Qt::CaseSensitive)){
            qDebug() << "path is ok.";
        }else{
            warnMsg(tr("Path without access rights: ") + path);
        }

    }else{
        warnMsg(tr("File path that does not exist: ") + path);
    }
    //完整路径名补充
    if (g_sane_object->ocrFlag == 1) {
        if (!path.endsWith(".txt", Qt::CaseInsensitive)) {
            path = path.append(fileType);
        }
    } else {
        if (!path.endsWith(".jpg", Qt::CaseInsensitive)
                && !path.endsWith(".png", Qt::CaseInsensitive)
                && !path.endsWith(".pdf", Qt::CaseInsensitive)
                && !path.endsWith(".bmp", Qt::CaseInsensitive)
                && !path.endsWith(".tiff", Qt::CaseInsensitive)
                && !path.endsWith(".ofd", Qt::CaseInsensitive)) {
            path = path.append(fileType);
        }
    }
    g_user_signal->saveAsButtonClicked(path);
    g_user_signal->exitWindowWithSaveFlag = false;
    if(exitApp){
        g_user_signal->exitApplication();
    }
}

bool ScanSettingsWidget::showRunningDialog()
{
    QString fileName = m_saveNameEdit->text();
    QString path = currentSaveDirectory;
    QString format = m_formatComboBox->currentText();
    QString filePath;
    if(g_sane_object->userInfo.pageNumber.compare(QApplication::tr("Multiple"), Qt::CaseInsensitive) == 0 || g_sane_object->userInfo.pageNumber.compare("Multiple", Qt::CaseInsensitive) == 0  || g_sane_object->userInfo.type.compare(QApplication::tr("ADF Duplex"), Qt::CaseInsensitive) == 0 || g_sane_object->userInfo.type.compare("ADF Duplex", Qt::CaseInsensitive) == 0){
        if(format != "tiff" && format != "pdf"){
            filePath = path + "/" + fileName + "[1]" + "." + format;
        }else{
            filePath = path + "/" + fileName + "." + format;
        }
    }else{
        if(format != "tiff" && format != "pdf"){
            filePath = path + "/" + fileName + "." + format;
        }else{
            filePath = path + "/" + fileName + "." + format;
        }
    }
    QFileInfo file(filePath);
    if(file.exists()){
        QMessageBox::StandardButton box;
        QString tipsStr1 = tr("The file ");
        QString tipsStr2 = tr(" already exists, do you want to overwrite it? If you are performing a multi page scan, it may cause multiple files to be overwritten. Please be cautious!");
        QString tipsStr = tipsStr1 + file.fileName()  + tipsStr2;
        box = QMessageBox::question(this->parentWidget(),tr("tips"),tipsStr,QMessageBox::Ok|QMessageBox::Cancel);
        if(box == QMessageBox::Ok){
            QFile(filePath).remove();
            if(g_sane_object->userInfo.pageNumber.compare(QApplication::tr("Multiple"), Qt::CaseInsensitive) == 0 || g_sane_object->userInfo.pageNumber.compare("Multiple", Qt::CaseInsensitive) == 0 || g_sane_object->userInfo.type.compare(QApplication::tr("ADF Duplex"), Qt::CaseInsensitive) == 0 || g_sane_object->userInfo.type.compare("ADF Duplex", Qt::CaseInsensitive) == 0){
                // 删tmp下的临时文件
                QString tmpdir = "/tmp/kylin-scanner/images/";
                int i = 1;
                QString rmdir = tmpdir + fileName + "[" + QString::number(i) + "].pnm";
                QString rmdir2 = tmpdir + fileName + "[" + QString::number(i) + "].jpg";

                do{
                    QFile(rmdir).remove();
                    QFile(rmdir2).remove();
                    i++;
                    rmdir = tmpdir + fileName + "[" + QString::number(i) + "].pnm";
                    rmdir2 = tmpdir + fileName + "[" + QString::number(i) + "].jpg";
                }while(QFileInfo(rmdir).exists());
            }else{
                QString tmpdir = "/tmp/kylin-scanner/images/";
                QString rmdir = tmpdir + file.completeBaseName() + ".pnm";
                QString rmdir2 = tmpdir + file.completeBaseName() + ".jpg";

                if(QFile(rmdir).exists()){
                    QFile(rmdir).remove();
                }
                if(QFile(rmdir2).exists()){
                    QFile(rmdir2).remove();
                }
            }
            return true;
        }else{
            return false;
        }
    }
    return true;
}
void ScanSettingsWidget::scanButtonClickedSlot()
{
    bool res = showRunningDialog();
    if(res){
        qDebug()<<"scan a new file!";
    }else{
        return;
    }
    updateSettingsStatusForStartScan();

    g_user_signal->startScanOperation();
}

void ScanSettingsWidget::fontSizeChanged()
{
    setLabelAttributes(m_deviceLabel, tr("Device"));
    setLabelAttributes(m_pageNumberLabel, tr("Pages"));
    setLabelAttributes(m_typeLabel, tr("Type"));
    setLabelAttributes(m_colorLabel, tr("Colour"));
    setLabelAttributes(m_resolutionLabel, tr("Resolution"));
    setLabelAttributes(m_sizeLabel, tr("Size"));
    setLabelAttributes(m_formatLabel, tr("Format"));
    setLabelAttributes(m_saveNameLabel, tr("Name"));
    setLabelAttributes(m_saveDirectoryLabel, tr("Save"));

    setSaveButtonLabelAttributes(m_saveDirectoryButtonLabel, currentSaveDirectory, ScanSettingsButtonElideWidth);

    m_deviceAddButton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_deviceAddButton->setIconSize(QSize(24, 24));

    m_sendMailButton->adjustSize();
    m_sendMailButton->setText(tr("Mail to"));
    m_sendMailButton->setToolTip(tr("Mail to"));

    m_SaveAsButton->adjustSize();
    m_SaveAsButton->setText(currentSaveAsDirectory);
    m_SaveAsButton->setToolTip(currentSaveAsDirectory);

}

void ScanSettingsWidget::fontSizeChangedSlot()
{
    float systemFontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QString fontType = m_themeData->get("systemFont").toString();
    QFont font(fontType, systemFontSize);
    scanButtonRightLabel->setFont(font);
    fontSizeChanged();
}

void ScanSettingsWidget::setDeviceBoxDisableSlot()
{
    m_deviceComboBox->setEnabled(false);
}

void ScanSettingsWidget::showWaittingDialogSlot()
{
    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    m_waittingDialog = new WaittingDialog(widget);
    m_waittingDialog->show();


    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerFindNoDriverDevice)));
    buried_data.insert("action", "Show waitting dialog and make deviceFinder start work.");
    buried_data.insert("function", "in scansettingswidget.cpp function ShowWaittingDialogSlot()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "find_no_driver_device", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);

    m_devFinder = new deviceFinder();
    QObject::connect(m_devFinder, &deviceFinder::succeed, this, &ScanSettingsWidget::showNewDeviceListPageSuccessSlot);
    QObject::connect(m_devFinder, &deviceFinder::failed, this, &ScanSettingsWidget::showNewDeviceListPageFailSlot);
    m_devFinder->startWorker();
}

void ScanSettingsWidget::showNewDeviceListPageSuccessSlot()
{
    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }

    qDebug() << "Install succeed";
    m_waittingDialog->close();
    // 初始化页面并，解析内容并展示
    m_noDriverDevices.clear();
    m_noDriverDevices = m_devFinder->getList();
    m_deviceListPage = new newDeviceListPage(m_noDriverDevices, widget);
    m_devFinder->finished();
}

void ScanSettingsWidget::showNewDeviceListPageFailSlot()
{
    qDebug() << "Install failed";
}

void ScanSettingsWidget::setAddDevStatusSlot(bool status)
{
    m_deviceAddButton->setEnabled(status);
}

void ScanSettingsWidget::updateSettingBtnSlotForInstallDriver()
{
    m_scanButton->setEnabled(false);
    m_deviceComboBox->setEnabled(false);
    m_pageNumberComboBox->setEnabled(false);
    m_typeComboBox->setEnabled(false);
    m_colorComboBox->setEnabled(false);
    m_resolutionComboBox->setEnabled(false);
    m_sizeComboBox->setEnabled(false);
    m_formatComboBox->setEnabled(false);
    m_saveNameEdit->setEnabled(false);
    m_saveDirectoryButtonLabel->setEnabled(false);
    if(ThumbnailWidget::scanPictureIsEmpty){
        m_sendMailButton->setEnabled(false);
        m_SaveAsButton->setEnabled(false);
    }else{
        m_sendMailButton->setEnabled(true);
        m_SaveAsButton->setEnabled(true);
    }
}

bool ScanSettingsWidget::isDarkTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    if (systemTheme == STYLE_NAME_KEY_DARK || systemTheme == STYLE_NAME_KEY_BLACK) {
        return true;
    } else {
        return false;
    }
}
void ScanSettingsWidget::setupGui()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setFixedWidth(ScanSettingsWidgetWidth);

    scanButtonLeftLabel->setPixmap(QPixmap(":/default-connect-page/begin-scan.svg"));
    scanButtonRightLabel->setText(tr("Start Scan"));
    scanButtonRightLabel->setStyleSheet("color:white;");

    scanButtonHLayout->addStretch();
    scanButtonHLayout->addWidget(scanButtonLeftLabel);
    scanButtonHLayout->addSpacing(4);
    scanButtonHLayout->addWidget(scanButtonRightLabel);
    scanButtonHLayout->addStretch();
    scanButtonHLayout->setSpacing(4);
    scanButtonHLayout->setContentsMargins(4, 0, 0, 0);
    m_scanButton->setLayout(scanButtonHLayout);
    m_scanButton->setCursor(Qt::PointingHandCursor);
    m_scanButton->setProperty("isImportant", true);
    scanButtonCrapLayout = new QHBoxLayout(this);
    scanButtonCrapLayout->addSpacing(24);
    scanButtonCrapLayout->addWidget(m_scanButton);
    scanButtonCrapLayout->addSpacing(24);


    m_deviceSettingsLabel->setText(tr("Scanner device"));
    QPalette palFont;
    palFont.setColor(QPalette::Text,QColor("#8C8C8C"));
    m_deviceSettingsLabel->setPalette(palFont);

    m_fileSettingsLabel->setText(tr("File settings"));
    m_fileSettingsLabel->setPalette(palFont);

    m_saveNameEdit->setText(tr("scanner01"));

    m_saveDirectoryLabel->setFocusPolicy(Qt::TabFocus);

    m_saveDirectoryButtonLayout->addWidget(m_saveDirectoryButtonLabel);
    m_saveDirectoryButtonLayout->setSpacing(0);
    m_saveDirectoryButtonLayout->setContentsMargins(8, 0, 0, 0);
    m_saveDirectoryButton->setLayout(m_saveDirectoryButtonLayout);
    m_saveDirectoryButton->setReadOnly(true);

    if (currentSaveDirectory.isEmpty()) {
        QString scannerPicturePath = g_config_signal->m_scannerImagePath;
        currentSaveDirectory = scannerPicturePath ;

        g_sane_object->userInfo.saveDirectory = currentSaveDirectory;

        qDebug() << "saveDirectory = " << currentSaveDirectory;
    }

    currentSaveAsDirectory = tr("Save as");
    fontSizeChanged();
    m_deviceAddButton->setFixedSize(36, 36);
    m_deviceHLayout->setSpacing(0);
    m_deviceHLayout->addWidget(m_deviceComboBox);
    m_deviceHLayout->addSpacing(8);
    m_deviceHLayout->addWidget(m_deviceAddButton);
    m_deviceHLayout->setContentsMargins(0, 0, 0, 0);
    m_deviceWidget->setLayout(m_deviceHLayout);


    m_settingsFormLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    m_settingsFormLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    m_settingsFormLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_settingsFormLayout->setLabelAlignment(Qt::AlignLeft);
    m_settingsFormLayout->setHorizontalSpacing(7);
    m_settingsFormLayout->setVerticalSpacing(8);
    m_settingsFormLayout->addRow(m_deviceSettingsLabel);
    m_settingsFormLayout->addRow(m_deviceLabel, m_deviceWidget);
    m_settingsFormLayout->addRow(m_pageNumberLabel, m_pageNumberComboBox);
    m_settingsFormLayout->addRow(m_typeLabel, m_typeComboBox);
    m_settingsFormLayout->addRow(m_colorLabel, m_colorComboBox);
    m_settingsFormLayout->addRow(m_resolutionLabel, m_resolutionComboBox);
    m_settingsFormLayout->addRow(m_fileSettingsLabel);
    m_settingsFormLayout->addRow(m_sizeLabel, m_sizeComboBox);
    m_settingsFormLayout->addRow(m_formatLabel, m_formatComboBox);
    m_settingsFormLayout->addRow(m_saveNameLabel, m_saveNameEdit);
    m_settingsFormLayout->addRow(m_saveDirectoryLabel, m_saveDirectoryButton);
    m_settingsFormLayout->setContentsMargins(24, 0, 24, 0);
    m_settingsFormLayout->setObjectName("m_settingsFormLayout");

    m_pageNumberComboBox->setObjectName("m_pageNumberComboBox");
    m_typeComboBox->setObjectName("m_typeComboBox");
    m_colorComboBox->setObjectName("m_colorComboBox");
    m_resolutionComboBox->setObjectName("m_resolutionComboBox");
    m_sizeComboBox->setObjectName("m_sizeComboBox");
    m_formatComboBox->setObjectName("m_formatComboBox");
    m_saveNameEdit->setObjectName("m_saveNameEdit");
    m_saveDirectoryButton->setObjectName("m_saveDirectoryButton");

    m_sendMailButton->setMinimumSize(ScanSettingsWidgetButtonSize);
    m_SaveAsButton->setMinimumSize(ScanSettingsWidgetButtonSize);

    m_buttonsHLayout->setSpacing(0);
    m_buttonsHLayout->addWidget(m_sendMailButton);
    m_buttonsHLayout->addSpacing(12);
    m_buttonsHLayout->addWidget(m_SaveAsButton);
    m_buttonsHLayout->setContentsMargins(24, 0, 24, 0);

    m_mainVLayout->setSpacing(0);
    m_mainVLayout->addSpacing(16);
    m_mainVLayout->addLayout(scanButtonCrapLayout);
    m_mainVLayout->addSpacing(16);
    m_mainVLayout->addLayout(m_settingsFormLayout);
    m_mainVLayout->addSpacing(16);
    m_mainVLayout->addLayout(m_buttonsHLayout);
    m_mainVLayout->addSpacing(24);
    m_mainVLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVLayout->setObjectName("m_mainVLayout");

    setLayout(m_mainVLayout);
    bool isPCMode = GlobalUserSignal::getInstance()->getCurrentMode();
    rotateChangedSlot(isPCMode);
}

void ScanSettingsWidget::initConnect()
{
    connect(m_scanButton, &QPushButton::clicked, this, &ScanSettingsWidget::scanButtonClickedSlot);
    connect(GlobalUserSignal::getInstance(), &GlobalUserSignal::scanStart, m_scanButton, &QPushButton::click);
    connect(m_scanButton, &QPushButton::clicked, g_user_signal, &GlobalUserSignal::exitOcrWhenScanSignal);
    connect(m_deviceComboBox, &QComboBox::currentTextChanged, this, &ScanSettingsWidget::deviceCurrentTextChangedSlot);
    connect(m_deviceAddButton, &kdk::KPushButton::clicked, this, &ScanSettingsWidget::showWaittingDialogSlot);
    connect(m_pageNumberComboBox, &QComboBox::currentTextChanged, this, &ScanSettingsWidget::pageNumberCurrentTextChangedSlot);
    connect(m_typeComboBox, &QComboBox::currentTextChanged, this, &ScanSettingsWidget::typeCurrentTextChangedSlot);
    connect(m_colorComboBox, &QComboBox::currentTextChanged, this, &ScanSettingsWidget::colorCurrentTextChangedSlot);
    connect(m_resolutionComboBox, &QComboBox::currentTextChanged, this, &ScanSettingsWidget::resolutionCurrentTextChangedSlot);
    connect(m_sizeComboBox, &QComboBox::currentTextChanged, this, &ScanSettingsWidget::sizeCurrentTextChangedSlot);
    connect(m_formatComboBox, &QComboBox::currentTextChanged, this, &ScanSettingsWidget::formatCurrentTextChangedSlot);
    connect(m_saveNameEdit, &QLineEdit::textChanged, this, &ScanSettingsWidget::nameCurrentTextChangedSlot);
    connect(m_saveDirectoryButton, &QLineEdit::selectionChanged, this, &ScanSettingsWidget::selectSaveDirectorySlot);

    connect(m_sendMailButton, &QPushButton::clicked, this, &ScanSettingsWidget::sendMailButtonClickedSlot);
    connect(m_SaveAsButton, &QPushButton::clicked, this, &ScanSettingsWidget::saveAsButtonClickedSlot);
    connect(g_user_signal, &GlobalUserSignal::closeWindowSaveAsSignal, this, &ScanSettingsWidget::saveAsButtonClickedSlot);

    connect(m_themeData, &QGSettings::changed, this, &ScanSettingsWidget::fontSizeChangedSlot);
    connect(g_user_signal, &GlobalUserSignal::scanThreadFinishedSignal, this, &ScanSettingsWidget::updateSettingsStatusForEndScan);
    connect(g_user_signal, &GlobalUserSignal::stopOrFinishedScanSignal, this, &ScanSettingsWidget::updateSaveNameTextSettings);

    connect(g_user_signal, &GlobalUserSignal::toolbarOcrOperationFinishedSignal, this, &ScanSettingsWidget::updateSaveAsTextStore);
    connect(m_scanButton, &QPushButton::clicked, this, &ScanSettingsWidget::updateSaveAsTextRecover);
    connect(g_user_signal, &GlobalUserSignal::showImageAfterClickedThumbnailSignal, this, &ScanSettingsWidget::updateSaveAsTextRecover);
    connect(g_user_signal, &GlobalUserSignal::exitOCR, this, &ScanSettingsWidget::updateSaveAsTextRecover);
    connect(g_user_signal, &GlobalUserSignal::updateSettingSignal, this, &ScanSettingsWidget::updateSettingsForDetectDevices);

    connect(g_user_signal, &GlobalUserSignal::setDeviceBoxDisableSignal, this, &ScanSettingsWidget::setDeviceBoxDisableSlot);
    connect(GlobalUserSignal::getInstance(),&GlobalUserSignal::rotationChangedSig,this,&ScanSettingsWidget::rotateChangedSlot);
	connect(g_user_signal, &GlobalUserSignal::setScanIconDisableSignal, this, &ScanSettingsWidget::setScanIconDisable);
    connect(g_user_signal, &GlobalUserSignal::updateSettingBtnSignal, this, &ScanSettingsWidget::updateSettingBtnSlotForInstallDriver);

    connect(g_user_signal, &GlobalUserSignal::setPageNumberSignal, this, [=](QString text){
        if (text.compare("Multiple", Qt::CaseInsensitive) == 0){
            m_pageNumberComboBox->setCurrentText(tr("Multiple"));
        }else{
            m_pageNumberComboBox->setCurrentText(tr("Single"));
        }
    });

    connect(g_user_signal, &GlobalUserSignal::setScannerTypeSignal, this, [=](QString text){
        m_typeComboBox->setCurrentText(text);
    });

    connect(g_user_signal, &GlobalUserSignal::setScannerColorSignal, this, [=](QString text){
        if (0 == QString::compare(text, "Color", Qt::CaseInsensitive)) {
            m_colorComboBox->setCurrentText(tr("Color"));
        } else if(0 == QString::compare(text, "Lineart", Qt::CaseInsensitive)) {
            m_colorComboBox->setCurrentText(tr("Lineart"));
        } else {
           m_colorComboBox->setCurrentText(tr("Gray"));
        }
    });

    connect(g_user_signal, &GlobalUserSignal::setScannerResolutionSignal, this, [=](QString text){
        m_resolutionComboBox->setCurrentText(text);
    });


    connect(g_user_signal, &GlobalUserSignal::setScannerPageSizeSignal, this, [=](QString text){
        m_sizeComboBox->setCurrentText(text);
    });

    connect(g_user_signal, &GlobalUserSignal::setScannerFormatSignal, this, [=](QString text){
        m_formatComboBox->setCurrentText(text);
    });

    connect(g_user_signal, &GlobalUserSignal::setScannerSavePathSignal, this, [=](QString text){
        currentSaveDirectory = text;
        g_sane_object->userInfo.saveDirectory = currentSaveDirectory;
        setSaveButtonLabelAttributes(m_saveDirectoryButtonLabel, text, ScanSettingsButtonElideWidth);
    });
}
void ScanSettingsWidget::rotateChangedSlot(bool isPCMode){
    if(isPCMode){
        m_scanButton->setFixedHeight(36);
        m_deviceComboBox->setFixedHeight(36);
        m_pageNumberComboBox->setFixedHeight(36);
        m_typeComboBox->setFixedHeight(36);
        m_colorComboBox->setFixedHeight(36);
        m_resolutionComboBox->setFixedHeight(36);
        m_sizeComboBox->setFixedHeight(36);
        m_formatComboBox->setFixedHeight(36);
        m_saveNameEdit->setFixedHeight(36);
        m_saveDirectoryButton->setFixedHeight(36);
    }else{
        m_scanButton->setFixedHeight(48);
        m_deviceComboBox->setFixedHeight(48);
        m_pageNumberComboBox->setFixedHeight(48);
        m_typeComboBox->setFixedHeight(48);
        m_colorComboBox->setFixedHeight(48);
        m_resolutionComboBox->setFixedHeight(48);
        m_sizeComboBox->setFixedHeight(48);
        m_formatComboBox->setFixedHeight(48);
        m_saveNameEdit->setFixedHeight(48);
        m_saveDirectoryButton->setFixedHeight(48);
    }
}

void ScanSettingsWidget::updateScanButtonSettings()
{
    bool saneStatus = g_sane_object->getSaneStatus();

    m_scanButton->setEnabled(saneStatus);
    if(saneStatus == true){
        scanButtonRightLabel->setStyleSheet("color:white;");
    }else{
        scanButtonRightLabel->setStyleSheet("color:grey;");
    }
}

void ScanSettingsWidget::updateDeviceSettings()
{
    QStringList deviceStringList;

    bool saneStatus = g_sane_object->getSaneStatus();

    if (! saneStatus) {
        /// 1. Not find any scan device
        /// 2. This device open false

        deviceStringList = g_sane_object->getSaneNames();
        if (deviceStringList.count() > 1) {
            // Not only one scan device, just this scanner open false, we need set all sane names
        } else {
            //deviceStringList << tr("No available scanners");
        }
    } else {
        deviceStringList = g_sane_object->getSaneNames();
    }

    m_deviceComboBox->setEnabled(true);

    qDebug() << "Devices :  " << deviceStringList;
    setComboboxAttributes(m_deviceComboBox, deviceStringList);
}

/**
 * @brief ScanSettingsWidget::updatePageNumberSettings
 * Single page:
 *      user choose it, not get from scanners,
 *      only scan once.
 *
 * Multiple page:
 *      user choose it, not get from scanners
 *      scan lots of times, only stop by clicked scan button again or not pages in ADF scanners
 */
void ScanSettingsWidget::updatePageNumberSettings()
{
    QStringList pageNumberStringList;

    bool saneStatus = g_sane_object->getSaneStatus();

    pageNumberStringList << tr("Single") << tr("Multiple");

    qDebug() << "PageNumber :  " << pageNumberStringList;

    m_pageNumberComboBox->setEnabled(saneStatus);
    setComboboxAttributes(m_pageNumberComboBox, pageNumberStringList);
}


void ScanSettingsWidget::updateTypeSettings()
{
    QStringList typeStringList;

    bool saneStatus = g_sane_object->getSaneStatus();


    if (! saneStatus) {
        typeStringList << tr("Flatbed") << tr("ADF");
    } else {
        typeStringList = g_sane_object->getSaneTypes();
    }



    m_typeComboBox->setEnabled(saneStatus);

    if (! typeStringList.isEmpty()) {
        qDebug() << "Type :  " << typeStringList;
    } else {
        m_typeComboBox->setEnabled(false);
    }

    if (g_sane_object->haveSourceFlag == 0) {
        // For ESCL scanner device, which not get source type parameter
        //m_typeComboBox->setEnabled(false);
    }
    setComboboxAttributes(m_typeComboBox, typeStringList);
}

void ScanSettingsWidget::updateColorSettings()
{
    QStringList colorStringList;

    bool saneStatus = g_sane_object->getSaneStatus();

    if (! saneStatus) {
        colorStringList << tr("Gray") << tr("Color") << tr("Lineart");
    } else {
        colorStringList = g_sane_object->getSaneColors();
    }

    m_colorComboBox->setEnabled(saneStatus);

    // Avoid segmentation fault
    if (! colorStringList.isEmpty()) {
        qDebug() << "Color: " << colorStringList;
    }else{
        m_colorComboBox->setEnabled(false);
    }
    setComboboxAttributes(m_colorComboBox, colorStringList);
}

void ScanSettingsWidget::updateResolutionSettings(bool DetectedDevices)
{
    QStringList resolutionStringList;

    bool saneStatus = g_sane_object->getSaneStatus();

    if (! saneStatus) {
        resolutionStringList << tr("75 dpi") << tr("100 dpi") << tr("150 dpi");
    } else {
        resolutionStringList = g_sane_object->getSaneResolutions();
    }

    if (! resolutionStringList.isEmpty()) {
        qDebug() << "resolution: " << resolutionStringList;

        m_resolutionComboBox->setEnabled(saneStatus);
        setComboboxAttributes(m_resolutionComboBox, resolutionStringList);
    } else {
        /// Cannot get resolution from scanner: FUJITSU fi-7140
        resolutionStringList.clear();
        qDebug() << "resolution is empty! ";
        m_resolutionComboBox->setEnabled(false);

        if (DetectedDevices) {
            QString msg = tr("Resolution is empty!");
            warnMsg(msg);
        }
    }

}


void ScanSettingsWidget::updateSizeSettings()
{
    QStringList sizeStringList;

    bool saneStatus = g_sane_object->getSaneStatus();

    if (! saneStatus) {
        sizeStringList << "A4" << "A5";
    } else {
        sizeStringList = g_sane_object->getSaneSizes();
    }
    m_sizeComboBox->setEnabled(saneStatus);
    if(sizeStringList.isEmpty()){
        m_sizeComboBox->setEnabled(false);
    }
    setComboboxAttributes(m_sizeComboBox, sizeStringList);
}

/**
 * @brief ScanSettingsWidget::updateFormatSettings
 * format is choosed by user, so we need set it default.
 */
void ScanSettingsWidget::updateFormatSettings()
{
    QStringList formatStringList;

    bool saneStatus = g_sane_object->getSaneStatus();

    formatStringList << "jpg" << "png" << "pdf" << "bmp" << "tiff" << "ofd";

    m_formatComboBox->setEnabled(saneStatus);

    setComboboxAttributes(m_formatComboBox, formatStringList);

}

/**
 * @brief ScanSettingsWidget::updateSaveNameTextSettings
 * Consider multiple scan, so we set nameEdit timestamp.
 */
void ScanSettingsWidget::updateSaveNameTextSettings()
{
    bool saneStatus = g_sane_object->getSaneStatus();

    QDateTime currentTime = QDateTime::currentDateTime();
    qDebug() << "current time: " <<currentTime;

    QString currentTimeString = currentTime.toString("yyyy_MM_dd-hh_mm_ss_zzz");
    qDebug() << currentTimeString;

    m_saveNameEdit->setText(currentTimeString);
    m_saveNameEdit->setAlignment(Qt::AlignLeft);
    g_sane_object->nowSaveName = currentTimeString;

    m_saveNameEdit->setEnabled(saneStatus);
}

void ScanSettingsWidget::updateSaveDirectorySettings()
{
    bool saneStatus = g_sane_object->getSaneStatus();
    m_saveDirectoryButton->setEnabled(saneStatus);
}

void ScanSettingsWidget::updateSendMailSettings()
{
    bool saneStatus = g_sane_object->getSaneStatus();
    m_sendMailButton->setEnabled(saneStatus);
}

void ScanSettingsWidget::updateSaveAsSettings()
{
    bool saneStatus = g_sane_object->getSaneStatus();
    m_SaveAsButton->setEnabled(saneStatus);
}

void ScanSettingsWidget::updateSaveAsTextStore()
{
    if(g_sane_object->ocrFlag){
        m_SaveAsButton->setText(tr("Store text"));
        m_SaveAsButton->setToolTip(tr("Store text"));
    }
    qDebug() << "m_SaveAsButton text: " << m_SaveAsButton->text();
}

void ScanSettingsWidget::updateSaveAsTextRecover()
{
    qDebug() << "m_SaveAsButton text: " << m_SaveAsButton->text();
    m_SaveAsButton->setText(tr("Save as"));
    m_SaveAsButton->setToolTip(tr("Save as"));
}


/**
 * @brief ScanSettingsWidget::updateSettings
 * update scan settings while detect and open the default scanner successfully
 */
void ScanSettingsWidget::updateSettingsForDetectDevices()
{
    updateScanButtonSettings();
    updateDeviceSettings();
    updatePageNumberSettings();
    updateTypeSettings();
    updateColorSettings();
    updateResolutionSettings(true);
    updateSizeSettings();
    updateFormatSettings();
    updateSaveNameTextSettings();
    updateSaveDirectorySettings();
    if(ThumbnailWidget::scanPictureIsEmpty){
        m_sendMailButton->setEnabled(false);
        m_SaveAsButton->setEnabled(false);
    }else{
        m_sendMailButton->setEnabled(true);
        m_SaveAsButton->setEnabled(true);
    }
}

void ScanSettingsWidget::updateSettingsForSwitchDevices()
{
    updateScanButtonSettings();
    updatePageNumberSettings();
    updateTypeSettings();
    updateColorSettings();
    updateResolutionSettings(true);
    updateSizeSettings();
    updateFormatSettings();
    updateSaveNameTextSettings();
    updateSaveDirectorySettings();
    if(ThumbnailWidget::scanPictureIsEmpty){
        m_sendMailButton->setEnabled(false);
        m_SaveAsButton->setEnabled(false);
    }else{
        m_sendMailButton->setEnabled(true);
        m_SaveAsButton->setEnabled(true);
    }
    g_sane_object->getScannerSettings();
}

void ScanSettingsWidget::updateSettingsStatusForStartScan()
{
    m_deviceComboBox->setEnabled(false);
    m_scanButton->setEnabled(false);
    scanButtonRightLabel->setStyleSheet("color:grey;");
    m_pageNumberComboBox->setEnabled(false);
    m_typeComboBox->setEnabled(false);
    m_colorComboBox->setEnabled(false);
    m_resolutionComboBox->setEnabled(false);
    m_sizeComboBox->setEnabled(false);
    m_formatComboBox->setEnabled(false);
    m_saveNameEdit->setEnabled(false);
    m_saveDirectoryButton->setEnabled(false);
    m_sendMailButton->setEnabled(false);
    m_SaveAsButton->setEnabled(false);
}

void ScanSettingsWidget::updateSettingsStatusForEndScan(int saneStatus)
{
    qDebug() << "status: " << saneStatus;
    m_deviceComboBox->setEnabled(true);

    if (saneStatus == SANE_STATUS_NO_DOCS
            || saneStatus == SANE_STATUS_DEVICE_BUSY
            || saneStatus == SANE_STATUS_CANCELLED) {

        m_scanButton->setEnabled(true);
        scanButtonRightLabel->setStyleSheet("color:white;");

        m_pageNumberComboBox->setEnabled(true);
        m_typeComboBox->setEnabled(true);
        m_colorComboBox->setEnabled(true);
        m_resolutionComboBox->setEnabled(true);
        m_sizeComboBox->setEnabled(true);
        m_formatComboBox->setEnabled(true);
        m_saveNameEdit->setEnabled(true);
        m_saveDirectoryButton->setEnabled(true);
        m_sendMailButton->setEnabled(true);
        m_SaveAsButton->setEnabled(true);

    } else if (saneStatus == SANE_STATUS_INVAL) {
        m_scanButton->setEnabled(true);
        scanButtonRightLabel->setStyleSheet("color:white;");
        m_pageNumberComboBox->setEnabled(true);
        m_typeComboBox->setEnabled(true);
        m_colorComboBox->setEnabled(true);
        m_resolutionComboBox->setEnabled(true);
        m_sizeComboBox->setEnabled(true);
        m_formatComboBox->setEnabled(true);
        m_saveNameEdit->setEnabled(true);
        m_saveDirectoryButton->setEnabled(true);

        m_sendMailButton->setEnabled(false);
        m_SaveAsButton->setEnabled(false);
    } else {
        bool saneStatusBool = true;
        if (saneStatus == SANE_STATUS_GOOD || saneStatus == SANE_STATUS_EOF) {
            saneStatusBool = true;
        } else {
            saneStatusBool = false;
        }
        m_scanButton->setEnabled(saneStatusBool);
        if(saneStatusBool == true){
            scanButtonRightLabel->setStyleSheet("color:white;");
        }else{
            scanButtonRightLabel->setStyleSheet("color:grey;");
        }

        m_pageNumberComboBox->setEnabled(saneStatusBool);
        m_typeComboBox->setEnabled(saneStatusBool);
        m_colorComboBox->setEnabled(saneStatusBool);
        m_resolutionComboBox->setEnabled(saneStatusBool);
        m_sizeComboBox->setEnabled(saneStatusBool);
        m_formatComboBox->setEnabled(saneStatusBool);
        m_saveNameEdit->setEnabled(saneStatusBool);
        m_saveDirectoryButton->setEnabled(saneStatusBool);
        m_sendMailButton->setEnabled(saneStatusBool);
        m_SaveAsButton->setEnabled(saneStatusBool);
    }
}

void ScanSettingsWidget::setLabelAttributes(QLabel *label, const QString &text, int labelHeight)
{
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setFixedWidth(ScanSettingsLabelWidth);
    label->setText(text);

    int elideWidth = ScanSettingsLabelElideWidth;

    QFontMetrics fontMetrics(label->font());

    if (fontMetrics.width(text) > label->width()) {
        QString elideText = QFontMetrics(label->font()).elidedText(text, Qt::ElideRight, elideWidth);
        label->setText(elideText);
        label->setToolTip(text);
    } else {
        label->setText(text);
        label->setToolTip("");
    }
}


void ScanSettingsWidget::setSaveButtonAttributes(QPushButton *button, const QString &text, int elideWidth)
{
    button->setStyleSheet("QPushButton{text-align:left;}");
    QFontMetrics fontMetrics(button->font());

    if (fontMetrics.width(text) > elideWidth) {
        QString elideText = QFontMetrics(button->font()).elidedText(text, Qt::ElideRight, elideWidth);
        button->setText(elideText);
        button->setToolTip(text);
    } else {
        button->setText(text);
        button->setToolTip("");
    }

}

void ScanSettingsWidget::setSaveButtonLabelAttributes(QLabel *label, const QString &text, int elideWidth)
{
    QFontMetrics fontMetrics(label->font());

    if (fontMetrics.width(text) > elideWidth) {
        QString elideText = QFontMetrics(label->font()).elidedText(text, Qt::ElideRight, elideWidth);
        label->setText(elideText);
        label->setToolTip(text);
    } else {
        label->setText(text);
        label->setToolTip("");
    }

}

void ScanSettingsWidget::setComboboxAttributes(QComboBox *combobox, QStringList strList)
{
    QStandardItemModel *model = new QStandardItemModel();
    int maxTooltipTextLength = 20;

    combobox->clear();

    for (int i = 0; i < strList.size(); ++i) {
        QStandardItem *item = new QStandardItem(strList.at(i));;
        int curTextLen = strList.at(i).length();

        if ( curTextLen >= maxTooltipTextLength) {
            item->setToolTip(strList.at(i));
        } else {
            item->setToolTip("");
        }

        model->appendRow(item);
    }
    combobox->setModel(model);
    combobox->setInsertPolicy(QComboBox::NoInsert);
    combobox->setFocusPolicy(Qt::ClickFocus);
}

void ScanSettingsWidget::setNameEditTooltip()
{
    QString str = m_saveNameEdit->text ();
    m_saveNameEdit->setToolTip (str);
}
void ScanSettingsWidget::warnMsg(QString msg)
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setObjectName("warnMsgBox");
    msgBox->setText(msg);
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setWindowTitle(QApplication::tr("Scanner"));
    msgBox->setStandardButtons(QMessageBox::Yes);
    msgBox->setContextMenuPolicy(Qt::NoContextMenu);
    msgBox->button(QMessageBox::Yes)->setText(tr("Yes"));

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        msgBox->setParent(widget);
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - msgBox->width()/2;
        int y = rect.y() + rect.height()/2 - msgBox->height()/2;
        msgBox->move(x,y);
    }

    msgBox->exec();
}

inline int realLength(const char* str) {
    // 设置当前程序的区域（locale），以支持宽字符处理
    setlocale(LC_ALL, "");

    // 将多字节字符串转换为宽字符字符串
    wchar_t wstr[1024];
    mbstowcs(wstr, str, sizeof(wstr) / sizeof(wchar_t));

    // 计算宽字符字符串的显示宽度
    return wcswidth(wstr, wcslen(wstr));
}

// 截取指定显示宽度的字符串
std::string truncateToWidth(const std::string& utf8Str, int maxWidth) {
    setlocale(LC_ALL, ""); // 设置区域以支持多字节字符

    // 将 UTF-8 转为宽字符
    wchar_t wstr[1024];
    mbstowcs(wstr, utf8Str.c_str(), sizeof(wstr) / sizeof(wchar_t));

    std::wstring truncated;
    int currentWidth = 0;

    // 遍历宽字符，累加显示宽度，直到超过 maxWidth
    for (size_t i = 0; wstr[i] != L'\0'; ++i) {
        int charWidth = wcwidth(wstr[i]);
        if (charWidth < 0) charWidth = 0; // 避免负值
        if (currentWidth + charWidth > maxWidth) {
            break; // 超过最大宽度，停止截取
        }
        truncated += wstr[i];
        currentWidth += charWidth;
    }

    // 将截取结果转回 UTF-8
    char result[1024];
    wcstombs(result, truncated.c_str(), sizeof(result));
    return std::string(result);
}
