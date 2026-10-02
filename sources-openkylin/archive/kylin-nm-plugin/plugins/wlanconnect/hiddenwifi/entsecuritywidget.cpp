/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "entsecuritywidget.h"
#include "../checkedlineedit.h"
#include "../oobecheckstyle.h"

#include <QFileDialog>
#include <QListView>
#include <QScrollBar>

#define  LAYOUT_SPACING  16
#define  LABEL_MIN_WIDTH  150
#define  EDIT_MIN_WIDTH  274
#define  EDIT_FIX_HiGHT 36
#define  HORIZON_SPACING  8
#define  ROW_MIN_HEIGHT 36

EntSecurityWidget::EntSecurityWidget(bool isOobe, bool isLockScreen, QWidget *parent)
    : m_isOobe(isOobe), m_isLockScreen(isLockScreen), QWidget(parent)
{
    initUI();
    initConnect();
}

EntSecurityWidget::~EntSecurityWidget()
{
//    if (m_loginHintDialog != nullptr) {
//        delete m_loginHintDialog;
//    }
}

void EntSecurityWidget::getEnterpriseType(KyEapMethodType &enterpriseType)
{
    enterpriseType = (KyEapMethodType)m_eapTypeCombox->currentData().toInt();
}

void EntSecurityWidget::setPreFillInfo(const QVariantMap &info)
{
    // 设置EAP类型
    QStringList eapList = info.value("eap").toStringList();
    if (eapList.isEmpty()) {
        qWarning() << "未找到EAP类型";
        return;
    }

    QString type = eapList.first().toUpper(); // 通常只有一种
    m_eapTypeCombox->setCurrentText(type);
    // 设置用户名和密码（根据当前EAP类型）
    QString identity = info.value("identity").toString();
    QString password = info.value("password").toString();

    if (type == "PEAP" || type == "TTLS" || type == "LEAP" || type == "PWD") {
        m_userNameEdit->setText(identity);
        m_userPwdEdit->setText(password);
    } else if (type == "TLS") {
        m_identityEdit->setText(identity);
        m_clientPrivateKeyPwdEdit->setText(password);
    } else if (type == "FAST") {
        m_identityEdit->setText(identity);
        m_userNameEdit->setText(identity); // FAST 可能需要匿名身份和用户名，这里简化
        m_userPwdEdit->setText(password);
    }
}

void EntSecurityWidget::initUI()
{
    //企业wifi共有
    m_eapTypeLabel = new QLabel(this);
    m_eapTypeCombox = new QComboBox(this);
    m_eapTypeCombox->setFixedHeight(ROW_MIN_HEIGHT);

    //TLS
    m_identityLable = new QLabel(this);
    m_domainLable = new QLabel(this);
    m_caCertPathLabel = new QLabel(this);
    m_caNeedFlagLabel = new QLabel(this);
    m_clientCertPathLabel = new FixLabel(this);
    m_clientPrivateKeyLabel = new FixLabel(this);
    m_clientPrivateKeyPwdLabel = new FixLabel(this);
    m_pwdOptionLabel = new FixLabel(this);
    //TLS
    m_identityEdit = new LineEdit(this);
    m_domainEdit = new LineEdit(this);
    m_caCertPathCombox = new QComboBox(this);
    m_caNeedBox = new QCheckBox(this);

    m_identityEdit->setFixedHeight(EDIT_FIX_HiGHT);
    m_domainEdit->setFixedHeight(EDIT_FIX_HiGHT);
    m_caCertPathCombox->setFixedHeight(ROW_MIN_HEIGHT);

    if(m_isOobe){
        m_caNeedBox->setProperty("oobe", true);
        m_caNeedBox->setStyle(new OobeCheckStyle(m_caNeedBox->style()));
        m_clientPrivateKeyPwdEdit = new CheckedLineEdit(this);
        static_cast<CheckedLineEdit *>(m_clientPrivateKeyPwdEdit)->setInputBackgroundTransparent(false);
    }else{
        m_clientPrivateKeyPwdEdit = new KPasswordEdit(this);     
    }
    m_clientCertPathCombox = new QComboBox(this);
    m_clientCertPathCombox->setFixedHeight(ROW_MIN_HEIGHT);
    m_clientPrivateKeyCombox = new QComboBox(this);
    m_clientPrivateKeyCombox->setFixedHeight(ROW_MIN_HEIGHT);

    m_pwdOptionCombox = new QComboBox(this);
    m_pwdOptionCombox->setFixedHeight(ROW_MIN_HEIGHT);

//    m_loginHintDialog = new LogHintDialog(this->parentWidget());

    //PEAP TTLS共有
    m_eapMethodLabel = new FixLabel(this);
    m_userNameLabel = new QLabel(this);
    m_userPwdLabel = new QLabel(this);
    m_eapMethodCombox = new QComboBox(this);
    m_userNameEdit = new LineEdit(this);

    m_userNameEdit->setFixedHeight(EDIT_FIX_HiGHT);
    m_eapMethodCombox->setFixedHeight(ROW_MIN_HEIGHT);

    if(m_isOobe){
        m_userPwdEdit = new CheckedLineEdit(this);
        static_cast<CheckedLineEdit *>(m_userPwdEdit)->setInputBackgroundTransparent(false);
    }else{
        m_userPwdEdit = new KPasswordEdit(this);     
    }

    //FAST
    m_pacProvisionComboBox = new QComboBox(this);
    m_pacProvisionComboBox->setFixedHeight(ROW_MIN_HEIGHT);
    m_pacFilePathComboBox = new QComboBox(this);
    m_pacFilePathComboBox->setFixedHeight(ROW_MIN_HEIGHT);
    m_pacProvisionLabel = new FixLabel(this);
    m_pacCheckBox = new QCheckBox(this);
    m_pacFileLabel = new QLabel(this);
    m_pacProvisionLabel->setFixedWidth(LABEL_MIN_WIDTH);

    //允许自动PAC配置复选框
    m_pacCheckWidget = new QWidget(this);
    m_pacCheckWidget->setMinimumHeight(72);
    QVBoxLayout *pacCheckLayout = new QVBoxLayout(m_pacCheckWidget);
    pacCheckLayout->setContentsMargins(0, 0, 0, 0);
    pacCheckLayout->setSpacing(0);
    pacCheckLayout->addWidget(m_pacProvisionComboBox);
    pacCheckLayout->addWidget(m_pacCheckBox);

    //main layout
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 16, 0, 0);
    mainLayout->setColumnMinimumWidth(0, LABEL_MIN_WIDTH);
    mainLayout->setColumnMinimumWidth(1, EDIT_MIN_WIDTH);
    mainLayout->setVerticalSpacing(LAYOUT_SPACING);
    // EAP认证 Label和选项框 第0行，第0列，第1列
    mainLayout->addWidget(m_eapTypeLabel, 0, 0);
    mainLayout->addWidget(m_eapTypeCombox, 0, 1);
    // 匿名身份 Label和输入框 第1行，第0列，第1列
    mainLayout->addWidget(m_identityLable, 1, 0);
    mainLayout->addWidget(m_identityEdit, 1, 1);
    //PAC配置 允许自动PAC配置 第2行，第0列；第2行,第1列，占2行1列
    mainLayout->addWidget(m_pacProvisionLabel, 2, 0, Qt::AlignTop);
    mainLayout->addWidget(m_pacCheckWidget, 2, 1);
    // PAC文件
    mainLayout->addWidget(m_pacFileLabel, 4, 0);
    mainLayout->addWidget(m_pacFilePathComboBox, 4, 1);

    //内部认证 Label和选项框 第5行，第0列，第1列
    mainLayout->addWidget(m_eapMethodLabel, 5, 0);
    mainLayout->addWidget(m_eapMethodCombox, 5, 1);
    //用户名 Label和输入框 第6行，第0列，第1列
    mainLayout->addWidget(m_userNameLabel, 6, 0);
    mainLayout->addWidget(m_userNameEdit, 6, 1);
    //密码 Label和密码框 第7行，第0列，第1列
    mainLayout->addWidget(m_userPwdLabel, 7, 0);
    mainLayout->addWidget(m_userPwdEdit, 7, 1);

    m_tlsWidget = new QWidget(this);
    QGridLayout *bottomLayout = new QGridLayout(m_tlsWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setColumnMinimumWidth(0, LABEL_MIN_WIDTH);
    bottomLayout->setVerticalSpacing(8);
    // 域 Label和输入框 第0行，第0列，第1列
    bottomLayout->addWidget(m_domainLable, 0, 0);
    bottomLayout->addWidget(m_domainEdit, 0, 1);

    // CA证书选项框及CheckBox布局
    QWidget *CaWidget = new QWidget(this);
    QGridLayout *checkLayout = new QGridLayout(CaWidget);
    checkLayout->setContentsMargins(0, 0, 0, 0);
    checkLayout->setVerticalSpacing(0);
    checkLayout->setColumnMinimumWidth(0, 16);
    checkLayout->addWidget(m_caCertPathCombox, 0, 0, 1, 2);
    checkLayout->addWidget(m_caNeedBox, 1, 0);
    checkLayout->addWidget(m_caNeedFlagLabel, 1, 1);

    // CA证书 Label第1行，第0列
    bottomLayout->addWidget(m_caCertPathLabel, 1, 0);
    // CA证书选项框 不需要CA证书复选框 从第1行，第1列开始，占2行1列
    bottomLayout->addWidget(CaWidget, 1, 1, 2, 1);

    // 用户证书 Label和选项框 第3行，第0列，第1列
    bottomLayout->addWidget(m_clientCertPathLabel, 3, 0);
    bottomLayout->addWidget(m_clientCertPathCombox, 3, 1);
    // 用户私钥 Label和选项框 第4行，第0列，第1列
    bottomLayout->addWidget(m_clientPrivateKeyLabel, 4, 0);
    bottomLayout->addWidget(m_clientPrivateKeyCombox, 4, 1);
    // 私钥密码 Label和密码框 第5行，第0列，第1列
    bottomLayout->addWidget(m_clientPrivateKeyPwdLabel, 5, 0);
    bottomLayout->addWidget(m_clientPrivateKeyPwdEdit, 5, 1);
    // 密码选项 Label和选项框 第6行，第0列，第1列
    bottomLayout->addWidget(m_pwdOptionLabel, 6, 0);
    bottomLayout->addWidget(m_pwdOptionCombox, 6, 1);

    mainLayout->addWidget(m_tlsWidget, 8, 0, 6,2);

    m_eapMethodLabel->setFixedWidth(LABEL_MIN_WIDTH);
    m_clientCertPathLabel->setFixedWidth(LABEL_MIN_WIDTH);
    m_clientPrivateKeyLabel->setFixedWidth(LABEL_MIN_WIDTH);
    m_clientPrivateKeyPwdLabel->setFixedWidth(LABEL_MIN_WIDTH);
    m_pwdOptionLabel->setFixedWidth(LABEL_MIN_WIDTH);

    //企业wifi共有
    m_eapTypeLabel->setText(tr("EAP type"));
    //TLS
    m_identityLable->setText(tr("Identity"));
    m_domainLable->setText(tr("Domain"));
    m_caCertPathLabel->setText(tr("CA certficate"));
    m_caNeedFlagLabel->setText(tr("no need for CA certificate"));
    m_clientCertPathLabel->setLabelText(tr("User certificate"));
    m_clientPrivateKeyLabel->setLabelText(tr("User private key"));
    m_clientPrivateKeyPwdLabel->setLabelText(tr("User key password"));
    m_pwdOptionLabel->setLabelText(tr("Password options"));

    QString hint = tr("Required");
    m_identityEdit->setPlaceholderText(tr("Required")); //必填
    m_clientPrivateKeyPwdEdit->setPlaceholderText(hint);
    m_clientPrivateKeyPwdEdit->setUseCustomPalette(true);//一定要设置这个不然点击就变回主题颜色了
    m_userPwdEdit->setUseCustomPalette(true);
    if (m_isOobe) {
        this->setStyleSheet(
            "QWidget { background-color: transparent; color: #FFFFFF; }"
            "QLabel { color: #FFFFFF; }"
            "QComboBox { background-color: #3a3a3a; color: #FFFFFF; border: 1px solid rgba(255,255,255,20); border-radius:4px; height:26px}"
            "QComboBox QAbstractItemView { background-color: #3a3a3a; color: #FFFFFF; selection-background-color: #3790FA; border: 1px solid rgba(255,255,255,20); }"
            "QLineEdit, QPlainTextEdit, QSpinBox { background-color: #3a3a3a; color: #FFFFFF; border: 1px solid rgba(255,255,255,20); border-radius:4px; }"
            "KPasswordEdit { background-color: #3a3a3a; color: #FFFFFF; }");

        QPalette editPal;
        editPal.setColor(QPalette::Base, QColor("#3a3a3a"));
        editPal.setColor(QPalette::Text, Qt::white);
        editPal.setColor(QPalette::PlaceholderText, QColor(200,200,200));

        // Ensure password edits and text edits use the darker palette
        m_clientPrivateKeyPwdEdit->setUseCustomPalette(true);
        m_clientPrivateKeyPwdEdit->setPalette(editPal);
        m_userPwdEdit->setUseCustomPalette(true);
        m_userPwdEdit->setPalette(editPal);
        m_identityEdit->setPalette(editPal);
        m_domainEdit->setPalette(editPal);
        m_userNameEdit->setPalette(editPal);

        // 对所有下拉框设置popup view，确保展开时有深色不透明背景
        QList<QComboBox*> comboList = {
            m_eapTypeCombox, m_caCertPathCombox, m_clientCertPathCombox,
            m_clientPrivateKeyCombox, m_pwdOptionCombox, m_eapMethodCombox,
            m_pacProvisionComboBox, m_pacFilePathComboBox
        };
        for (QComboBox *cb : comboList) {
            QListView *listView = new QListView();
            listView->setAutoFillBackground(true);
            QPalette cbPal = listView->palette();
            cbPal.setColor(QPalette::Base, QColor("#3a3a3a"));
            cbPal.setColor(QPalette::Text, Qt::white);
            cbPal.setColor(QPalette::Highlight, QColor("#3790FA"));
            cbPal.setColor(QPalette::HighlightedText, Qt::white);
            listView->setPalette(cbPal);
            cb->setView(listView);
            cb->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
            cb->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
        }

        m_clientPrivateKeyPwdEdit->setEnabled(true);
        m_userPwdEdit->setEnabled(true);
    } else {
        QPalette pal = this->palette();
        pal.setColor(QPalette::ButtonText, Qt::black);
        m_clientPrivateKeyPwdEdit->setPalette(pal);
        m_userPwdEdit->setPalette(pal);
        m_clientPrivateKeyPwdEdit->setEnabled(true);
        m_userPwdEdit->setEnabled(true);
    }
    //PEAP TTLS共有
    m_eapMethodLabel->setLabelText(tr("Ineer authentication"));
    m_userNameLabel->setText(tr("Username"));
    m_userPwdLabel->setText(tr("Password"));

    m_userNameEdit->setPlaceholderText(tr("Required"));
    m_userPwdEdit->setPlaceholderText(hint);

    m_eapTypeCombox->addItem("TLS", TLS);
    m_eapTypeCombox->addItem("PEAP", PEAP);
    m_eapTypeCombox->addItem("TTLS", TTLS);
    m_eapTypeCombox->addItem("LEAP", LEAP);
    m_eapTypeCombox->addItem("PWD", PWD);
    m_eapTypeCombox->addItem("FAST", FAST);
    m_eapTypeCombox->setCurrentIndex(TLS);

    //TLS
    m_caCertPathCombox->addItem(tr("None"), QString(tr("None"))); //无
    if (!m_isLockScreen)
        m_caCertPathCombox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...
    else
        m_caCertPathCombox->addItem(tr("Please log in to the system first."), QString(tr("Please log in to the system first.")));

    m_clientCertPathCombox->addItem(tr("None"), QString(tr("None"))); //无
    if (!m_isLockScreen)
        m_clientCertPathCombox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...
    else
        m_clientCertPathCombox->addItem(tr("Please log in to the system first."), QString(tr("Please log in to the system first.")));

    m_clientPrivateKeyCombox->addItem(tr("None"), QString(tr("None"))); //无
    if (!m_isLockScreen)
        m_clientPrivateKeyCombox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...
    else
        m_clientPrivateKeyCombox->addItem(tr("Please log in to the system first."), QString(tr("Please log in to the system first.")));

    m_pwdOptionCombox->addItem(tr("Store passwords only for this user"), QString(tr("Store passwords only for this user"))); //仅为该用户存储密码
    m_pwdOptionCombox->addItem(tr("Store passwords for all users"), QString(tr("Store passwords for all users"))); //存储所有用户的密码
    m_pwdOptionCombox->addItem(tr("Ask this password every time"), QString(tr("Ask this password every time"))); //每次询问这个密码

    //FAST
    m_pacProvisionLabel->setLabelText(tr("PAC provisioning")); //PAC配置
    m_pacCheckBox->setChecked(true);
    QString pacFlagText = tr("Allow automatic PAC provisioning"); //允许自动PAC配置
    QFontMetrics  fontMetrics(this->font());
    int fontSize = fontMetrics.width(pacFlagText);
    if (fontSize > 240) {
        //最大宽度240像素
        QString elideNote = fontMetrics.elidedText(pacFlagText, Qt::ElideRight, 240);
        m_pacCheckBox->setText(elideNote);
        m_pacCheckBox->setToolTip(tr("Allow automatic PAC provisioning"));
    } else {
        m_pacCheckBox->setText(pacFlagText);
    }
    m_pacFileLabel->setText(tr("PAC file")); //PAC文件
    m_pacProvisionComboBox->addItem(tr("Anonymous"), ANON); //匿名
    m_pacProvisionComboBox->addItem(tr("Authenticated"), AUTHEN); //已认证
    m_pacProvisionComboBox->addItem(tr("Both"), BOTH); //两者兼用
    m_pacProvisionComboBox->setCurrentIndex(0);
    m_pacFilePathComboBox->addItem(tr("None"), QString(tr("None"))); //无
    if (!m_isLockScreen)
        m_pacFilePathComboBox->addItem(tr("Choose from file..."), QString(tr("Choose from file..."))); //从文件中选择...
    else
        m_pacFilePathComboBox->addItem(tr("Please log in to the system first."), QString(tr("Please log in to the system first.")));

    QRegExp rx("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
    QRegExpValidator *latitude = new QRegExpValidator(rx, this);
    m_clientPrivateKeyPwdEdit->setValidator(latitude);
    m_userPwdEdit->setValidator(latitude);
    //禁用ClearBtn按钮
    m_clientPrivateKeyPwdEdit->setClearButtonEnabled(false);
    m_userPwdEdit->setClearButtonEnabled(false);
    showTls();
}

void EntSecurityWidget::initConnect()
{
    //EAP方式变化
    connect(m_eapTypeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EntSecurityWidget::onEapTypeComboxIndexChanged);

    connect(m_caNeedBox, &QCheckBox::clicked, this, &EntSecurityWidget::onCaNeedBoxClicked);

    connect(m_caCertPathCombox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &EntSecurityWidget::onCaCertPathComboxIndexChanged);

    connect(m_clientCertPathCombox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &EntSecurityWidget::onClientCertPathComboxIndexChanged);

    connect(m_clientPrivateKeyCombox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &EntSecurityWidget::onClientPrivateKeyComboxIndexChanged);

    connect(m_eapTypeCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_identityEdit, &LineEdit::textChanged, this, &EntSecurityWidget::setEnableOfSaveBtn);
    connect(m_caCertPathCombox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_caNeedBox, &QCheckBox::stateChanged, this, &EntSecurityWidget::setEnableOfSaveBtn);
    connect(m_clientCertPathCombox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_clientPrivateKeyCombox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_clientPrivateKeyPwdEdit, &LineEdit::textChanged, this, &EntSecurityWidget::setEnableOfSaveBtn);
    connect(m_eapMethodCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_userNameEdit, &LineEdit::textChanged, this, &EntSecurityWidget::setEnableOfSaveBtn);
    connect(m_userPwdEdit, &LineEdit::textChanged, this, &EntSecurityWidget::setEnableOfSaveBtn);

//    connect(m_loginHintDialog, &LogHintDialog::LogHintDialogClosed, this, &EntSecurityWidget::setComboxIndex);

    connect(m_pacCheckBox, &QCheckBox::stateChanged, this, &EntSecurityWidget::setEnableOfSaveBtn);
    connect(m_pacProvisionComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_pacFilePathComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(setEnableOfSaveBtn()));

    connect(m_pacCheckBox, &QCheckBox::clicked, this, &EntSecurityWidget::onPacBoxClicked);
    connect(m_pacFilePathComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &EntSecurityWidget::onPacFilePathComboxIndexChanged);

}

void EntSecurityWidget::showTls()
{
    //TLS
    m_identityLable->show();
    m_identityEdit->show();
    m_tlsWidget->show();

    //PEAP TTLS共有
    m_eapMethodLabel->hide();
    m_userNameLabel->hide();
    m_userPwdLabel->hide();

    m_eapMethodCombox->hide();
    m_userNameEdit->hide();
    m_userPwdEdit->hide();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();
}

void EntSecurityWidget::showPeapOrTtls()
{
    //TLS
    m_identityLable->hide();
    m_identityEdit->hide();
    m_tlsWidget->hide();

    //PEAP TTLS共有
    m_eapMethodLabel->show();
    m_userNameLabel->show();
    m_userPwdLabel->show();

    m_eapMethodCombox->show();
    m_userNameEdit->show();
    m_userPwdEdit->show();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();
}

void EntSecurityWidget::showLeapOrPwd()
{
    //TLS
    m_identityLable->hide();
    m_identityEdit->hide();
    m_tlsWidget->hide();

    //PEAP TTLS共有
    m_eapMethodLabel->hide();
    m_userNameLabel->show();
    m_userPwdLabel->show();
    m_eapMethodCombox->hide();
    m_userNameEdit->show();
    m_userPwdEdit->show();

    //FAST
    m_pacProvisionLabel->hide();
    m_pacCheckWidget->hide();
    m_pacFileLabel->hide();
    m_pacFilePathComboBox->hide();
}

void EntSecurityWidget::showFast()
{
    //TLS
    m_identityLable->show();
    m_identityEdit->show();
    m_tlsWidget->hide();

    //PEAP TTLS共有
    m_eapMethodLabel->show();
    m_userNameLabel->show();
    m_userPwdLabel->show();
    m_eapMethodCombox->show();
    m_userNameEdit->show();
    m_userPwdEdit->show();

    //FAST
    m_pacProvisionLabel->show();
    m_pacCheckWidget->show();
    m_pacFileLabel->show();
    m_pacFilePathComboBox->show();
}

bool EntSecurityWidget::checkConnectBtnIsEnabled()
{
    int type = m_eapTypeCombox->currentData().toInt();
    if (type == TLS) {
        return false;
        if (m_identityEdit->text().isEmpty()) {
            qDebug() << "tls identity is empty";
            return false;
        }
        QFile cafile(m_caCertPathCombox->currentText());
        if(!m_caNeedBox->isChecked() && !cafile.exists()) {
            qDebug() << "ca cert filepath " << m_caCertPathCombox->currentText() << " is invalid";
            return false;
        }

        QFile cliCafile(m_clientCertPathCombox->currentText());
        if(!cliCafile.exists()) {
            qDebug() << "client cert filepath " << m_clientCertPathCombox->currentText() << " is invalid";
            return false;
        }

        QFile cliKeyfile(m_clientPrivateKeyCombox->currentText());
        if(!cliKeyfile.exists()) {
            qDebug() << "client private key filepath " << m_clientPrivateKeyCombox->currentText() << " is invalid";
            return false;
        }

        if(m_clientPrivateKeyPwdEdit->text().isEmpty()) {
            qDebug() << "client Private Key password is empty";
            return false;
        }
    } else if (type == PEAP || type == TTLS || type == LEAP || type == PWD) {
        if(m_userNameEdit->text().isEmpty() || m_userPwdEdit->text().isEmpty()) {
            qDebug() << "user name or user password is empty";
            return false;
        }
    } else if (type == FAST) {
        if (m_identityEdit->text().isEmpty()) {
            qDebug() << "fast anonymous identity is empty";
            return false;
        }
        if(!m_pacCheckBox->isChecked()) {
            qDebug() << "Not allow automatic PAC provisioning ";
            return false;
        }
        if(m_userNameEdit->text().isEmpty() || m_userPwdEdit->text().isEmpty()) {
            qDebug() << "user name or user password is empty";
            return false;
        }
    }
    return true;
}

void EntSecurityWidget::onEapTypeComboxIndexChanged()
{
    qDebug() << "onEapTypeComboxIndexChanged";
    int index = m_eapTypeCombox->currentData().toInt();
    if (index == TLS) {
        showTls();
        Q_EMIT this->eapTypeChanged(TLS);
    } else if (index == PEAP) {
        showPeapOrTtls();
        m_eapMethodCombox->clear();
        m_eapMethodCombox->addItem("MSCHAPv2", MSCHAPV2_PEAP);
        m_eapMethodCombox->addItem("MD5", MD5_PEAP);
        m_eapMethodCombox->addItem("GTC", GTC_PEAP);
        Q_EMIT this->eapTypeChanged(PEAP);
    } else if (index == TTLS) {
        showPeapOrTtls();
        m_eapMethodCombox->clear();
        m_eapMethodCombox->addItem("pap", PAP);
        m_eapMethodCombox->addItem("mschap", MSCHAP);
        m_eapMethodCombox->addItem("mschapv2(eap)", MSCHAPV2_EAP);
        m_eapMethodCombox->addItem("mschapv2", MSCHAPV2);
        m_eapMethodCombox->addItem("chap", CHAP);
        m_eapMethodCombox->addItem("md5(eap)", MD5_EAP);
        m_eapMethodCombox->addItem("gtc(eap)", GTC_EAP);
        Q_EMIT this->eapTypeChanged(TTLS);
    } else if (index == LEAP) {
        showLeapOrPwd();
        Q_EMIT this->eapTypeChanged(LEAP);
    }  else if (index == PWD) {
        showLeapOrPwd();
        Q_EMIT this->eapTypeChanged(PWD);
    } else if (index == FAST) {
        showFast();
        m_eapMethodCombox->clear();
        m_eapMethodCombox->addItem("GTC", GTC_FAST);
        m_eapMethodCombox->addItem("MSCHAPv2", MSCHAPV2_FAST);
        Q_EMIT this->eapTypeChanged(FAST);
    }

}

void EntSecurityWidget::setEnableOfSaveBtn()
{
    Q_EMIT setSecuPageState(checkConnectBtnIsEnabled());
}

void EntSecurityWidget::onCaNeedBoxClicked()
{
    if (m_caNeedBox->isChecked()) {
        m_caCertPathCombox->setItemText(0, tr("None"));
        m_caCertPathCombox->setEnabled(false);
    }  else {
        m_caCertPathCombox->setEnabled(true);
    }
}

void EntSecurityWidget::onPacBoxClicked()
{
    if (!m_pacCheckBox->isChecked()) {
        m_pacProvisionComboBox->setCurrentIndex(ANON);
        m_pacProvisionComboBox->setEnabled(false);
    }  else {
        m_pacProvisionComboBox->setEnabled(true);
    }
}

void EntSecurityWidget::onCaCertPathComboxIndexChanged(QString str)
{
    if (m_isLockScreen) {
        if (m_caCertPathCombox->isEnabled()) {
//            m_loginHintDialog->show();
        }
    } else {
        if (str.contains("Choose from file...") || str.contains("从文件选择..."))
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a CA certificate"), "recent:///",
                                                            tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
            if (!fileName.isNull()) {
                QStringList nameList = fileName.split("/");
                m_caCertPathCombox->blockSignals(true);
                m_caCertPathCombox->setItemText(0, fileName);
                m_caCertPathCombox->setCurrentIndex(0);
                m_caCertPathCombox->blockSignals(false);
            } else {
                m_caCertPathCombox->blockSignals(true);
                m_caCertPathCombox->setItemText(0, tr("None"));
                m_caCertPathCombox->setCurrentIndex(0);
                m_caCertPathCombox->blockSignals(false);
            }
        } else {
            qWarning() << "Choose file is null or unvalible";
        }
    }
}

void EntSecurityWidget::onClientCertPathComboxIndexChanged(QString str)
{
    if (m_isLockScreen) {
        if (m_clientCertPathCombox->isEnabled()) {
//            m_loginHintDialog->show();
        }
    } else {
        if (str.contains("Choose from file...") || str.contains("从文件选择..."))
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a CA certificate"), "recent:///",
                                                            tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
            if (!fileName.isNull()) {
                m_clientCertPathCombox->blockSignals(true);
                m_clientCertPathCombox->setItemText(0, fileName);
                m_clientCertPathCombox->setCurrentIndex(0);
                m_clientCertPathCombox->blockSignals(false);
            } else {
                m_clientCertPathCombox->blockSignals(true);
                m_clientCertPathCombox->setItemText(0, tr("None"));
                m_clientCertPathCombox->setCurrentIndex(0);
                m_clientCertPathCombox->blockSignals(false);
            }
        } else {
            qWarning() << "Choose file is null or unvalible";
        }
    }
}

void EntSecurityWidget::onClientPrivateKeyComboxIndexChanged(QString str)
{
    if (m_isLockScreen) {
        if (m_clientPrivateKeyCombox->isEnabled()) {
//            m_loginHintDialog->show();
        }
    } else {
        if (str.contains("Choose from file...") || str.contains("从文件选择..."))
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a CA certificate"), "recent:///",
                                                            tr("CA Files (*.pem *.der *.p12 *.crt *.cer *.pfx)"));
            if (!fileName.isNull()) {
                QStringList nameList = fileName.split("/");
                m_clientPrivateKeyCombox->blockSignals(true);
                m_clientPrivateKeyCombox->setItemText(0, fileName);
                m_clientPrivateKeyCombox->setCurrentIndex(0);
                m_clientPrivateKeyCombox->blockSignals(false);
            } else {
                m_clientPrivateKeyCombox->blockSignals(true);
                m_clientPrivateKeyCombox->setItemText(0, tr("None"));
                m_clientPrivateKeyCombox->setCurrentIndex(0);
                m_clientPrivateKeyCombox->blockSignals(false);
            }
        } else {
            qWarning() << "Choose file is null or unvalible";
        }
    }
}

void EntSecurityWidget::onPacFilePathComboxIndexChanged(QString str)
{
    if (m_isLockScreen) {
        return;
    } else {
        if (str.contains("Choose from file...") || str.contains("从文件选择..."))
        {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a PAC file"), "recent:///",
                                                            tr("PAC Files (*.pac)"));
            if (!fileName.isNull()) {
                QStringList nameList = fileName.split("/");
                m_pacFilePathComboBox->blockSignals(true);
                m_pacFilePathComboBox->setItemText(0, fileName);
                m_pacFilePathComboBox->setCurrentIndex(0);
                m_pacFilePathComboBox->blockSignals(false);
            } else {
                m_pacFilePathComboBox->blockSignals(true);
                m_pacFilePathComboBox->setItemText(0, tr("None"));
                m_pacFilePathComboBox->setCurrentIndex(0);
                m_pacFilePathComboBox->blockSignals(false);
            }
        } else {
            qWarning() << "Choose file is null or unvalible";
        }
    }
}

void EntSecurityWidget::setComboxIndex()
{
    if (m_caCertPathCombox->isEnabled()) {
        m_caCertPathCombox->setCurrentIndex(0);
    }
    if (m_clientCertPathCombox->isEnabled()) {
        m_clientCertPathCombox->setCurrentIndex(0);
    }
    if (m_clientPrivateKeyCombox->isEnabled()) {
        m_clientPrivateKeyCombox->setCurrentIndex(0);
    }
}

KyEapMethodPeapInfo EntSecurityWidget::assemblePeapInfo()
{
    KyEapMethodPeapInfo info;
    switch (m_eapMethodCombox->currentIndex()) {
    case 0:
        info.phase2AuthMethod = KyAuthMethodMschapv2;
        break;
    case 1:
        info.phase2AuthMethod = KyAuthMethodMd5;
        break;
    case 2:
        info.phase2AuthMethod = KyAuthMethodGtc;
        break;
    default:
        break;
    }
    info.userName = m_userNameEdit->text();
    info.userPWD = m_userPwdEdit->text();
    info.m_passwdFlag = NetworkManager::Setting::None;
//    switch (m_pwdOptionCombox->currentIndex()) {
//        case 0:
//            info.m_passwdFlag = NetworkManager::Setting::AgentOwned;
//            info.userPWD = m_userPwdEdit->text();
//        break;
//        case 1:
//            info.m_passwdFlag = NetworkManager::Setting::None;
//            info.userPWD = m_userPwdEdit->text();
//        break;
//        case 2:
//            info.m_passwdFlag = NetworkManager::Setting::NotSaved;
//        break;
//        default:
//        break;
//    }

    return info;
}
KyEapMethodTtlsInfo EntSecurityWidget::assembleTtlsInfo()
{
    KyEapMethodTtlsInfo info;
    switch (m_eapMethodCombox->currentIndex()) {
    case PAP:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodPap;
        break;
    case MSCHAP:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodChap;
        break;
    case MSCHAPV2_EAP:
        info.authType = AUTH_EAP;
        info.authEapMethod = KyAuthEapMethodMschapv2;
        break;
    case MSCHAPV2:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodMschapv2;
        break;
    case CHAP:
        info.authType = AUTH_NO_EAP;
        info.authNoEapMethod = KyAuthMethodChap;
        break;
    case MD5_EAP:
        info.authType = AUTH_EAP;
        info.authEapMethod = KyAuthEapMethodMd5;
        break;
    case GTC_EAP:
        info.authType = AUTH_EAP;
        info.authEapMethod = KyAuthEapMethodGtc;
        break;
    default:
        break;
    }
    info.userName = m_userNameEdit->text();
    info.m_passwdFlag = NetworkManager::Setting::None;
    info.userPWD = m_userPwdEdit->text();
//    switch (m_pwdOptionCombox->currentIndex()) {
//        case 0:
//            info.m_passwdFlag = NetworkManager::Setting::AgentOwned;
//            info.userPWD = m_userPwdEdit->text();
//        break;
//        case 1:
//            info.m_passwdFlag = NetworkManager::Setting::None;
//            info.userPWD = m_userPwdEdit->text();
//        break;
//        case 2:
//            info.m_passwdFlag = NetworkManager::Setting::NotSaved;
//        break;
//        default:
//        break;
//    }
    return info;
}

KyEapMethodLeapInfo EntSecurityWidget::assembleLeapInfo()
{
    KyEapMethodLeapInfo info;
    info.m_userName = m_userNameEdit->text();
    info.m_userPwd = m_userPwdEdit->text();
    info.m_passwdFlag = NetworkManager::Setting::None;
    return info;
}

KyEapMethodPwdInfo EntSecurityWidget::assemblePwdInfo()
{
    KyEapMethodPwdInfo info;
    info.m_userName = m_userNameEdit->text();
    info.m_userPwd = m_userPwdEdit->text();
    info.m_passwdFlag = NetworkManager::Setting::None;
    return info;
}

KyEapMethodFastInfo EntSecurityWidget::assembleFastInfo()
{
    KyEapMethodFastInfo info;
    info.m_anonIdentity = m_identityEdit->text();
    switch (m_pacProvisionComboBox->currentIndex()) {
    case ANON:
        info.m_pacProvisioning = KyFastProvisioningAllowUnauthenticated;
        break;
    case AUTHEN:
        info.m_pacProvisioning = KyFastProvisioningAllowAuthenticated;
        break;
    case BOTH:
        info.m_pacProvisioning = KyFastProvisioningAllowBoth;
        break;
    default:
        break;
    }
    info.m_allowAutoPacFlag = m_pacCheckBox->isChecked();
    info.m_pacFilePath = m_pacFilePathComboBox->currentText();

    switch (m_eapMethodCombox->currentIndex()) {
    case GTC_FAST:
        info.m_authMethod = KyAuthMethodGtc;
        break;
    case MSCHAPV2_FAST:
        info.m_authMethod = KyAuthMethodMschapv2;
        break;
    default:
        break;
    }
    info.m_userName = m_userNameEdit->text();
    info.m_userPwd = m_userPwdEdit->text();
    info.m_passwdFlag = NetworkManager::Setting::None;
    return info;
}



#define ICON_SIZE 16,16

LogHintDialog::LogHintDialog(QWidget *parent) : KDialog(parent)
{
    //弹窗 TLS添加本地文件 登录系统提示
    m_iconLabel = new QLabel(this);
    m_contentLabel = new QLabel(this);
    m_confirmBtn = new QPushButton(this);
    m_dialogLayout = new QVBoxLayout(this);
    m_dialogLayout->setContentsMargins(20, 0, 24, 23);
    m_dialogLayout->setSpacing(0);
    this->setFixedSize(420, 168);
    this->closeButton();

    QIcon m_icon = QIcon::fromTheme("dialog-warning");
    m_iconLabel->setPixmap(m_icon.pixmap(ICON_SIZE));
    m_contentLabel->setText(tr("Please log in to the system first."));
    m_confirmBtn->setText(tr("Confirm"));

    QWidget *tipWidget = new QWidget(this);
    QHBoxLayout *tipLayout = new QHBoxLayout(tipWidget);
    tipLayout->addWidget(m_iconLabel);
    tipLayout->addWidget(m_contentLabel);
    tipLayout->addStretch();

    QWidget *btnWidget = new QWidget(this);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmBtn);

    m_dialogLayout->addWidget(tipWidget);
    m_dialogLayout->addStretch();
    m_dialogLayout->addWidget(btnWidget);
    this->mainWidget()->setLayout(m_dialogLayout);

    connect(m_confirmBtn, &QPushButton::clicked, this, [=] {     
        Q_EMIT LogHintDialogClosed();
        close();
    });

    connect(this->closeButton(), &QPushButton::clicked, this, [=] {
        Q_EMIT LogHintDialogClosed();
        close();
    });
}


