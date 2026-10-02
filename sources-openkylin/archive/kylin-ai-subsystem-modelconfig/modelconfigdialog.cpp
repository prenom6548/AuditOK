#include "modelconfigdialog.h"
#include "aisubsystem.h"
#include <ukuistylehelper/ukuistylehelper.h>
#include <ukui-log4qt.h>

ModelConfigDialog::ModelConfigDialog(PrivateModel model, QWidget* parent) : QDialog(parent) {
    isEdit = true;
    initLayout();
    initConnect();
    changeTheme();

    ModelConfig mc;
    const auto models = mc.getExtendedModels(model.capability);
    for (const auto &model : models) {
        QString modelName = QString::fromStdString(model.modelName);
        QString modelVendor = QString::fromStdString(model.vendor);
        QString iconPath = getModelIconPath(modelVendor);
        comboboxModelName->addItem(QIcon(iconPath), modelName);
    }

    comboboxModelName->setCurrentText(model.modelName);
    comboboxModelName->setEnabled(false);
    buttonAccept->setText(tr("Ok"));

    loadModelData(model);
}

ModelConfigDialog::ModelConfigDialog(QWidget *parent):
    QDialog(parent)
{
    isEdit = false;
    initLayout();
    initConnect();
    changeTheme();
}

ModelConfigDialog::~ModelConfigDialog()
{

}

void ModelConfigDialog::initLayout()
{
    setFixedSize(420, 316);

    labelModelName = new kdk::KLabel;
    labelModelName->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KLabel_labelModelName");
    labelModelName->setText(tr("Model Type"));
    labelModelName->setFixedWidth(80);
    labelAPPID = new QLabel("APPID");
    labelAPPID->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelAPPID");
    labelAPPID->setFixedWidth(80);
    labelApiKey = new QLabel("APIKey");
    labelApiKey->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelApiKey");
    labelApiKey->setFixedWidth(80);
    labelSecretKey = new kdk::KLabel;
    labelSecretKey->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KLabel_labelSecretKey");
    labelSecretKey->setText("APISecret");
    labelSecretKey->setFixedWidth(80);

    comboboxModelName = new QComboBox;   // baidu,xunfei
    comboboxModelName->setAccessibleName("kylin-ai-subsystem-modelconfig_QComboBox_comboboxModelName");
    comboboxModelName->setFixedHeight(36);
    comboboxModelName->setIconSize(QSize(16, 16));

    passwordAPPID = new kdk::KPasswordEdit(nullptr);
    connect(passwordAPPID, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            passwordAPPID->setState(LoginState::LoginFailed);
        } else {
            passwordAPPID->setState(LoginState::Ordinary);
        }
    });
    passwordAPPID->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_passwordAPPID");
    passwordAPPID->setFocusPolicy(Qt::WheelFocus);
    passwordAPPID->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExp regExp("[ A-Za-z0-9!@#$%^&*()_+\\-=\\[\\]{}|;':\",./<>?`~\\\\]*");
    QRegExpValidator* validator_appid = new QRegExpValidator(regExp, passwordAPPID);
    passwordAPPID->setValidator(validator_appid);

    passwordEditApi = new kdk::KPasswordEdit(nullptr);
    connect(passwordEditApi, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            passwordEditApi->setState(LoginState::LoginFailed);
        } else {
            passwordEditApi->setState(LoginState::Ordinary);
        }
    });
    passwordEditApi->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_passwordEditApi");
    passwordEditApi->setFocusPolicy(Qt::WheelFocus);
    passwordEditApi->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_api = new QRegExpValidator(regExp, passwordEditApi);
    passwordEditApi->setValidator(validator_api);

    passwordEditSecret = new kdk::KPasswordEdit(nullptr);
    connect(passwordEditSecret, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            passwordEditSecret->setState(LoginState::LoginFailed);
        } else {
            passwordEditSecret->setState(LoginState::Ordinary);
        }
    });
    passwordEditSecret->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_passwordEditSecret");
    passwordEditSecret->setFocusPolicy(Qt::WheelFocus);
    passwordEditSecret->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_secret = new QRegExpValidator(regExp, passwordEditSecret);
    passwordEditSecret->setValidator(validator_secret);

    // 语音听写&在线语音合成(流式版)
    lineSeparator1 = new QFrame(this);
    lineSeparator1->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_lineSeparator1");
    lineSeparator1->setFrameShape(QFrame::HLine); // 横线
    lineSeparator1->setFixedHeight(1);

    labelSpeechContinuous = new QLabel(tr("Voice dictation & online voice synthesis (streaming version)"));
    labelSpeechContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelSpeechContinuous");
    labelAPPIDContinuous = new QLabel("APPID");
    labelAPPIDContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelAPPIDContinuous");
    labelAPPIDContinuous->setFixedWidth(80);
    labelApiKeyContinuous = new QLabel("APIKey");
    labelApiKeyContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelApiKeyContinuous");
    labelApiKeyContinuous->setFixedWidth(80);
    labelSecretKeyContinuous = new kdk::KLabel;
    labelSecretKeyContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KLabel_labelSecretKeyContinuous");
    labelSecretKeyContinuous->setText("APISecret");
    labelSecretKeyContinuous->setFixedWidth(80);

    switchContinuous = new kdk::KSwitchButton(nullptr);
    switchContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KSwitchButton_switchContinuous");
    switchContinuous->setChecked(true);
    switchContinuous->setFixedSize(44, 24);


    pwdAppIdContinuous = new kdk::KPasswordEdit(nullptr);
    connect(pwdAppIdContinuous, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            pwdAppIdContinuous->setState(LoginState::LoginFailed);
        } else {
            pwdAppIdContinuous->setState(LoginState::Ordinary);
        }
    });
    pwdAppIdContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_pwdAppIdContinuous");
    pwdAppIdContinuous->setFocusPolicy(Qt::WheelFocus);
    pwdAppIdContinuous->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_appid_cont = new QRegExpValidator(regExp, pwdAppIdContinuous);
    pwdAppIdContinuous->setValidator(validator_appid_cont);

    pwdEditApiContinuous = new kdk::KPasswordEdit(nullptr);
    connect(pwdEditApiContinuous, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            pwdEditApiContinuous->setState(LoginState::LoginFailed);
        } else {
            pwdEditApiContinuous->setState(LoginState::Ordinary);
        }
    });
    pwdEditApiContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_pwdEditApiContinuous");
    pwdEditApiContinuous->setFocusPolicy(Qt::WheelFocus);
    pwdEditApiContinuous->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_api_cont = new QRegExpValidator(regExp, pwdEditApiContinuous);
    pwdEditApiContinuous->setValidator(validator_api_cont);

    pwdEditSecretContinuous = new kdk::KPasswordEdit(nullptr);
    connect(pwdEditSecretContinuous, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            pwdEditSecretContinuous->setState(LoginState::LoginFailed);
        } else {
            pwdEditSecretContinuous->setState(LoginState::Ordinary);
        }
    });
    pwdEditSecretContinuous->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_pwdEditSecretContinuous");
    pwdEditSecretContinuous->setFocusPolicy(Qt::WheelFocus);
    pwdEditSecretContinuous->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_secret_cont = new QRegExpValidator(regExp, pwdEditSecretContinuous);
    pwdEditSecretContinuous->setValidator(validator_secret_cont);

    // 实时语音转写

    lineSeparator2 = new QFrame(this);
    lineSeparator2->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_lineSeparator2");
    lineSeparator2->setFrameShape(QFrame::HLine); // 横线
    lineSeparator2->setFixedHeight(1);

    labelSpeechRealTime = new QLabel(tr("Real time voice transcription"));
    labelSpeechRealTime->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelSpeechRealTime");
    labelAPPIDRealTime = new QLabel("APPID");
    labelAPPIDRealTime->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelAPPIDRealTime");
    labelAPPIDRealTime->setFixedWidth(80);
    labelApiKeyRealTime = new QLabel("APIKey");
    labelApiKeyRealTime->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelApiKeyRealTime");
    labelApiKeyRealTime->setFixedWidth(80);

    switchRealTime = new kdk::KSwitchButton(nullptr);
    switchRealTime->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KSwitchButton_switchRealTime");
    switchRealTime->setChecked(true);
    switchRealTime->setFixedSize(44, 24);

    pwdAppIdRealTime = new kdk::KPasswordEdit(nullptr);
    connect(pwdAppIdRealTime, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            pwdAppIdRealTime->setState(LoginState::LoginFailed);
        } else {
            pwdAppIdRealTime->setState(LoginState::Ordinary);
        }
    });
    pwdAppIdRealTime->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_pwdAppIdRealTime");
    pwdAppIdRealTime->setFocusPolicy(Qt::WheelFocus);
    pwdAppIdRealTime->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_appid_rt = new QRegExpValidator(regExp, pwdAppIdRealTime);
    pwdAppIdRealTime->setValidator(validator_appid_rt);

    pwdEditApiRealTime = new kdk::KPasswordEdit(nullptr);
    connect(pwdEditApiRealTime, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains(' ')) {
            pwdEditApiRealTime->setState(LoginState::LoginFailed);
        } else {
            pwdEditApiRealTime->setState(LoginState::Ordinary);
        }
    });
    pwdEditApiRealTime->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_pwdEditApiRealTime");
    pwdEditApiRealTime->setFocusPolicy(Qt::WheelFocus);
    pwdEditApiRealTime->setFixedHeight(36);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_api_rt = new QRegExpValidator(regExp, pwdEditApiRealTime);
    pwdEditApiRealTime->setValidator(validator_api_rt);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setVerticalSpacing(8);
    gridLayout->addWidget(labelModelName, 0, 0);            // 第一行第一列
    gridLayout->addWidget(labelAPPID, 1, 0);
    gridLayout->addWidget(labelApiKey, 2, 0);
    gridLayout->addWidget(labelSecretKey, 3, 0);

    gridLayout->addWidget(comboboxModelName, 0, 1);             // 第一行第二列
    gridLayout->addWidget(passwordAPPID, 1, 1);
    gridLayout->addWidget(passwordEditApi, 2, 1);
    gridLayout->addWidget(passwordEditSecret, 3, 1);

    // 语音听写&在线语音合成(流式版)
    continuousWidget = new QWidget;
    continuousWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_continuousWidget");
//    continuousWidget->setContentsMargins(0, 6, 0, 6);
    continuousWidget->setContentsMargins(0, 4, 0, 4);
    QHBoxLayout *continuousLayout = new QHBoxLayout;
    continuousLayout->setContentsMargins(0, 0, 0, 0);
    labelSpeechContinuous->setAlignment(Qt::AlignVCenter);
    continuousLayout->addWidget(labelSpeechContinuous);
    continuousLayout->addStretch();
    continuousLayout->addWidget(switchContinuous);
    continuousWidget->setLayout(continuousLayout);

    QGridLayout *gridContinuousLayout = new QGridLayout();
    gridContinuousLayout->setVerticalSpacing(8);
    gridContinuousLayout->addWidget(labelAPPIDContinuous, 0, 0);  // 第一行第一列
    gridContinuousLayout->addWidget(labelApiKeyContinuous, 1, 0);
    gridContinuousLayout->addWidget(labelSecretKeyContinuous, 2, 0);

    gridContinuousLayout->addWidget(pwdAppIdContinuous, 0, 1);    // 第一行第二列
    gridContinuousLayout->addWidget(pwdEditApiContinuous, 1, 1);
    gridContinuousLayout->addWidget(pwdEditSecretContinuous, 2, 1);

    lineSeparator1->setVisible(false);
    labelSpeechContinuous->setVisible(false);
    labelAPPIDContinuous->setVisible(false);
    labelApiKeyContinuous->setVisible(false);
    labelSecretKeyContinuous->setVisible(false);
    switchContinuous->setVisible(false);
    pwdAppIdContinuous->setVisible(false);
    pwdEditApiContinuous->setVisible(false);
    pwdEditSecretContinuous->setVisible(false);
    continuousWidget->setVisible(false);

    // 实时语音转写
    realTimeWidget = new QWidget;
    realTimeWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_realTimeWidget");
//    realTimeWidget->setContentsMargins(0, 6, 0, 6);
    realTimeWidget->setContentsMargins(0, 4, 0, 4);
    QHBoxLayout *realTimeLayout = new QHBoxLayout;
    realTimeLayout->setContentsMargins(0, 0, 0, 0);
    labelSpeechRealTime->setAlignment(Qt::AlignVCenter);
    realTimeLayout->addWidget(labelSpeechRealTime);
    realTimeLayout->addStretch();
    realTimeLayout->addWidget(switchRealTime);
    realTimeWidget->setLayout(realTimeLayout);

    QGridLayout *gridRealTimeLayout = new QGridLayout();
    gridRealTimeLayout->setVerticalSpacing(8);
    gridRealTimeLayout->setHorizontalSpacing(8);
    gridRealTimeLayout->addWidget(labelAPPIDRealTime, 0, 0);      // 第一行第一列
    gridRealTimeLayout->addWidget(labelApiKeyRealTime, 1, 0);

    gridRealTimeLayout->addWidget(pwdAppIdRealTime, 0, 1);        // 第一行第二列
    gridRealTimeLayout->addWidget(pwdEditApiRealTime, 1, 1);

    lineSeparator2->setVisible(false);
    labelSpeechRealTime->setVisible(false);
    labelAPPIDRealTime->setVisible(false);
    labelApiKeyRealTime->setVisible(false);
    switchRealTime->setVisible(false);
    pwdAppIdRealTime->setVisible(false);
    pwdEditApiRealTime->setVisible(false);
    realTimeWidget->setVisible(false);

    labelErrorInfo = new QLabel();
    labelErrorInfo->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelErrorInfo");
    labelErrorInfo->setStyleSheet("color:red; font-size: 14px; font-weight: 400; line-height: 20px;");

    QHBoxLayout *errorInfoLayout = new QHBoxLayout;
    errorInfoLayout->setContentsMargins(90, 0, 0, 0);
    errorInfoLayout->addWidget(labelErrorInfo);
    errorInfoLayout->addStretch();


    btnHelpLink = new kdk::KBorderlessButton(tr("Model Configuration Tutorial"));
    btnHelpLink->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KBorderlessButton_btnHelpLink");

    buttonCancel = new QPushButton(tr("Cancel"));
    buttonCancel->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_buttonCancel");
    buttonCancel->setFixedHeight(36);
    buttonCancel->setFocusPolicy(Qt::NoFocus);
    buttonCancel->setProperty("useButtonPalette", true);

    buttonAccept = new QPushButton(tr("Accept"));
    buttonAccept->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_buttonAccept");
    buttonAccept->setFixedHeight(36);
    buttonAccept->setFocusPolicy(Qt::NoFocus);
    buttonAccept->setEnabled(false);
    buttonAccept->setProperty("isImportant", true);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(btnHelpLink);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonCancel);
    buttonLayout->setSpacing(16);
    buttonLayout->addWidget(buttonAccept);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(24, 16, 24, 24);
    contentLayout->addLayout(gridLayout);
    contentLayout->addWidget(lineSeparator1);
    contentLayout->addWidget(continuousWidget);
    contentLayout->addLayout(gridContinuousLayout);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(lineSeparator2);
    contentLayout->addWidget(realTimeWidget);
    contentLayout->addLayout(gridRealTimeLayout);
    contentLayout->addLayout(errorInfoLayout);
    contentLayout->addStretch();
    contentLayout->addLayout(buttonLayout);

    contentWidget = new QWidget;
    contentWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_contentWidget");
    contentWidget->setContentsMargins(0, 0, 0, 0);
    contentWidget->setLayout(contentLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(contentWidget);
    this->setLayout(vLayout);
}

void ModelConfigDialog::initConnect()
{
    connect(comboboxModelName, SIGNAL(currentIndexChanged(int)), this, SLOT(updateLayout(int)));
    connect(btnHelpLink, &QPushButton::clicked, [this](){
        QDesktopServices::openUrl(kaglobal::openLink);
    });

    connect(buttonCancel, &QPushButton::clicked, [this](){reject();});
    connect(buttonAccept, &QPushButton::clicked, [this](){accept();});
    connect(switchContinuous, &KSwitchButton::stateChanged, this, &ModelConfigDialog::onSwitchContinuousChanged);
    connect(switchRealTime, &KSwitchButton::stateChanged, this, &ModelConfigDialog::onSwitchRealTimeChanged);

    connect(passwordAPPID, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordAPPID); });
    connect(passwordEditApi, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordEditApi); });
    connect(passwordEditSecret, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordEditSecret); });

    connect(pwdAppIdContinuous, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordAPPID); });
    connect(pwdEditApiContinuous, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordEditApi); });
    connect(pwdEditSecretContinuous, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordEditSecret); });
    connect(pwdAppIdRealTime, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordAPPID); });
    connect(pwdEditApiRealTime, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordEditApi); });

    connect(passwordAPPID, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });
    connect(passwordEditApi, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });
    connect(passwordEditSecret, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });
    connect(pwdAppIdContinuous, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });
    connect(pwdEditApiContinuous, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });
    connect(pwdEditSecretContinuous, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });
    connect(pwdAppIdRealTime, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });
    connect(pwdEditApiRealTime, &KPasswordEdit::textChanged, this, [this]() { checkIsHasSpaces(); });

}

void ModelConfigDialog::changeTheme()
{
    KyInfo() << "changetheme";
    QPalette p = this->palette();
    QColor color = p.color(QPalette::Base);
    p.setColor(QPalette::Window, color);
    this->setPalette(p);
}

void ModelConfigDialog::updateLayout(int index)
{
    if (parentWidget())
    {
        if (isEdit == false) {
            // 添加时
            buttonAccept->setText(tr("Add"));
            passwordAPPID->setText("");
            passwordEditApi->setText("");
            passwordEditSecret->setText("");
            labelErrorInfo->setText("");

            pwdAppIdContinuous->setText("");
            pwdEditApiContinuous->setText("");
            pwdEditSecretContinuous->setText("");
            pwdAppIdRealTime->setText("");
            pwdEditApiRealTime->setText("");

        }
        // 全部隐藏
        labelAPPID->setVisible(false);
        labelApiKey->setVisible(false);
        labelSecretKey->setVisible(false);
        passwordAPPID->setVisible(false);
        passwordEditApi->setVisible(false);
        passwordEditSecret->setVisible(false);

        lineSeparator1->setVisible(false);
        labelSpeechContinuous->setVisible(false);
        labelAPPIDContinuous->setVisible(false);
        labelApiKeyContinuous->setVisible(false);
        labelSecretKeyContinuous->setVisible(false);
        switchContinuous->setVisible(false);
        pwdAppIdContinuous->setVisible(false);
        pwdEditApiContinuous->setVisible(false);
        pwdEditSecretContinuous->setVisible(false);
        continuousWidget->setVisible(false);

        lineSeparator2->setVisible(false);
        labelSpeechRealTime->setVisible(false);
        labelAPPIDRealTime->setVisible(false);
        labelApiKeyRealTime->setVisible(false);
        switchRealTime->setVisible(false);
        pwdAppIdRealTime->setVisible(false);
        pwdEditApiRealTime->setVisible(false);
        realTimeWidget->setVisible(false);

        // 遍历验证信息，根据字段设置是否可见
        ModelConfig mc;
        QString currentText = comboboxModelName->itemText(index);
        std::vector<MultiAuthentication> multiAuths  = mc.getModelAuthentications(currentText.toStdString());
        for (const auto &authen : multiAuths) {
            for (const auto &au : authen.authentications) {
                if (au.key == "appId") {
                    labelAPPID->setVisible(true);
                    passwordAPPID->setVisible(true);
                }
                if (au.key == "apiKey") {
                    labelApiKey->setVisible(true);
                    passwordEditApi->setVisible(true);
                }
                if (au.key == "secretKey") {
                    labelSecretKey->setVisible(true);
                    passwordEditSecret->setVisible(true);
                }
                if (au.key == "appIdContinuous") {
                    lineSeparator1->setVisible(true);
                    labelSpeechContinuous->setVisible(true);
                    switchContinuous->setVisible(true);
                    continuousWidget->setVisible(true);
                    labelAPPIDContinuous->setVisible(true);
                    pwdAppIdContinuous->setVisible(true);

                }
                if (au.key == "apiKeyContinuous") {
                    lineSeparator1->setVisible(true);
                    labelSpeechContinuous->setVisible(true);
                    switchContinuous->setVisible(true);
                    continuousWidget->setVisible(true);
                    labelApiKeyContinuous->setVisible(true);
                    pwdEditApiContinuous->setVisible(true);
                }
                if (au.key == "secretKeyContinuous") {
                    lineSeparator1->setVisible(true);
                    labelSpeechContinuous->setVisible(true);
                    switchContinuous->setVisible(true);
                    continuousWidget->setVisible(true);
                    labelSecretKeyContinuous->setVisible(true);
                    pwdEditSecretContinuous->setVisible(true);
                }

                if (au.key == "appIdRealTime") {
                    lineSeparator2->setVisible(true);
                    labelSpeechRealTime->setVisible(true);
                    switchRealTime->setVisible(true);
                    realTimeWidget->setVisible(true);
                    labelAPPIDRealTime->setVisible(true);
                    pwdAppIdRealTime->setVisible(true);

                }
                if (au.key == "apiKeyRealTime") {
                    lineSeparator2->setVisible(true);
                    labelSpeechRealTime->setVisible(true);
                    switchRealTime->setVisible(true);
                    realTimeWidget->setVisible(true);
                    labelApiKeyRealTime->setVisible(true);
                    pwdEditApiRealTime->setVisible(true);
                }
            }
        }
        if (m_capability == AiCapability::Nlp || m_capability == AiCapability::Vision) {
            this->setFixedSize(420, 316);

        } else if (m_capability == AiCapability::Speech) {
            if (currentText == "讯飞-语音大模型")
            {
                this->setFixedSize(420, 502);
            } else {
                this->setFixedSize(420, 316);

            }
        }

        moveToCenter();
    }
}

void ModelConfigDialog::updateButtonState()
{
    if (passwordAPPID->isVisible() == false) {    // 深度求索,百度的自然语言和文生图不需要appid
        if(passwordEditSecret->isVisible() == false
                && passwordEditApi->isVisible() == true
                && passwordEditApi->text().length() > 0) {
            buttonAccept->setEnabled(true);
            return;
        }
        else if (passwordEditApi->text().length()>= 12 && passwordEditSecret->text().length() >= 12) {
            buttonAccept->setEnabled(true);

        } else {
            buttonAccept->setEnabled(false);
        }

    } else if (passwordEditSecret->isVisible()) { // AppId必填，ApiKey和SecrectKey大于等于12
        if (passwordAPPID->text().length() > 0 && passwordEditApi->text().length()>= 12 && passwordEditSecret->text().length() >= 12)
        {
            buttonAccept->setEnabled(true);

        } else {
            buttonAccept->setEnabled(false);
        }
    } else {                                      // 不需要SecrectKey的话，AppId必填，ApiKey大于等于12
        if (passwordAPPID->text().length() > 0 && passwordEditApi->text().length()>= 12)
        {
            buttonAccept->setEnabled(true);

        } else {
            buttonAccept->setEnabled(false);
        }
    }

    // 在已选择的没填完之前，置灰；在无选择的时候，置灰

    if (switchContinuous->isVisible() && switchRealTime->isVisible())
    {
        bool bothSwitchesChecked = switchRealTime->isChecked() && switchContinuous->isChecked();
        bool realTimeChecked = switchRealTime->isChecked();
        bool continuousChecked = switchContinuous->isChecked();
        bool enableButton = false;
        // 如果两个开关都被选择
        if (bothSwitchesChecked) {
            if (pwdAppIdContinuous->text().length() > 0
                && pwdAppIdRealTime->text().length() > 0
                && pwdEditApiRealTime->text().length() >= 12
                && pwdEditApiContinuous->text().length() >= 12
                && pwdEditSecretContinuous->text().length() >= 12) {
                enableButton = true;
            }
        }
        // 如果只有一个开关被选择
        else if (realTimeChecked || continuousChecked) {
            if (realTimeChecked
                && pwdAppIdRealTime->text().length() > 0
                && pwdEditApiRealTime->text().length() >= 12) {
                enableButton = true;

            } else if (continuousChecked
                       && pwdAppIdContinuous->text().length() > 0
                       && pwdEditApiContinuous->text().length() >= 12
                       && pwdEditSecretContinuous->text().length() >= 12) {
                enableButton = true;
            }
        }
        // 没有开关被选择
        else {
            enableButton = false;
        }

        buttonAccept->setEnabled(enableButton);
    }
}
bool ModelConfigDialog::checkIsHasSpaces()
{
    bool flag = false;

    QList<kdk::KPasswordEdit *> passwordEdits =  this->findChildren<kdk::KPasswordEdit*>();
    if(passwordEdits.isEmpty()) {
        return flag;
    }
    else
    {
        for(int i = 0; i <passwordEdits.size() ;i ++)
        {
            kdk::KPasswordEdit *currentEdit = passwordEdits.at(i);
            if(currentEdit->text().contains(" ") || currentEdit->text().contains('\n'))
            {
                this->labelErrorInfo->setText(tr("Cannot contain spaces, please re-enter!"));
                flag = true;
                buttonAccept->setEnabled(false);
                break;
            }
            else
            {
                this->labelErrorInfo->setText("");
            }

        }

    }
    return flag;
}

void ModelConfigDialog::onPasswordEditChanged(KPasswordEdit *passwordEdit)
{
    if (passwordEdit->text().length() == 0) {
        passwordEdit->setEchoModeBtnVisible(true);
    }
    updateButtonState();

}

void ModelConfigDialog::onSwitchContinuousChanged(bool isVisible) {
    labelAPPIDContinuous->setVisible(isVisible);
    labelApiKeyContinuous->setVisible(isVisible);
    labelSecretKeyContinuous->setVisible(isVisible);
    pwdAppIdContinuous->setVisible(isVisible);
    pwdEditApiContinuous->setVisible(isVisible);
    pwdEditSecretContinuous->setVisible(isVisible);

    adjustDialogSize();
    updateButtonState();
    isContinuousSelected = isVisible;
}

void ModelConfigDialog::onSwitchRealTimeChanged(bool isVisible) {
    labelAPPIDRealTime->setVisible(isVisible);
    labelApiKeyRealTime->setVisible(isVisible);
    pwdAppIdRealTime->setVisible(isVisible);
    pwdEditApiRealTime->setVisible(isVisible);

    adjustDialogSize();
    updateButtonState();
    isRealTimeSelected = isVisible;

}

void ModelConfigDialog::adjustDialogSize() {
    if (switchContinuous->isVisible() && switchRealTime->isVisible())
    {
        bool bothSwitchesChecked = switchRealTime->isChecked() && switchContinuous->isChecked();
        bool realTimeChecked = switchRealTime->isChecked();
        bool continuousChecked = switchContinuous->isChecked();

        // 如果两个开关都被选择
        if (bothSwitchesChecked) {
            this->setFixedSize(420, 502);
        }
        // 如果只有一个开关被选择
        else if (realTimeChecked || continuousChecked) {
            this->setFixedSize(420, 414);
        }
        // 没有开关被选择
        else {
            this->setFixedSize(420, 316);
        }
    }
}

/**
 * @brief ModelConfigDialog::initUIData
 *          初始化所有模型数据
 */
void ModelConfigDialog::initUIData(const AiCapability capability, const DeployType deployType) {
    m_capability = capability;
    if(m_capability == AiCapability::Nlp){
        this->setWindowTitle(tr("Add-Public Cloud NLP Model"));  // 添加-公有云文本类模型
    }else if(m_capability == AiCapability::Vision){
        this->setWindowTitle(tr("Add-Public Cloud Visual Model"));  // 添加-公有云图像类模型
    }else if(m_capability == AiCapability::Speech){
        this->setWindowTitle(tr("Add-Public Cloud Speech Model"));  // 添加-公有云语音类模型
    }

    ModelConfig mc;
    const auto models = mc.getExtendedModels(capability);
    for (const auto &model : models) {
        QString modelName = QString::fromStdString(model.modelName);
        QString modelVendor = QString::fromStdString(model.vendor);

        //根据部署类别，当前内容只显示云端模型
        if (model.deployType == DeployType::Custom) {
            QString iconPath = getModelIconPath(modelVendor);
            // 添加modelVendor（存储在Qt::UserRole中）
            comboboxModelName->addItem(QIcon(iconPath), modelName, modelVendor);
        }
        KyInfo() << modelName <<static_cast<int>(deployType);
    }

    QString passwdTip = tr("Required");
    passwordAPPID->setPlaceholderText(passwdTip);
    passwordEditApi->setPlaceholderText(passwdTip);
    passwordEditSecret->setPlaceholderText(passwdTip);

    pwdAppIdContinuous->setPlaceholderText(passwdTip);
    pwdEditApiContinuous->setPlaceholderText(passwdTip);
    pwdEditSecretContinuous->setPlaceholderText(passwdTip);
    pwdAppIdRealTime->setPlaceholderText(passwdTip);
    pwdEditApiRealTime->setPlaceholderText(passwdTip);
}

// 编辑 - 加载对应模型数据
void ModelConfigDialog::loadModelData(PrivateModel model) {
    m_model = model;
    m_capability = model.capability;
    std::vector<MultiAuthentication> modelsAuthens = model.multiAuths;

    if(m_capability == AiCapability::Nlp){
        this->setWindowTitle(tr("Edit-Public Cloud NLP Model"));  // 编辑-公有云文本类模型
    }else if(m_capability == AiCapability::Vision){
        this->setWindowTitle(tr("Edit-Public Cloud Visual Model"));  // 编辑-公有云图像类模型
    }else if(m_capability == AiCapability::Speech){
        this->setWindowTitle(tr("Edit-Public Cloud Speech Model"));  // 编辑-公有云语音类模型
    }

    if (parentWidget()) {
        PrivateModelConfigWidget* privateModelConfigPage = TestAiSubSystem::getPrivateModelConfigPage();

        if (privateModelConfigPage) {
            if (model.capability == AiCapability::Speech) {
                ModelConfig mc;
                std::vector<ExtendedMultiAuthentication> extendedAuths =
                    mc.getModelExtendedAuthentications(model.modelName.toStdString());

                for (size_t i = 0; i < extendedAuths.size(); ++i) {
                    const auto& extAuth = extendedAuths[i];
                    KyInfo() << "Item" << i << ":" << QString::fromStdString(extAuth.itemName)
                             << "Enabled:" << extAuth.enabled;

                    for (const auto& auth : extAuth.authentications) {
                        KyInfo() << "  Auth key:" << QString::fromStdString(auth.key)
                                 << "friendlyKeyName:" << QString::fromStdString(auth.friendlyKeyName)
                                 << "value length:" << auth.value.length();
                    }
                }
            }
            passwordAPPID->setEchoMode(QLineEdit::Password);
            passwordAPPID->setEchoModeBtnVisible(true);
            passwordEditApi->setEchoMode(QLineEdit::Password);
            passwordEditApi->setEchoModeBtnVisible(true);
            passwordEditSecret->setEchoMode(QLineEdit::Password);
            passwordEditSecret->setEchoModeBtnVisible(true);

            pwdAppIdContinuous->setEchoMode(QLineEdit::Password);
            pwdAppIdContinuous->setEchoModeBtnVisible(true);
            pwdEditApiContinuous->setEchoMode(QLineEdit::Password);
            pwdEditApiContinuous->setEchoModeBtnVisible(true);
            pwdEditSecretContinuous->setEchoMode(QLineEdit::Password);
            pwdEditSecretContinuous->setEchoModeBtnVisible(true);

            pwdEditSecretContinuous->setEchoMode(QLineEdit::Password);
            pwdEditSecretContinuous->setEchoModeBtnVisible(true);
            pwdEditApiRealTime->setEchoMode(QLineEdit::Password);
            pwdEditApiRealTime->setEchoModeBtnVisible(true);

            // 全部隐藏
            labelAPPID->setVisible(false);
            labelApiKey->setVisible(false);
            labelSecretKey->setVisible(false);
            passwordAPPID->setVisible(false);
            passwordEditApi->setVisible(false);
            passwordEditSecret->setVisible(false);

            lineSeparator1->setVisible(false);
            labelSpeechContinuous->setVisible(false);
            labelAPPIDContinuous->setVisible(false);
            labelApiKeyContinuous->setVisible(false);
            labelSecretKeyContinuous->setVisible(false);
            switchContinuous->setVisible(false);
            pwdAppIdContinuous->setVisible(false);
            pwdEditApiContinuous->setVisible(false);
            pwdEditSecretContinuous->setVisible(false);
            continuousWidget->setVisible(false);

            lineSeparator2->setVisible(false);
            labelSpeechRealTime->setVisible(false);
            labelAPPIDRealTime->setVisible(false);
            labelApiKeyRealTime->setVisible(false);
            switchRealTime->setVisible(false);
            pwdAppIdRealTime->setVisible(false);
            pwdEditApiRealTime->setVisible(false);
            realTimeWidget->setVisible(false);

            for (const auto &authen : modelsAuthens) {
                for (const auto &au : authen.authentications) {
                    if (au.key == "appId") {
                        labelAPPID->setVisible(true);
                        passwordAPPID->setVisible(true);

                        passwordAPPID->setText(QString::fromStdString(au.value));
                    }
                    if (au.key == "apiKey") {
                        labelApiKey->setVisible(true);
                        passwordEditApi->setVisible(true);

                        passwordEditApi->setText(QString::fromStdString(au.value));
                    }
                    if (au.key == "secretKey") {
                        labelSecretKey->setVisible(true);
                        passwordEditSecret->setVisible(true);

                        passwordEditSecret->setText(QString::fromStdString(au.value));
                    }
                    if (au.key == "appIdContinuous") {
                        lineSeparator1->setVisible(true);
                        labelSpeechContinuous->setVisible(true);
                        switchContinuous->setVisible(true);
                        continuousWidget->setVisible(true);
                        labelAPPIDContinuous->setVisible(true);
                        pwdAppIdContinuous->setVisible(true);

                        pwdAppIdContinuous->setText(QString::fromStdString(au.value));

                    }
                    if (au.key == "apiKeyContinuous") {
                        lineSeparator1->setVisible(true);
                        labelSpeechContinuous->setVisible(true);
                        switchContinuous->setVisible(true);
                        continuousWidget->setVisible(true);
                        labelApiKeyContinuous->setVisible(true);
                        pwdEditApiContinuous->setVisible(true);

                        pwdEditApiContinuous->setText(QString::fromStdString(au.value));
                    }
                    if (au.key == "secretKeyContinuous") {
                        lineSeparator1->setVisible(true);
                        labelSpeechContinuous->setVisible(true);
                        switchContinuous->setVisible(true);
                        continuousWidget->setVisible(true);
                        labelSecretKeyContinuous->setVisible(true);
                        pwdEditSecretContinuous->setVisible(true);

                        pwdEditSecretContinuous->setText(QString::fromStdString(au.value));
                    }

                    if (au.key == "appIdRealTime") {
                        lineSeparator2->setVisible(true);
                        labelSpeechRealTime->setVisible(true);
                        switchRealTime->setVisible(true);
                        realTimeWidget->setVisible(true);
                        labelAPPIDRealTime->setVisible(true);
                        pwdAppIdRealTime->setVisible(true);

                        pwdAppIdRealTime->setText(QString::fromStdString(au.value));

                    }
                    if (au.key == "apiKeyRealTime") {
                        lineSeparator2->setVisible(true);
                        labelSpeechRealTime->setVisible(true);
                        switchRealTime->setVisible(true);
                        realTimeWidget->setVisible(true);
                        labelApiKeyRealTime->setVisible(true);
                        pwdEditApiRealTime->setVisible(true);

                        pwdEditApiRealTime->setText(QString::fromStdString(au.value));
                    }
                }
            }

            if (m_capability == AiCapability::Nlp || m_capability == AiCapability::Vision) {
                this->setFixedSize(420, 316);

            } else if (m_capability == AiCapability::Speech) {

                if (model.modelName == "讯飞-语音大模型")
                {
                    ModelConfig mc;
                    std::vector<ExtendedMultiAuthentication> extendedAuths =
                        mc.getModelExtendedAuthentications(model.modelName.toStdString());

                    bool continuousChecked = false;  // 默认值
                    bool realTimeChecked = false;    // 默认值

                    for (const auto& extAuth : extendedAuths) {
                        if (extAuth.itemName == "语音听写&语音合成") {
                            continuousChecked = extAuth.enabled;
                        } else if (extAuth.itemName == "实时语音转写") {
                            realTimeChecked = extAuth.enabled;
                        }
                    }

                    switchContinuous->setChecked(continuousChecked);  // 设置开关状态
                    switchRealTime->setChecked(realTimeChecked);      // 设置开关状态
                    isContinuousSelected = continuousChecked;
                    isRealTimeSelected = realTimeChecked;

                    // 如果两个开关都被选择
                    if (continuousChecked && realTimeChecked) {
//                        KyInfo() << "两个都选择了";
                        this->setFixedSize(420, 502);
                        onSwitchContinuousChanged(true);
                        onSwitchRealTimeChanged(true);
                    }
                    // 如果只有一个开关被选择
                    else if (realTimeChecked || continuousChecked) {
//                        KyInfo() << "只有一个被选择";
                        this->setFixedSize(420, 414);
                        if (continuousChecked) {
                            onSwitchContinuousChanged(true);
                            onSwitchRealTimeChanged(false);
                        } else {
                            onSwitchContinuousChanged(false);
                            onSwitchRealTimeChanged(true);
                        }
                    }
                    // 没有开关被选择
                    else {
//                        KyInfo() << "无选择";
                        this->setFixedSize(420, 316);
                        onSwitchContinuousChanged(false);
                        onSwitchRealTimeChanged(false);
                    }
                } else {
//                    KyInfo() << "百度";
                    this->setFixedSize(420, 316);
                }
            }
            moveToCenter();
        }
    }
    // 编辑模型, 能够编辑，说明key值在添加时都符合要求，编辑模型时‘确定’按钮默认是启用状态
    buttonAccept->setEnabled(true);
}

void ModelConfigDialog::moveToCenter()
{
    if (parentWidget()) {
        // 获取父窗口的几何信息
        QRect parentGeometry = parentWidget()->geometry();

        // 计算窗口应该放置的位置，以使其在父窗口中心
        int x = parentGeometry.x() + (parentGeometry.width() - this->width()) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - this->height()) / 2;

        // 移动窗口到计算出的位置
        this->move(x, y);
    }
}

void ModelConfigDialog::accept()
{
    if(checkIsHasSpaces()){//未检查到空格则继续进行
        return ;
    }

    KyInfo() << isEdit;

    if (parentWidget())
    {
        PrivateModelConfigWidget* privateModelConfigPage = TestAiSubSystem::getPrivateModelConfigPage();

        if (privateModelConfigPage) {
            //判断是否存在该模型
            if (isEdit == false) {
                // 判断是否存在该模型
                if (m_capability == AiCapability::Nlp) {
                    if (privateModelConfigPage->m_nlpPublicCloudModelItemMap.contains(
                            comboboxModelName->currentText())) {
                        KyInfo() << "nlp already exist";
                        labelErrorInfo->setText(tr("The model type already exists. Cannot be repeated!"));
                        return;
                    }
                } else if (m_capability == AiCapability::Vision) {
                    if (privateModelConfigPage->m_visionPublicCloudModelItemMap.contains(
                            comboboxModelName->currentText())) {
                        KyInfo() << "vision already exist";
                        labelErrorInfo->setText(tr("The model type already exists. Cannot be repeated!"));
                        return;
                    }
                } else if (m_capability == AiCapability::Speech) {
                    if (privateModelConfigPage->m_speechPublicCloudModelItemMap.contains(
                            comboboxModelName->currentText())) {
                        KyInfo() << "speech already exist";
                        labelErrorInfo->setText(tr("The model type already exists. Cannot be repeated!"));
                        return;
                    }
                }
            }

            PrivateModel model;
            model.capability = m_capability;
            model.modelName = comboboxModelName->currentText();
            //读取comboboxModelName中储存在Qt::UserRole中的vendor信息
            int currentIndex = comboboxModelName->currentIndex();
            if (currentIndex >= 0) {
                model.vendor = comboboxModelName->itemData(currentIndex, Qt::UserRole).toString();
            }

            ModelConfig mc;
            std::vector<MultiAuthentication> auths = mc.getModelAuthentications(model.modelName.toStdString());
            bool isValueChanged = false;

            for (auto &authen : auths) {
                for (auto &au : authen.authentications) {
                    if (au.key == "appId") {
                        if (au.value != passwordAPPID->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = passwordAPPID->text().toStdString();
                    }
                    if (au.key == "apiKey") {
                        if (au.value != passwordEditApi->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = passwordEditApi->text().toStdString();
                    }
                    if (au.key == "secretKey") {
                        if (au.value != passwordEditSecret->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = passwordEditSecret->text().toStdString();
                    }
                    if (au.key == "appIdContinuous") {
                        if (au.value != pwdAppIdContinuous->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = pwdAppIdContinuous->text().toStdString();
                    }
                    if (au.key == "apiKeyContinuous") {
                        if (au.value != pwdEditApiContinuous->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = pwdEditApiContinuous->text().toStdString();
                    }
                    if (au.key == "secretKeyContinuous") {
                        if (au.value != pwdEditSecretContinuous->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = pwdEditSecretContinuous->text().toStdString();
                    }
                    if (au.key == "appIdRealTime") {
                        if (au.value != pwdAppIdRealTime->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = pwdAppIdRealTime->text().toStdString();
                    }
                    if (au.key == "apiKeyRealTime") {
                        if (au.value != pwdEditApiRealTime->text().toStdString()) {
                            isValueChanged = true;
                        }
                        au.value = pwdEditApiRealTime->text().toStdString();
                    }
                }
            }
            model.multiAuths = auths;

            bool isValueEmpty = false;

            for (const auto &authen : model.multiAuths) {
                for (const auto &au : authen.authentications) {
                    if (au.key == "appIdRealTime" || au.key == "apiKeyRealTime") {
                        if (au.value == "" && isRealTimeSelected == true) {
                            isValueEmpty = true;
                        }
                    } else if (au.key == "appIdContinuous" || au.key == "apiKeyContinuous" || au.key == "secretKeyContinuous") {
                        if (au.value == "" && isContinuousSelected == true) {
                            isValueEmpty = true;
                        }
                    } else {
                        if (au.value == "") {
                            isValueEmpty = true;
                        }
                    }
                }
            }

            if (isValueEmpty) {
                if (isEdit == true) {
                    showErrorMessage(tr("Model Editing failed"), this);
                } else {
                    showErrorMessage(tr("Adding model failed"), this);
                }
                KyInfo() << model.modelName << "验证信息为空，不允许设置模型";
                return;
            }

            if (isEdit == true) {
                // 编辑模型
                KyInfo() << "要编辑的modelname" << model.modelName << "isValueChanged" << isValueChanged << "isContinuousSelected" << isContinuousSelected << "isRealTimeSelected" << isRealTimeSelected ;
                bool isCloseEditWindow = privateModelConfigPage->editSelectCloudModel(
                    model, isContinuousSelected, isRealTimeSelected, isValueChanged, this);
                if (!isCloseEditWindow) {
                    return;
                }
            } else {
                // 添加模型
                if (privateModelConfigPage->getModelItemMap(m_capability).count() == 0) {
                    // 第一个添加的模型，设为选中
                    if (m_capability == AiCapability::Speech) {
                        // 对于语音模型，传递当前的开关状态
                        bool ret = privateModelConfigPage->setSelectCloudModelConfig(model, isContinuousSelected,
                                                                                     isRealTimeSelected);
                        if (ret == true) {
                            privateModelConfigPage->addModelItem(model);
                            KyInfo() << model.modelName << " 添加选中模型成功";
                        } else {
                            showErrorMessage(tr("Adding model failed"), this);
                            KyInfo() << model.modelName << " 添加选中模型失败";
                            return;
                        }
                    } else {
                        bool ret = privateModelConfigPage->setSelectCloudModelConfig(model);
                        if (ret == true) {
                            privateModelConfigPage->addModelItem(model);
                            KyInfo() << model.modelName << " 添加选中模型成功";
                        } else {
                            showErrorMessage(tr("Adding model failed"), this);
                            KyInfo() << model.modelName << " 添加选中模型失败";
                            return;
                        }
                    }
                } else {
                    // 已有选中模型，只更新验证信息和UI
                    ModelConfig mc;
                    bool ret;

                    if (m_capability == AiCapability::Speech) {
                        // 区分不同语音模型类型，只对讯飞语音模型使用扩展认证信息
                        if (model.modelName == "讯飞-语音大模型") {
                            // 对于讯飞语音模型，需要保存认证信息和功能启用状态
                            std::vector<ExtendedMultiAuthentication> extendedAuths;

                            // 处理"语音听写&语音合成"项
                            ExtendedMultiAuthentication continuousAuth;
                            continuousAuth.itemName = "语音听写&语音合成";
                            continuousAuth.enabled = isContinuousSelected;  // 使用当前UI状态
                            // 从当前模型认证信息中复制相关认证字段
                            for (const auto& auth : model.multiAuths) {
                                if (auth.itemName == "语音听写&语音合成") {
                                    continuousAuth.authentications = auth.authentications;
                                    break;
                                }
                            }
                            extendedAuths.push_back(continuousAuth);

                            // 处理"实时语音转写"项
                            ExtendedMultiAuthentication realTimeAuth;
                            realTimeAuth.itemName = "实时语音转写";
                            realTimeAuth.enabled = isRealTimeSelected;  // 使用当前UI状态
                            // 从当前模型认证信息中复制相关认证字段
                            for (const auto& auth : model.multiAuths) {
                                if (auth.itemName == "实时语音转写") {
                                    realTimeAuth.authentications = auth.authentications;
                                    break;
                                }
                            }
                            extendedAuths.push_back(realTimeAuth);

                            // 保存扩展认证信息（含功能启用状态和认证字段）
                            ret = mc.setModelAuthentications(model.modelName.toStdString(), extendedAuths);
                            KyInfo() << "Saving extended authentication info for Xunfei model, success:" << ret;
                        } else {
                            // 其他语音模型，使用setModelAuthentications
                            ret = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);
                        }
                    } else {
                        // 非语音模型，使用setModelAuthentications
                        ret = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);
                    }

                    if (ret == true) {
                        privateModelConfigPage->addModelItem(model);
                        KyInfo() << model.modelName << " 添加模型验证信息成功";
                    } else {
                        showErrorMessage(tr("Adding model failed"), this);
                        KyInfo() << model.modelName << " 添加模型验证信息失败";
                        return;
                    }
                }

                if (m_capability == AiCapability::Speech) {
                    // 同步两个开关的状态
                    privateModelConfigPage->continuousSelected = isContinuousSelected;
                    privateModelConfigPage->realtimeSelected = isRealTimeSelected;
                }
            }
        }
    }

    labelErrorInfo->setText("");
    return QDialog::accept();
}

void ModelConfigDialog::keyPressEvent(QKeyEvent *event)
{
    // Key_Enter是小键盘的确认键，Key_Return是大键盘的回车键
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (buttonAccept->isEnabled()) {
            emit accept();
        }
    }
}

QString ModelConfigDialog::getModelIconPath(const QString& vendor) {
    // 根据vendor值来判断图标
    if (vendor == "ernie" || vendor == "baidu") {  // 百度相关模型
        return getModelIconPathByTheme(vendor);
    } else if (vendor == "spark" || vendor == "xunfei") {  // 讯飞相关模型
        return getModelIconPathByTheme(vendor);
    }

    // 默认返回自定义模型图标
    return getModelIconPathByTheme(vendor);
}

QString ModelConfigDialog::getModelIconPathByTheme(const QString& vendor) {
    QString defaultIconPath;
    QString lightIconPath;

    // 根据vendor值来判断图标
    if (vendor == "ernie" || vendor == "baidu") {  // 百度相关模型
        defaultIconPath = ":/res/icon/baidu.svg";
        lightIconPath = ":/res/icon/baidu-light.svg";
    } else if (vendor == "spark" || vendor == "xunfei") {  // 讯飞相关模型
        defaultIconPath = ":/res/icon/xunfei.svg";
        lightIconPath = ":/res/icon/xunfei-light.svg";
    } else {
        // 默认返回自定义模型图标
        defaultIconPath = ":/res/icon/custom-model.svg";
        lightIconPath = ":/res/icon/custom-model-light.svg";
    }

    QString theme = getCurrentTheme();

    if (theme == "ukui-dark" || theme == "ukui-black") {
        // 深色主题使用带-light后缀的图标
        // 检查深色主题图标是否存在，如果不存在则使用原始图标
        QPixmap pixmap(lightIconPath);
        if (!pixmap.isNull()) {
            return lightIconPath;
        }
    }

    // 浅色主题使用普通图标或深色主题图标不存在时的回退
    return defaultIconPath;
}

QString ModelConfigDialog::getCurrentTheme() {
    // 直接创建QGSettings获取当前主题，只在需要时使用
    const QByteArray themeSchema(THEME_QT_SCHEMA);
    if (QGSettings::isSchemaInstalled(themeSchema)) {
        QGSettings settings(themeSchema);
        return settings.get("styleName").toString();
    }
    return "";
}
