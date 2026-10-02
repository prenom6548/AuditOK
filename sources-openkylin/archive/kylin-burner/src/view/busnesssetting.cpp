#include "busnesssetting.h"
#include "ui_busnesssetting.h"
#include "src/data/datacdrom.h"
#include "src/po/translation.h"
#include "src/view/modaldialog.h"
#include "src/model/logicburnimage.h"
#include "src/model/logiccopydisc.h"
#include "src/model/logicmain.h"
#include "src/frame/signals.h"
#include "src/frame/logger.h"
#include "src/view/components/iconlabel.h"
#include "src/view/multiselectionfiledialog.h"

#include <QPalette>
#include <QDir>
#include <QStorageInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QFont>

BusnessSetting::BusnessSetting(QWidget *parent) :
    QWidget(parent),
    m_bVerfication(false),
    m_bCreateISO(false),
    ui(new Ui::BusnessSetting)
{
    this->setObjectName("kylin-burner_BusnessSetting");
    QFont f;
    ui->setupUi(this);
    setAccessibleName();

    ui->labelCopyright->setText(_("Copyright"));

    ui->labelPreparer->setText(_("Preparer"));
    ui->labelPublisher->setText(_("Publisher"));
    ui->labelVolumnID->setText(_("VoumnID"));
    ui->labelTempPath->setText(_("Temporary File"));
    ui->labelBurnSpeed->setText(_("Burn Speed"));
    ui->labelBurnFormat->setText(_("File System"));
    ui->checkBoxCloseSeeion->setText(_("Close session"));
    ui->checkBoxCreateISO->setText(_("Create ISO"));
    ui->checkBoxVerfication->setText(_("Data Verfication"));

    ui->labelCheckVolumnID->setText("");
    ui->labelCheckVolumnID->setStyleSheet("QLabel { color: red; }");
    ui->labelCheckVolumnID->hide();

    ui->labelCheckCopyright->setText("");
    ui->labelCheckCopyright->setStyleSheet("QLabel { color: red;}");
    ui->labelCheckCopyright->hide();

    m_isVaildVolumID = true;
    m_isVaildCopyright = true;

    m_oBrowsePath.clear();
    m_oCDROM = nullptr;
    m_isShowBeforBurn = false;

    connect(ui->lineEditVolumnID, &QLineEdit::textChanged, this, &BusnessSetting::checkVolumnID);
    connect(ui->lineEditCopyright, &QLineEdit::textChanged, this, &BusnessSetting::checkCopyright);
    connect(ui->lineEditTempPath, &QLineEdit::textChanged, this, &BusnessSetting::checkTempPath);
    connect(ui->comboBoxBurnFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &BusnessSetting::slotChangeFileSystemFormat);
    uiAdjust();
    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &BusnessSetting::slotChangeUIFontSize);
}

void BusnessSetting::setAccessibleName()
{
    ui->checkBoxCloseSeeion->setAccessibleName("kylin-burner_QCheckBox_BusnessSetting_checkBoxCloseSeeion");
    ui->checkBoxCreateISO->setAccessibleName("kylin-burner_QCheckBox_BusnessSetting_checkBoxCreateISO");
    ui->checkBoxVerfication->setAccessibleName("kylin-burner_QCheckBox_BusnessSetting_checkBoxVerfication");
    ui->comboBoxBurnFormat->setAccessibleName("kylin-burner_QComboBox_BusnessSetting_comboBoxBurnFormat");
    ui->comboBoxBurnSpeed->setAccessibleName("kylin-burner_QComboBox_BusnessSetting_comboBoxBurnSpeed");

    ui->labelBurnFormat->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelBurnFormat");
    ui->labelBurnSpeed->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelBurnSpeed");
    ui->labelCheckCopyright->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelCheckCopyright");
    ui->labelCheckVolumnID->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelCheckVolumnID");
    ui->labelCopyright->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelCopyright");
    ui->labelISOInfo->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelISOInfo");
    ui->labelPreparer->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelPreparer");
    ui->labelPublisher->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelPublisher");
    ui->labelTempPath->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelTempPath");
    ui->labelVolumnID->setAccessibleName("kylin-burner_QLabel_BusnessSetting_labelVolumnID");

    ui->layoutWidget->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget");
    ui->layoutWidget1->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget1");
    ui->layoutWidget2->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget2");
    ui->layoutWidget3->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget3");
    ui->layoutWidget4->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget4");
    ui->layoutWidget5->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget5");
    ui->layoutWidget6->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget6");
    ui->layoutWidget7->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget7");
    ui->layoutWidget8->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget8");
    ui->layoutWidget9->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget9");
    ui->layoutWidget10->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget10");
    //ui->layoutWidget11->setAccessibleName("kylin-burner_QWidget_BusnessSetting_layoutWidget11");

    ui->line->setAccessibleName("kylin-burner_Line_BusnessSetting_line");
    ui->lineEditCopyright->setAccessibleName("kylin-burner_CustomLineEdit_BusnessSetting_lineEditCopyright");
    ui->lineEditPreparer->setAccessibleName("kylin-burner_CustomLineEdit_BusnessSetting_lineEditPreparer");
    ui->lineEditPublisher->setAccessibleName("kylin-burner_CustomLineEdit_BusnessSetting_lineEditPublisher");
    ui->lineEditTempPath->setAccessibleName("kylin-burner_QLineEdit_BusnessSetting_lineEditTempPath");
    ui->lineEditVolumnID->setAccessibleName("kylin-burner_QLineEdit_BusnessSetting_lineEditVolumnID");
    ui->line_2->setAccessibleName("kylin-burner_Line_BusnessSetting_line2");
    ui->line_3->setAccessibleName("kylin-burner_Line_BusnessSetting_line3");
    ui->line_4->setAccessibleName("kylin-burner_Line_BusnessSetting_line4");

    ui->scrollArea->setAccessibleName("kylin-burner_QScrollArea_BusnessSetting_scrollArea");
    ui->scrollAreaWidgetContents->setAccessibleName("kylin-burner_QWidget_BusnessSetting_scrollAreaWidgetContents");
    ui->title->setAccessibleName("kylin-burner_QLabel_BusnessSetting_title");
}

void BusnessSetting::load(void *d)
{
    //点击"设置"按钮后，为界面赋值
    DataCDROM *cdrom;
    m_oCDROM = d;

    cdrom = static_cast<DataCDROM *>(d);

    cdrom->setting();

    ui->checkBoxCloseSeeion->setEnabled(false);
    ui->checkBoxCreateISO->setEnabled(true);
    ui->checkBoxCreateISO->setChecked(true);
    ui->checkBoxVerfication->setEnabled(true);

    ui->lineEditCopyright->setReadOnly(false);
    ui->lineEditPreparer->setReadOnly(false);
    ui->lineEditPublisher->setReadOnly(false);
    ui->lineEditVolumnID->setReadOnly(false);

    ui->lineEditVolumnID->setFocus();

    if (cdrom->isCDROM())
    {
        ui->labelISOInfo->setText(_("Media Info")); // 光盘信息
        ui->labelVolumnID->setText(_("Medium Name")); // 光盘名称
    }
    else
    {
        ui->labelISOInfo->setText(_("ISO info")); // 镜像信息
        ui->labelVolumnID->setText(_("VoumnID")); // 镜像名称
    }

    if (cdrom->isCDROM() && cdrom->mediumStatus() != DataMedium::MEDIUM_EMPTY)
    {
        ui->lineEditCopyright->setReadOnly(true);
        ui->lineEditPreparer->setReadOnly(true);
        ui->lineEditPublisher->setReadOnly(true);
    }
    else
    {
        ui->lineEditCopyright->setReadOnly(false);
        ui->lineEditPreparer->setReadOnly(false);
        ui->lineEditPublisher->setReadOnly(false);
    }

    ui->lineEditCopyright->setCursor(Qt::IBeamCursor);
    ui->lineEditPreparer->setCursor(Qt::IBeamCursor);
    ui->lineEditPublisher->setCursor(Qt::IBeamCursor);

    m_oSpeed = cdrom->mediumSpeed();
    m_oSpeeds.clear();
    while (m_oSpeed.size())
    {
        m_oSpeeds << m_oSpeed.takeFirst().toInt();
    }
    ui->comboBoxBurnSpeed->clear();
    ui->comboBoxBurnSpeed->addItem(_("default"), 0);

    QString suportWriteSpeedListStr = "Current Medium Support Write Speed = ";
    QString showWtireSpeedListStr = "BusnessSetting View Show Currnet Meduim Support Write Speed = ";
    for (int i = 0; i < m_oSpeeds.size(); ++i)
    {
        suportWriteSpeedListStr += QString(" %1;").arg(m_oSpeeds.at(i));
        if (cdrom->mediumType().contains("DVD+R DL") && (m_oSpeeds.at(i) > 4)) {
            continue;
        }
        showWtireSpeedListStr += QString(" %1;").arg(m_oSpeeds.at(i));
        ui->comboBoxBurnSpeed->addItem(QString("%1x").arg(m_oSpeeds.at(i)), m_oSpeeds.at(i));
    }
    LOGGER->info(suportWriteSpeedListStr);
    LOGGER->info(showWtireSpeedListStr);

    if (0 == m_oSpeeds.size()) {
        ui->comboBoxBurnSpeed->setEnabled(false);
    } else {
        ui->comboBoxBurnSpeed->setEnabled(true);
    }

    //使用UDF格式或ISO9660，默认ISO9660
    m_oBurnFormat.clear();
    if(!cdrom->isUdf()){
        m_oBurnFormat<< QString("ISO9660");
    }

    if(cdrom->isEnableUDF()) {
        m_oBurnFormat<< QString("UDF");
    }

    if (m_oBurnFormat.isEmpty()) {
        ui->comboBoxBurnFormat->setEnabled(false);
    } else {
        ui->comboBoxBurnFormat->clear();
        for(int i=0; i<m_oBurnFormat.size(); ++i) {
            ui->comboBoxBurnFormat->addItem(QString("%1").arg(m_oBurnFormat.at(i)));
        }
        ui->comboBoxBurnFormat->setEnabled(true);
    }

    updateCheckBoxStates();

    m_oPreparer = cdrom->settingPreparer();
    ui->lineEditPreparer->setText(m_oPreparer);

    m_oCopyright = cdrom->settingCopyright();
    ui->lineEditCopyright->setText(m_oCopyright);

    m_oPublisher = cdrom->settingPublisher();
    ui->lineEditPublisher->setText(m_oPublisher);

    m_oVolumnID = cdrom->settingVolumnID();
    QString mediaName = cdrom->mediumName();

    if(!m_oVolumnID.isEmpty()){
        ui->lineEditVolumnID->setText(m_oVolumnID);
    }else{
         ui->lineEditVolumnID->setText(mediaName);
    }

    if (BURNIMAGE == sender() || cdrom->mediumType().contains("DVD+RW")) {
        m_bCloseSession = true;
    }
    else {
        m_bCloseSession = cdrom->settingCloseSession();
    }

    m_bVerfication = cdrom->settingVerficationData();
    ui->checkBoxVerfication->setChecked(cdrom->settingVerficationData());

    if (BURNIMAGE == sender() || cdrom->mediumType().contains("DVD"))
    {
        ui->checkBoxCloseSeeion->setEnabled(false);
        ui->checkBoxCreateISO->setEnabled(true);
    }
    else
    {
        ui->checkBoxCloseSeeion->setEnabled(false);
    }

    if (BURNIMAGE == sender() || LogicCopyDisc::instance() == sender())
    {
        ui->lineEditVolumnID->setReadOnly(true);
        ui->lineEditVolumnID->setText(_("KylinBurner"));
    }


    ui->checkBoxCreateISO->setChecked(m_bCreateISO);

    setTemporaryPath(m_oTemporaryPth);

    if (cdrom->isCDROM()) {
        ui->title->setText(_("BurnSetting"));
        title(_("BurnSetting"));

        ui->line->show();

        if (-1 != m_oSpeeds.indexOf(m_iSpeed)) {
            int index = ui->comboBoxBurnSpeed->findData(m_iSpeed);
            ui->comboBoxBurnSpeed->setCurrentIndex(index);
        }

        if (-1 != m_oBurnFormat.indexOf(m_SBurnFormat)) {
            ui->comboBoxBurnFormat->setCurrentIndex(m_oBurnFormat.indexOf(m_SBurnFormat));
        }

    }
    else {
        ui->title->setText(_("ISOSetting"));
        title(_("ISOSetting"));
    }

    if (LogicCopyDisc::instance() == sender()) {
        ui->lineEditCopyright->setReadOnly(true);
        ui->lineEditPreparer->setReadOnly(true);
        ui->lineEditPublisher->setReadOnly(true);
        ui->lineEditVolumnID->setReadOnly(true);

        ui->checkBoxCloseSeeion->setEnabled(false);
        ui->checkBoxCreateISO->setEnabled(true);
        ui->checkBoxVerfication->setEnabled(false);
    }

    if (ui->comboBoxBurnFormat->currentText().contains("ISO9660")) {
        ui->checkBoxCreateISO->setEnabled(true);
    }else{
        ui->checkBoxCreateISO->setEnabled(false);
    }

    //确定和取消按钮可用
    emit ensureBtnStateChanged(true);
    emit cancelBtnStateChanged(true);
    updateLabelText();
}

void BusnessSetting::setVolumnID(QString s)
{
     m_oVolumnID = s;
     ui->lineEditVolumnID->setText(m_oVolumnID);
}

void BusnessSetting::setTempPath(QString s)
{
    m_oTemporaryPth = s;
}

void BusnessSetting::resetText(QLabel *item)
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString item_name = item->text();
    if (item->toolTip() != ""
            && -1 != item_name.indexOf("…")
            && -1 != item->toolTip().indexOf(item_name.mid(0, item_name.length() - 3)))
    {
        item_name = item->toolTip();
    }
    QString show_name = fontWidth.elidedText(item_name, Qt::ElideRight, item->width() - 3);
    item->setText(show_name);
    if (show_name.endsWith("…"))
        item->setToolTip(item_name);
    else
        item->setToolTip("");
}

void BusnessSetting::updateLabelText()
{
    resetText(ui->labelBurnSpeed);
    resetText(ui->labelVolumnID);
    resetText(ui->labelPreparer);
    resetText(ui->labelPublisher);
    resetText(ui->labelCopyright);
    resetText(ui->labelTempPath);
//    resetText();
}

void BusnessSetting::slotChangeUIFontSize(int fontSize)
{
     int scaleRatio = qAbs(fontSize - 10.0);
     QFont font = ui->title->font();
     font.setPixelSize(16 + scaleRatio);
     font.setBold(true);
     ui->title->setFont(font);

     QFont defaultFont;
     defaultFont.setPixelSize(14 + scaleRatio);

     QList<QWidget*> widgetList;
     widgetList << ui->labelCopyright << ui->labelPreparer << ui->labelPublisher << ui->labelVolumnID
              <<  ui->labelTempPath << ui->labelBurnSpeed << ui->labelBurnFormat << ui->lineEditCopyright
              << ui->lineEditPreparer <<  ui->lineEditPublisher <<  ui->lineEditTempPath <<  ui->lineEditVolumnID
              << ui->comboBoxBurnSpeed <<  ui->checkBoxCloseSeeion <<  ui->checkBoxCreateISO <<  ui->checkBoxVerfication
              << ui->labelCheckVolumnID << ui->labelCheckCopyright << ui->labelISOInfo;
     foreach (QWidget* index, widgetList) {
         index->setFont(defaultFont);
     }
     updateLabelText();
}

void BusnessSetting::setTemporaryPath(QString path)
{
    QDir dir(path);
    if (path.isEmpty() || !dir.exists())
        return;

    ui->lineEditTempPath->setText(path);

    ui->lineEditTempPath->setToolTip(path);
    m_oBrowsePath = path;
}

void BusnessSetting::slotsBrowse()
{
#ifndef UNIT_TEST
    QString testPath;

    m_oBrowsePath = QFileDialog::getExistingDirectory(this, _("Browse"), m_oBrowsePath);
    if (!m_oBrowsePath.isEmpty())
    {
        testPath = m_oBrowsePath + "/.t/.k";
        QDir dir(testPath);
        if (dir.mkpath(testPath))
        {
            dir.removeRecursively();
            setTemporaryPath(m_oBrowsePath);
        }
        else
        {
            QMessageBox::critical(this, _("Invalid path"),
                                  _("Cannot choose a read "
                                    "only path %1 to "
                                    "temporary path").arg(m_oBrowsePath));
            m_oBrowsePath = m_oTemporaryPth;
        }
    }
#endif
}

void BusnessSetting::slotsAccept()
{
    setIsShowBeforBurn(false);

    DataCDROM *cdrom = static_cast<DataCDROM *>(m_oCDROM);
    if (!cdrom) {
        return;
    }

    cdrom->setSettingPreparer(ui->lineEditPreparer->text());
    cdrom->setSettingPublisher(ui->lineEditPublisher->text());
    cdrom->setSettingVolumnID(ui->lineEditVolumnID->text());
    cdrom->setSettingCopyright(ui->lineEditCopyright->text());
    cdrom->setSettingTempPath(m_oBrowsePath);
    cdrom->setSettingIsShowBeforBurn(m_isShowBeforBurn);

    if (cdrom->isCDROM()) {
        int speed = ui->comboBoxBurnSpeed->currentData().toInt();
        cdrom->setSettingSpeed(speed);
        cdrom->setSettingBurnFormat(m_oBurnFormat.at(ui->comboBoxBurnFormat->currentIndex()));
        cdrom->setSettingCloseSession(ui->checkBoxCloseSeeion->isChecked());
        cdrom->setSettingCreateISO(ui->checkBoxCreateISO->isChecked());
        cdrom->setSettingVerficationData(ui->checkBoxVerfication->isChecked());
        emit g_user_signal->changeBtnBusnessName();
    }

    disconnect(MODAL, SIGNAL(rejected()), this, SLOT(slotsAccept()));
}

BusnessSetting::~BusnessSetting()
{
    delete ui;
}

void BusnessSetting::uiAdjust(){
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    ui->lineEditVolumnID->setFixedWidth(251+29);
    ui->lineEditCopyright->setFixedWidth(251+29);
    ui->lineEditPreparer->setFixedWidth(251+29);
    ui->lineEditTempPath->setFixedWidth(251+29);
    ui->lineEditPublisher->setFixedWidth(251+29);
    ui->comboBoxBurnSpeed->setFixedWidth(251+29);
    ui->comboBoxBurnFormat->setFixedWidth(251+29);

    initTempPathLineEdit();
}

void BusnessSetting::initTempPathLineEdit()
{
    // 在QLineEdit中集成一个可点击图标
    ui->lineEditTempPath->setTextMargins(0, 0, 36, 0);
    ui->lineEditTempPath->setReadOnly(true);
    QHBoxLayout *searchLayout = new QHBoxLayout(ui->lineEditTempPath);
    searchLayout->setMargin(5);
    searchLayout->setSpacing(0);
    buttonBrowse = new IConLabel;
    buttonBrowse->setAccessibleName("kylin-burner_IConLabel_BusnessSetting_buttonBrowse");
    buttonBrowse->setToolTip(_("Browse"));
    buttonBrowse->setFixedSize(24, 24);
    if ("ukui-black" == LogicMain::instance()->theme()) {
        buttonBrowse->setThemeIconSchema("", ":/folder-visiting-symbolic-dark.svg", QSize(19,17));
    } else {
        buttonBrowse->setThemeIconSchema("folder-visiting-symbolic", ":/folder-visiting-symbolic.svg", QSize(19,17));
    }
    searchLayout->addWidget(buttonBrowse);
    searchLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#ifndef UNIT_TEST
    connect(buttonBrowse, &IConLabel::clicked, this, [=]() {
        MultiSelectionFileDialog selectPathDialog;
        selectPathDialog.setExtendedSelection();
        selectPathDialog.setDirectory(QDir::currentPath());
        selectPathDialog.setFileMode(QFileDialog::DirectoryOnly);
        selectPathDialog.setWindowTitle(_("Please select a path"));
        selectPathDialog.setWindowIcon(QIcon::fromTheme("burner"));
        if (selectPathDialog.exec()) {
            QStringList selectFiles = selectPathDialog.selectedFiles();
            if (!selectFiles.isEmpty()) {
                QString fileName = selectFiles.at(0);
                ui->lineEditTempPath->setText(fileName);
                ui->lineEditTempPath->setToolTip(fileName);
                m_oBrowsePath = fileName;
            }
        }
    });
#endif
}

void BusnessSetting::slotChangeFileSystemFormat(int index)
{
    if (index == 1) {
        // 切换UDF文件系统格式
        ui->checkBoxCreateISO->setChecked(false);
        ui->checkBoxCreateISO->setEnabled(false);
        ui->checkBoxVerfication->setChecked(false);
        ui->checkBoxVerfication->setEnabled(false);
        int index = ui->comboBoxBurnSpeed->findData(0);
        ui->comboBoxBurnSpeed->setCurrentIndex(index);
        ui->comboBoxBurnSpeed->setEnabled(false);

    } else if (index == 0){
        // 切换ISO9660文件系统格式
        ui->checkBoxCreateISO->setChecked(false);
        ui->checkBoxCreateISO->setEnabled(true);
        ui->checkBoxVerfication->setChecked(false);
        ui->checkBoxVerfication->setEnabled(true);
        ui->comboBoxBurnSpeed->setEnabled(true);
    }
}

void BusnessSetting::checkVolumnID()
{
    if (ui->lineEditVolumnID->text().isEmpty()) {
        ui->labelCheckVolumnID->hide();
        ui->verticalLayout_3->setSpacing(8);
        m_isVaildVolumID = false;
    } else {
        QByteArray lineTextVolumnID = ui->lineEditVolumnID->text().toUtf8();
        if (lineTextVolumnID.size() >= 33) {
            ui->labelCheckVolumnID->setText(_("Name is longer than 32 characters"));
            ui->verticalLayout_3->setSpacing(24);
            ui->labelCheckVolumnID->show();
            m_isVaildVolumID = false;
        } else {
            ui->labelCheckVolumnID->hide();
            ui->verticalLayout_3->setSpacing(8);
            m_isVaildVolumID = true;
        }
    }
    updateModelDialogEnsureBtnState();
}

void BusnessSetting::checkCopyright()
{
    QByteArray lineTextCopyright = ui->lineEditCopyright->text().toUtf8();
    if (lineTextCopyright.size() > 37) {
        ui->labelCheckCopyright->setText(_("Content is longer than 37 characters"));
        ui->labelCheckCopyright->show();
        m_isVaildCopyright = false;
    }
    else {
        ui->labelCheckCopyright->hide();
        m_isVaildCopyright = true;
    }
    updateModelDialogEnsureBtnState();
}

void BusnessSetting::checkTempPath()
{
    if (ui->lineEditTempPath->text().isEmpty()) {
        m_isVaildTempPath = false;
    } else {
        QByteArray lineTextTempPath = ui->lineEditTempPath->text().toUtf8();
        QDir dir(lineTextTempPath); 
        if (!dir.exists()) {
            m_isVaildTempPath = false;
        }
        else {
            m_isVaildTempPath = true;
        }
    }
    updateModelDialogEnsureBtnState();
}

void BusnessSetting::updateModelDialogEnsureBtnState()
{
    emit ensureBtnStateChanged(m_isVaildVolumID && m_isVaildCopyright && m_isVaildTempPath);
}

void BusnessSetting::paintEvent(QPaintEvent *event)
{
    QPalette pal = ui->line->palette();
    if ("ukui-black" == LogicMain::instance()->theme()) {
        pal.setColor(QPalette::Dark, QColor("#37373B"));
        ui->line->setFrameShadow(QFrame::Plain);
        ui->line_2->setFrameShadow(QFrame::Plain);
        ui->line_3->setFrameShadow(QFrame::Plain);
        ui->line_4->setFrameShadow(QFrame::Plain);
        buttonBrowse->setThemeIconSchema("", ":/folder-visiting-symbolic-dark.svg", QSize(19,17));
    } else {
        pal.setColor(QPalette::Dark, QColor("#F2F2F2"));
        ui->line->setFrameShadow(QFrame::Sunken);
        ui->line_2->setFrameShadow(QFrame::Sunken);
        ui->line_3->setFrameShadow(QFrame::Sunken);
        ui->line_4->setFrameShadow(QFrame::Sunken);
        buttonBrowse->setThemeIconSchema("folder-visiting-symbolic", ":/folder-visiting-symbolic.svg", QSize(19,17));
    }
    ui->line->setPalette(pal);
    ui->line_2->setPalette(pal);
    ui->line_3->setPalette(pal);
    ui->line_4->setPalette(pal);

    QWidget::paintEvent(event);
}

void BusnessSetting::updateCheckBoxStates()
{
    QString currentText = ui->comboBoxBurnFormat->currentText();

    ui->checkBoxCloseSeeion->setEnabled(false);

    ui->checkBoxCreateISO->setChecked(false);
    ui->checkBoxVerfication->setChecked(false);

    if (currentText == "UDF") {
        // 切换UDF文件系统格式
        ui->checkBoxCreateISO->setEnabled(false);
        ui->checkBoxVerfication->setEnabled(false);
        ui->comboBoxBurnSpeed->setEnabled(false);

    } else if (currentText == "ISO9660"){
        // 切换ISO9660文件系统格式
        ui->checkBoxCreateISO->setEnabled(true);
        ui->checkBoxVerfication->setEnabled(true);
        ui->comboBoxBurnSpeed->setEnabled(true);
    }
    else {
        ui->checkBoxCreateISO->setEnabled(false);
        ui->checkBoxVerfication->setEnabled(false);
    }
}

CustomLineEdit::CustomLineEdit(QWidget *parent) : QLineEdit(parent)
{

}
