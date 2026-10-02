/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "phoneinfowidget.h"
#include "preferences.h"
#include "utils.h"
#include "global.h"
#include "messagebox.h"
#include "customwidget.h"

#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDBusInterface>
#include <QDBusReply>
#include <QMessageBox>
#include <QTime>
#include <unistd.h>
#include <QProcess>
#include <QGSettings>
#include <sys/syslog.h>

using namespace Global;
PhoneInfoWidget::PhoneInfoWidget(QWidget *parent)
    :SettingScrollContent(parent)
{
    m_systemBusInterface = new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus(), this);
    QFrame *centralWidget = new QFrame;
    m_UserName = kmre::utils::getUserName();
    m_UserId = kmre::utils::getUid();
    m_pref = new Preferences;
    modelLabel = new QLabel;
    IMEILabel = new QLabel;
    Label1 = new QLabel;
    Label2 = new QLabel;
    Label3 = new QLabel;
    Label4 = new QLabel;
    line1 = new QLineEdit;
    line2 = new QLineEdit;
    IMEIline = new QLineEdit;
    presetBox1 = new QComboBox;
    presetBox2 = new QComboBox;
    presetBox3 = new QComboBox;
    presetBox4 = new QComboBox;
    presetBox5 = new QComboBox;
    presetBox6 = new QComboBox;
    modelLabel->setText(tr("Phone model"));
    IMEILabel->setText(tr("IMEI setting"));
    auto *presetBtn = new CustomWidget<QRadioButton>(tr("preset model"), 130, 150, 0);
    auto *customBtn = new CustomWidget<QRadioButton>(tr("custom"), 130, 150, 0);
    auto *randBtn = new CustomWidget<QPushButton>(tr("random"), 80, 90, 0);
    presetBtn->setChecked(!m_pref->m_PhoneInfoCustom);
    customBtn->setChecked(m_pref->m_PhoneInfoCustom);
    custom = m_pref->m_PhoneInfoCustom;

    modelLabel->setFixedWidth(100);
    QFont ft;
    QFontMetrics fm(ft);
    QString elided_text = fm.elidedText(tr("Phone model"), Qt::ElideRight, 100);
    modelLabel->setText(elided_text);
    if (fm.width(tr("Phone model")) > modelLabel->width()) {
        modelLabel->setToolTip(tr("Phone model"));
    }
    IMEILabel->setFixedWidth(100);
    Label1->setText(tr("brand"));
    Label2->setText(tr("model"));
    Label3->setText(tr("brand"));
    Label4->setText(tr("model"));
    Label1->setFixedWidth(100);
    Label2->setFixedWidth(100);
    Label3->setFixedWidth(100);
    Label4->setFixedWidth(100);
//    Label1->setText(tr("vendor"));
//    Label3->setText(tr("name"));
//    Label5->setText(tr("equip"));
//    Label3->setFixedWidth(80);
//    Label4->setFixedWidth(80);
//    Label5->setFixedWidth(80);
    line1->setFixedWidth(350);
    line2->setFixedWidth(350);
//    line3->setFixedWidth(150);
//    line4->setFixedWidth(150);
//    line5->setFixedWidth(150);
    IMEIline->setFixedWidth(240);
    QRegExp regExp("([0-9]{15})");
    QRegExpValidator *validator = new QRegExpValidator(regExp, this);
    IMEIline->setValidator(validator);
    presetFrame = new QFrame;
    customFrame = new QFrame;

    presetBox1->setFixedWidth(350);
    presetBox2->setFixedWidth(350);
    presetBox3->setFixedWidth(350);
    presetBox4->setFixedWidth(350);
    presetBox5->setFixedWidth(350);
    presetBox6->setFixedWidth(350);
    presetBox1->addItem(tr("default"));
    presetBox1->addItem(tr("Samsung"));
    presetBox1->addItem(tr("HUAWEI"));
    presetBox1->addItem(tr("Xiaomi"));
    presetBox1->addItem(tr("ZTE"));
//    presetBox2->addItem(tr("please choose.."));
    presetBox2->addItem(tr("default"));
    presetBox2->setVisible(false);
    presetBox3->addItem("Galaxy S10");
    presetBox3->setVisible(false);
    presetBox4->addItem("Mate 40");
    presetBox4->setVisible(false);
    presetBox5->addItem("MI 2");
    presetBox5->setVisible(false);
    presetBox6->addItem("N928Dt");
    presetBox6->setVisible(false);

    presetFrame->setVisible(!m_pref->m_PhoneInfoCustom);
    customFrame->setVisible(m_pref->m_PhoneInfoCustom);

    if (!custom) {
        presetBox1->setCurrentIndex(m_pref->PhonepresetbrandIndex);
        this->setpresetbrandInfo(m_pref->PhonepresetbrandIndex);
    }
    if (custom) {
        line1->setText(m_pref->PhoneBrand);
        line2->setText(m_pref->PhoneModel);
//        line1->setText(m_pref->PhoneVendor);
//        line3->setText(m_pref->PhoneName);
//        line5->setText(m_pref->PhoneEquip);
    }
    IMEIline->setText(m_pref->PhoneIMEI);

    this->m_title->setText(tr("PhoneInfo Setting"));
    QLabel *m_title = new QLabel;
    m_title->setText(tr("PhoneInfo Setting"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");

    QHBoxLayout *m_hlayout = new QHBoxLayout;
    m_hlayout->addWidget(modelLabel);
    m_hlayout->addSpacing(80);
    m_hlayout->addWidget(presetBtn);
    m_hlayout->addSpacing(60);
    m_hlayout->addWidget(customBtn);
    m_hlayout->addStretch();

    QVBoxLayout *presetlayout = new QVBoxLayout(presetFrame);
    presetlayout->setMargin(0);
    presetlayout->setSpacing(0);
    QHBoxLayout *brandlayout1 = new QHBoxLayout;
    brandlayout1->setMargin(0);
    brandlayout1->addWidget(Label3);
    brandlayout1->addSpacing(80);
    brandlayout1 ->addWidget(presetBox1);
    brandlayout1->addStretch();
    QHBoxLayout *modellayout1 = new QHBoxLayout;
    modellayout1->setMargin(0);
    modellayout1->addWidget(Label4);
    modellayout1->addSpacing(80);
    modellayout1->addWidget(presetBox2);
    modellayout1->addWidget(presetBox3);
    modellayout1->addWidget(presetBox4);
    modellayout1->addWidget(presetBox5);
    modellayout1->addWidget(presetBox6);
    modellayout1->addStretch();

    presetlayout->addLayout(brandlayout1);
    presetlayout->addSpacing(16);
    presetlayout->addLayout(modellayout1);

    QVBoxLayout *customhlayout = new QVBoxLayout(customFrame);
    customhlayout->setMargin(0);
    customhlayout->setSpacing(0);
    QHBoxLayout *brandlayout = new QHBoxLayout;
    brandlayout->setMargin(0);
    brandlayout->addWidget(Label1);
    brandlayout->addSpacing(80);
    brandlayout->addWidget(line1);
    brandlayout->addStretch();
    QHBoxLayout *modellayout = new QHBoxLayout;
    modellayout->setMargin(0);
    modellayout->addWidget(Label2);
    modellayout->addSpacing(80);
    modellayout->addWidget(line2);
    modellayout->addStretch();
//    QHBoxLayout *vendorlayout = new QHBoxLayout;
//    vendorlayout->addWidget(Label1);
//    vendorlayout->addSpacing(10);
//    vendorlayout->addWidget(line1);
//    vendorlayout->addStretch();
//    QHBoxLayout *namelayout = new QHBoxLayout;
//    namelayout->addWidget(Label3);
//    namelayout->addSpacing(10);
//    namelayout->addWidget(line3);
//    namelayout->addStretch();
//    QHBoxLayout *equiplayout = new QHBoxLayout;
//    equiplayout->addWidget(Label5);
//    equiplayout->addSpacing(10);
//    equiplayout->addWidget(line5);
//    equiplayout->addStretch();

    customhlayout->addLayout(brandlayout);
    customhlayout->addSpacing(16);
    customhlayout->addLayout(modellayout);
//    customhlayout->addLayout(vendorlayout);
//    customhlayout->addLayout(namelayout);  
//    customhlayout->addLayout(equiplayout);

    QHBoxLayout *imeilayout = new QHBoxLayout;
    imeilayout->addWidget(IMEILabel);
    imeilayout->addSpacing(80);
    imeilayout->addWidget(IMEIline, 0, Qt::AlignHCenter);
    imeilayout->addSpacing(16);
    imeilayout->addWidget(randBtn);
    imeilayout->addStretch();

//    QHBoxLayout *equiplayout = new QHBoxLayout;
//    equiplayout->addWidget(EquipLabel);
//    equiplayout->addSpacing(20);
//    equiplayout->addWidget(Equipline, 0, Qt::AlignHCenter);
//    equiplayout->addStretch();

    QLabel *tiplabel = new QLabel(tr("The Settings take effect after the environment is restarted"));
    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->setMargin(0);
    vlayout1->setSpacing(0);
    vlayout1->addSpacing(8);
    vlayout1->addWidget(tiplabel);
    vlayout1->addSpacing(8);
    vlayout1->addLayout(m_hlayout);
    vlayout1->addSpacing(8);
    vlayout1->addWidget(presetFrame);
//    vlayout1->addWidget(presetBox , 0,Qt::AlignHCenter);
    vlayout1->addWidget(customFrame);
    vlayout1->addSpacing(16);
    vlayout1->addLayout(imeilayout);
    vlayout1->addSpacing(10);

    centralWidget->setLayout(vlayout1);
    this->setContent(centralWidget);

    const QByteArray styleid("org.ukui.style");
    if (QGSettings::isSchemaInstalled(styleid)) {
        QGSettings *stylesettings = new QGSettings(styleid, QByteArray(), this);
        connect(stylesettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "systemFontSize" || key == "systemFont") {
                QFont ft;
                QFontMetrics fm(ft);
                QString elided_text = fm.elidedText(tr("Phone model"), Qt::ElideRight, 100);
                modelLabel->setText(elided_text);
                if (fm.width(tr("Phone model")) > modelLabel->width()) {
                    modelLabel->setToolTip(tr("Phone model"));
                }
            }
        });
    }
    connect(presetBtn, SIGNAL(toggled(bool)), this, SLOT(setpresetvisible(bool)));
    connect(customBtn, SIGNAL(toggled(bool)), this, SLOT(setcustomvisible(bool)));
    connect(presetBox1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PhoneInfoWidget::setpresetbrandInfo);
    connect(presetBox2, QOverload<int>::of(&QComboBox::activated), this, &PhoneInfoWidget::setpresetmodelInfo);
    connect(presetBox3, QOverload<int>::of(&QComboBox::activated), this, &PhoneInfoWidget::setpresetmodelInfo);
    connect(presetBox4, QOverload<int>::of(&QComboBox::activated), this, &PhoneInfoWidget::setpresetmodelInfo);
    connect(presetBox5, QOverload<int>::of(&QComboBox::activated), this, &PhoneInfoWidget::setpresetmodelInfo);
    connect(presetBox6, QOverload<int>::of(&QComboBox::activated), this, &PhoneInfoWidget::setpresetmodelInfo);
    connect(IMEIline, &QLineEdit::textChanged, this, &PhoneInfoWidget::setimeiInfo);
    connect(line1, &QLineEdit::textChanged, this, &PhoneInfoWidget::setcustombrand);
    connect(line2, &QLineEdit::textChanged, this, &PhoneInfoWidget::setcustommodel);
    connect(randBtn, &QPushButton::clicked, this, &PhoneInfoWidget::setrandimei);

}

void PhoneInfoWidget::setpresetvisible(bool checked)
{
    custom = !checked;
    presetFrame->setVisible(checked);
    presetBox1->setCurrentIndex(m_pref->PhonepresetbrandIndex);
    this->setpresetbrandInfo(m_pref->PhonepresetbrandIndex);
}

void PhoneInfoWidget::setcustomvisible(bool checked)
{
    custom = checked;
    customFrame->setVisible(checked);
    m_pref->m_PhoneInfoCustom = custom;
    m_pref->updatecustomConfig();
}

void PhoneInfoWidget::setimeiInfo()
{
    IMEI = IMEIline->text();
    m_pref->PhoneIMEI = IMEI;
    m_pref->updatePhoneImei();
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.imei", IMEI);
}

void PhoneInfoWidget::setcustombrand()
{
    Brand = line1->text();
    m_pref->PhoneName = "";
    m_pref->PhoneVendor = Brand;
    m_pref->PhoneBrand = Brand;
    m_pref->updatePhoneInfoConfig();
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.manufacturer", Brand);
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.brand", Brand);
}

void PhoneInfoWidget::setcustommodel()
{
    Model = line2->text();
    m_pref->PhoneName = "";
    m_pref->PhoneModel = Model;
    m_pref->updatePhoneInfoConfig();
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.model", Model);
}

void PhoneInfoWidget::setpresetbrandInfo(int index)
{
    m_pref->PhonepresetbrandIndex = index;
    m_pref->updatePhoneBrandIndex();
    if (index == 0) {
        presetBox2->setVisible(true);
        presetBox3->setVisible(false);
        presetBox4->setVisible(false);
        presetBox5->setVisible(false);
        presetBox6->setVisible(false);
    }
    else if (index == 1) {
        presetBox2->setVisible(false);
        presetBox3->setVisible(true);
        presetBox4->setVisible(false);
        presetBox5->setVisible(false);
        presetBox6->setVisible(false);
    }
    else if (index == 2) {
        presetBox2->setVisible(false);
        presetBox3->setVisible(false);
        presetBox4->setVisible(true);
        presetBox5->setVisible(false);
        presetBox6->setVisible(false);
    }
    else if (index == 3) {
        presetBox2->setVisible(false);
        presetBox3->setVisible(false);
        presetBox4->setVisible(false);
        presetBox5->setVisible(true);
        presetBox6->setVisible(false);
    }
    else if (index == 4) {
        presetBox2->setVisible(false);
        presetBox3->setVisible(false);
        presetBox4->setVisible(false);
        presetBox5->setVisible(false);
        presetBox6->setVisible(true);
    }
}

void PhoneInfoWidget::setpresetmodelInfo()
{
    QString model;
    int index = m_pref->PhonepresetbrandIndex;
    if (index == 0) {
        model =  presetBox2->currentText();
    }
    else if (index == 1) {
        model =  presetBox3->currentText();
    }
    else if (index == 2) {
        model =  presetBox4->currentText();
    }
    else if (index == 3) {
        model =  presetBox5->currentText();
    }
    else if (index == 4) {
        model =  presetBox6->currentText();
    }
    if (model == tr("default")) {
        Vendor = "";
        Brand = "";
        Name = "";
        Model = "";
        Equip = "";
        Serialno = "";
        Board = "";
    }
    if (model == "Galaxy S10") {
        Vendor = "samsung";
        Brand = "samsung";
        Name = "dream2qltezh";
        Model = "SM-G9550";
        Equip = "dream2qltezh";
        Serialno = "988913415958415431";
        Board = "msm8998";
    }
    if (model == "Mate 40") {
        Vendor = "HUAWEI";
        Brand = "HUAWEI";
        Name = "OCE-AN10";
        Model = "OCE-AN10";
        Equip = "HWOCE-L";
        Serialno = "XWN0221715012994";
        Board = "OCE";
    }
    if (model == "MI 2") {
        Vendor = "Xiaomi";
        Brand = "Xiaomi";
        Name = "aries";
        Model = "MI 2";
        Equip = "aries";
        Serialno = "988913415958415431";
        Board = "MSM8960";
    }
    if (model == "N928Dt") {
        Vendor = "ZTE";
        Brand = "ZTE";
        Name = "N928Dt";
        Model = "ZTE N928Dt";
        Equip = "N928Dt";
        Serialno = "ZT9251234568214";
        Board = "msm8909";
    }

    m_pref->m_PhoneInfoCustom = custom;
    m_pref->PhoneVendor = Vendor;
    m_pref->PhoneBrand = Brand;
    m_pref->PhoneName = Name;
    m_pref->PhoneModel = Model;
    m_pref->PhoneEquip = Equip;
    m_pref->PhoneSerialno = Serialno;
    m_pref->PhoneBoard = Board;
    m_pref->updatePhoneInfoConfig();

    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.manufacturer", Vendor);
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.brand", Brand);
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.product", Name);
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.model", Model);
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.device", Equip);
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.serialno", Serialno);
    m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.custom.board", Board);
    if (KylinUI::MessageBox::restartEnv(this)) {
        QDBusInterface interface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
        interface.call("StopContainer", kmre::utils::getUserName(), (int32_t)getuid());
        emit restartEnv();
    }
}

void PhoneInfoWidget::setrandimei()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    const char chrs[] = "0123456789";
    int chrs_size = sizeof(chrs);
    char *ch = new char[16];
    for(int i=0; i<15; i++)
    {
        int randomx= rand() % (chrs_size - 1);
        ch[i] = chrs[randomx];
    }
    QString randimei(ch);
    IMEIline->setText(randimei);
}
PhoneInfoWidget::~PhoneInfoWidget()
{
    {
        if (m_pref) {
            delete m_pref;
            m_pref = nullptr;
        }
    }
    if (m_systemBusInterface) {
        delete m_systemBusInterface;
        m_systemBusInterface = nullptr;
    }
}
