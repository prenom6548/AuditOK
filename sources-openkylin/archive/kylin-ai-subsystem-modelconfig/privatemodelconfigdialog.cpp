#include "privatemodelconfigdialog.h"
#include "aisubsystem.h"
#include "util.h"

PrivateModelConfigDialog::PrivateModelConfigDialog(PrivateModel model, QWidget *parent) :
    QDialog(parent)
{
    isEdit = true;
    initLayout();
    initConnect();
    changeTheme();


    buttonAccept->setText(tr("Ok"));

    loadModelData(model);

}

PrivateModelConfigDialog::PrivateModelConfigDialog(QWidget *parent) : QDialog(parent)
{
    isEdit = false;
    initLayout();
    initConnect();
    changeTheme();
}

PrivateModelConfigDialog::~PrivateModelConfigDialog()
{

}

void PrivateModelConfigDialog::initUIData(AiCapability capability, DeployType deployType, const QString& sourceGroup) {
    m_capability = capability;
    m_sourceGroup = sourceGroup;

    if (sourceGroup == "localNetwork") {
        if (capability == AiCapability::Nlp) {
            this->setWindowTitle(tr("Add-Local Network NLP Model"));  // 添加-局域网文本类模型
        } else if (capability == AiCapability::Vision) {
            this->setWindowTitle(tr("Add-Local Network Visual Model"));  // 添加-局域网图像类模型
        } else if (capability == AiCapability::Speech) {
            this->setWindowTitle(tr("Add-Local Network Speech Model"));  // 添加-局域网语音类模型
        }
    } else {
        if (capability == AiCapability::Nlp) {
            this->setWindowTitle(tr("Add-Public Cloud NLP Model"));  // 添加-公有云文本类模型
        } else if (capability == AiCapability::Vision) {
            this->setWindowTitle(tr("Add-Public Cloud Visual Model"));  // 添加-公有云图像类模型
        } else if (capability == AiCapability::Speech) {
            this->setWindowTitle(tr("Add-Public Cloud Speech Model"));  // 添加-公有云语音类模型
        }
    }
}
// 编辑- 加载对应模型数据
void PrivateModelConfigDialog::loadModelData(PrivateModel model) {
    m_model = model;
    m_capability = model.capability;
    std::vector<MultiAuthentication> modelsAuthens = model.multiAuths;

    // 根据来源组和能力类型设置编辑窗口标题
    if (m_capability == AiCapability::Nlp) {
        if (m_sourceGroup == "localNetwork") {
            this->setWindowTitle(tr("Edit-Local Network NLP Model"));  // 编辑-局域网文本类模型
        } else {
            this->setWindowTitle(tr("Edit-Public Cloud NLP Model"));  // 编辑-公有云文本类模型
        }
    } else if (m_capability == AiCapability::Vision) {
        this->setWindowTitle(tr("Edit-Visual Model"));  // 编辑-视觉大模型
    } else if (m_capability == AiCapability::Speech) {
        this->setWindowTitle(tr("Edit-Speech Model"));  // 编辑-语音大模型
    }
    for (const auto &authen : modelsAuthens) {
        for (const auto &au : authen.authentications) {
            if (au.key == "apiKey") {
                passwordEditApi->setText(QString::fromStdString(au.value));
            }

        }
    }
    lineeditCustomName->setText(model.modelName);
    lineeditModelName->setText(model.modelVersion);
    passwordEditApi->setText(model.apiKey);
    lineeditRequestUrl->setText(model.apiUrl);
    passwordEditApi->setEchoMode(QLineEdit::Password);
    passwordEditApi->setEchoModeBtnVisible(true);

    //moveToCenter();
}

void PrivateModelConfigDialog::keyPressEvent(QKeyEvent *event)
{
    // Key_Enter是小键盘的确认键，Key_Return是大键盘的回车键
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (buttonAccept->isEnabled()) {
            emit accept();
        }
    }
}

void PrivateModelConfigDialog::onPasswordEditChanged(KPasswordEdit *passwordEdit)
{
    if (passwordEdit->text().length() == 0) {
        passwordEdit->setEchoModeBtnVisible(true);
    }
//    updateButtonState();

}

void PrivateModelConfigDialog::changeTheme()
{

}

void PrivateModelConfigDialog::initLayout()
{
    setFixedSize(452, 324);

    labelCustomName = new kdk::KLabel;
    labelCustomName->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelAccountName");
    labelCustomName->setText(tr("CustomName"));
    labelCustomName->setFixedWidth(80);
    labelModelName = new kdk::KLabel;
    labelModelName->setText(tr("ModelName"));
    labelModelName->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelModelName");
    labelModelName->setFixedWidth(80);
    labelApiKey = new QLabel("APIKey");
    labelApiKey->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_labelApiKey");
    labelApiKey->setFixedWidth(80);
    labelRequestUrl = new kdk::KLabel;
    labelRequestUrl->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KLabel_labelRequestUrl");
    labelRequestUrl->setText(tr("RequestUrl"));
    labelRequestUrl->setFixedWidth(80);

    lineeditCustomName = new QLineEdit;
    lineeditCustomName->setAccessibleName("kylin-ai-subsystem-modelconfig_QLineEdit_lineeditAccountName");
    lineeditCustomName->setFixedHeight(36);
    lineeditCustomName->setFocusPolicy(Qt::WheelFocus);
    lineeditCustomName->setPlaceholderText(tr("Required (To distinguish models)"));
    //编辑状态下禁止修改此内容
    if(isEdit){
        lineeditCustomName->setEnabled(false);
    }
    lineeditModelName = new QLineEdit;
    lineeditModelName->setAccessibleName("kylin-ai-subsystem-modelconfig_QLineEdit_lineeditModelName");
    lineeditModelName->setFixedHeight(36);
    lineeditModelName->setPlaceholderText(tr("Optional"));
    lineeditModelName->setFocusPolicy(Qt::WheelFocus);
    passwordEditApi = new kdk::KPasswordEdit(nullptr);
    connect(passwordEditApi, &QLineEdit::textChanged, [=](const QString &text)
    {
       if (text.contains(' '))
       {
          passwordEditApi->setState(LoginState::LoginFailed);
       }
       else
       {
          passwordEditApi->setState(LoginState::Ordinary);
       }
   });
    passwordEditApi->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_passwordEditApi");
    passwordEditApi->setFocusPolicy(Qt::WheelFocus);
    passwordEditApi->setFixedHeight(36);
    QString tip = QString(tr("Optional (Required for cloud services)"));
    passwordEditApi->setPlaceholderText(tip);

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExp regExp("[ A-Za-z0-9!@#$%^&*()_+\\-=\\[\\]{}|;':\",./<>?`~\\\\]*");
    QRegExpValidator* validator_api = new QRegExpValidator(regExp, passwordEditApi);
    passwordEditApi->setValidator(validator_api);

    lineeditRequestUrl = new QLineEdit;
    lineeditRequestUrl->setAccessibleName("kylin-ai-subsystem-modelconfig_QLineEdit_lineeditRequestUrl");
    lineeditRequestUrl->setFixedHeight(36);
    lineeditRequestUrl->setPlaceholderText(tr("Required"));

    // 不允许输入中文字符，只允许输入英文字母、数字、空格和常见符号
    QRegExpValidator* validator_url = new QRegExpValidator(regExp, lineeditRequestUrl);
    lineeditRequestUrl->setValidator(validator_url);

    QVBoxLayout* inputFieldsLayout = new QVBoxLayout();

    // 自定义名称
    QWidget* customNameContainer = new QWidget();
    QGridLayout* customNameGrid = new QGridLayout(customNameContainer);
    customNameGrid->setContentsMargins(0, 0, 0, 0);
    customNameGrid->setVerticalSpacing(0);

    labelErrorCustomName = new QLabel();
    labelErrorCustomName->setStyleSheet("color:red; font-size: 14px; font-weight: 400; line-height: 20px;");
    labelErrorCustomName->setVisible(true);

    customNameGrid->addWidget(labelCustomName, 0, 0);       // 第一行第一列
    customNameGrid->addWidget(lineeditCustomName, 0, 1);    // 第一行第二列
    customNameGrid->addWidget(labelErrorCustomName, 1, 1);  // 第二行第二列

    // 模型名称
    QWidget* modelNameContainer = new QWidget();
    QGridLayout* modelNameGrid = new QGridLayout(modelNameContainer);
    modelNameGrid->setContentsMargins(0, 0, 0, 0);
    modelNameGrid->setVerticalSpacing(0);

    labelErrorModelName = new QLabel();
    labelErrorModelName->setStyleSheet("color:red; font-size: 14px; font-weight: 400; line-height: 20px;");
    labelErrorModelName->setVisible(true);

    modelNameGrid->addWidget(labelModelName, 0, 0);
    modelNameGrid->addWidget(lineeditModelName, 0, 1);
    modelNameGrid->addWidget(labelErrorModelName, 1, 1);

    // APIKey
    QWidget* apiKeyContainer = new QWidget();
    QGridLayout* apiKeyGrid = new QGridLayout(apiKeyContainer);
    apiKeyGrid->setContentsMargins(0, 0, 0, 0);
    apiKeyGrid->setVerticalSpacing(0);

    labelErrorApiKey = new QLabel();
    labelErrorApiKey->setStyleSheet("color:red; font-size: 14px; font-weight: 400; line-height: 20px;");
    labelErrorApiKey->setVisible(true);

    apiKeyGrid->addWidget(labelApiKey, 0, 0);
    apiKeyGrid->addWidget(passwordEditApi, 0, 1);
    apiKeyGrid->addWidget(labelErrorApiKey, 1, 1);

    // 请求地址
    QWidget* requestUrlContainer = new QWidget();
    QGridLayout* requestUrlGrid = new QGridLayout(requestUrlContainer);
    requestUrlGrid->setContentsMargins(0, 0, 0, 0);
    requestUrlGrid->setVerticalSpacing(0);

    labelErrorRequestUrl = new QLabel();
    labelErrorRequestUrl->setStyleSheet("color:red; font-size: 14px; font-weight: 400; line-height: 20px;");
    labelErrorRequestUrl->setVisible(true);

    requestUrlGrid->addWidget(labelRequestUrl, 0, 0);
    requestUrlGrid->addWidget(lineeditRequestUrl, 0, 1);
    requestUrlGrid->addWidget(labelErrorRequestUrl, 1, 1);

    inputFieldsLayout->addWidget(customNameContainer);
    inputFieldsLayout->addWidget(modelNameContainer);
    inputFieldsLayout->addWidget(apiKeyContainer);
    inputFieldsLayout->addWidget(requestUrlContainer);

    inputFieldsLayout->addStretch();

    buttonCancel = new QPushButton(tr("Cancel"));
    buttonCancel->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_buttonCancel");
    buttonCancel->setFixedHeight(36);
    buttonCancel->setFixedWidth(94);
    buttonCancel->setFocusPolicy(Qt::NoFocus);
    buttonCancel->setProperty("useButtonPalette", true);

    buttonAccept = new QPushButton(tr("Accept"));
    buttonAccept->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_buttonAccept");
    buttonAccept->setFixedHeight(36);
    buttonAccept->setFixedWidth(94);
    buttonAccept->setFocusPolicy(Qt::NoFocus);
    buttonAccept->setEnabled(false);
    buttonAccept->setProperty("isImportant", true);

    btnHelpLink = new kdk::KBorderlessButton(tr("Model Configuration Tutorial"));
    btnHelpLink->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KBorderlessButton_btnHelpLink");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(btnHelpLink);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonCancel);
    buttonLayout->setSpacing(16);
    buttonLayout->addWidget(buttonAccept);


    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(24, 16, 24, 24);
    contentLayout->addLayout(inputFieldsLayout);
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

void PrivateModelConfigDialog::initConnect()
{
    connect(btnHelpLink, &QPushButton::clicked, [this]() { QDesktopServices::openUrl(kaglobal::openLink); });
    connect(buttonCancel, &QPushButton::clicked, [this](){reject();});
    connect(buttonAccept,&QPushButton::clicked,this,&PrivateModelConfigDialog::accept);

    connect(passwordEditApi, &KPasswordEdit::textChanged, this, [this]() { onPasswordEditChanged(passwordEditApi); });
    connect(passwordEditApi,&KPasswordEdit::textChanged,this,&PrivateModelConfigDialog::updateButtonState);
    connect(lineeditRequestUrl,&QLineEdit::textChanged,this,&PrivateModelConfigDialog::updateButtonState);
    connect(lineeditCustomName,&QLineEdit::textChanged,this,&PrivateModelConfigDialog::updateButtonState);
    connect(lineeditCustomName,&QLineEdit::textChanged,this, [=](const QString &text){

        int totalUnits = 0;
        QString truncatedText;
        for (int i = 0; i < text.length(); ++i) {
            QChar currentChar = text[i];
            // 中文、英文或其他字符占用 1 个单位
            totalUnits += 1;
            // 如果超出最大长度，就截取到此为止
            if (totalUnits > MAX_NAME_COUNT) {
                break;
            }

            truncatedText.append(currentChar);
        }

        if (text != truncatedText) {
            labelErrorCustomName->setText(tr("The length must not exceed 32 characters"));
            lineeditCustomName->setText(truncatedText);
        }

        if (totalUnits < MAX_NAME_COUNT) {
            labelErrorCustomName->setText("");
        }

        updateButtonState();

    });
}

void PrivateModelConfigDialog::updateButtonState()
{
    bool enableButton = true;
    if(!lineeditCustomName->text().isEmpty() && !lineeditRequestUrl->text().isEmpty())
    {
        // 检查API密钥是否包含空格
        if (passwordEditApi->text().contains(" ")) {
            labelErrorApiKey->setText(tr("Cannot contain spaces, please re-enter!"));
            enableButton = false;
        } else {
            labelErrorApiKey->setText("");
        }

        // 检查请求URL是否包含空格
        if (lineeditRequestUrl->text().contains(" ")) {
            labelErrorRequestUrl->setText(tr("Cannot contain spaces, please re-enter!"));
            enableButton = false;
        } else {
            labelErrorRequestUrl->setText("");
        }
    } else {
        // 检查API密钥是否包含空格
        if (passwordEditApi->text().contains(" ")) {
            labelErrorApiKey->setText(tr("Cannot contain spaces, please re-enter!"));
        } else {
            labelErrorApiKey->setText("");
        }

        // 检查请求URL是否包含空格
        if (lineeditRequestUrl->text().contains(" ")) {
            labelErrorRequestUrl->setText(tr("Cannot contain spaces, please re-enter!"));
        } else {
            labelErrorRequestUrl->setText("");
        }

        enableButton = false;
    }
    buttonAccept->setEnabled(enableButton);
}

bool PrivateModelConfigDialog::checkIsHasSpaces()
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
            if (currentEdit->text().contains(" ")) {
                flag = true;
                break;
            }
        }

    }
    return flag;
}


void PrivateModelConfigDialog::accept()
{
    if(checkIsHasSpaces()){//未检查到空格则继续进行
        return;
    }

    PrivateModelConfigWidget *privateModelConfigPage   =  TestAiSubSystem::getPrivateModelConfigPage();

    if (privateModelConfigPage) {
        //判断是否编辑模型
        if (isEdit == false) {
            // 判断是否存在该模型
            if (m_capability == AiCapability::Nlp) {
                if (privateModelConfigPage->m_nlpLocalNetworkModelItemMap.contains(lineeditCustomName->text()) ||
                    privateModelConfigPage->m_nlpPublicCloudModelItemMap.contains(lineeditCustomName->text())) {
                    KyInfo() << "nlp already exist";
                    labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                    return;
                }
            } else if (m_capability == AiCapability::Vision) {
                if (privateModelConfigPage->m_visionPublicCloudModelItemMap.contains(lineeditCustomName->text())) {
                    KyInfo() << "vision already exist";
                    labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                    return;
                }
            } else if (m_capability == AiCapability::Speech) {
                if (privateModelConfigPage->m_speechPublicCloudModelItemMap.contains(lineeditCustomName->text())) {
                    KyInfo() << "speech already exist";
                    labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                    return;
                }
            }
        }

        PrivateModel model;
        model.capability = m_capability;
        model.modelName =  lineeditCustomName ->text();
        model.modelVersion = lineeditModelName ->text();
        model.apiKey =  passwordEditApi ->text();
        model.apiUrl = lineeditRequestUrl->text();

        if (model.modelName == "" || model.apiUrl == "") {
            if (isEdit == true) {
                showErrorMessage(tr("Model Editing failed"), this);
            } else {
                showErrorMessage(tr("Adding model failed"), this);
            }
            KyInfo() << model.modelName << "必填信息为空，不允许设置模型";
            return;
        }
        ModelConfig mc;
        std::vector<MultiAuthentication> auths = mc.getModelAuthentications(model.modelName.toStdString());
        bool isValueChanged = true;
        for (auto &authen : auths) {
            for (auto &au : authen.authentications) {
                if (au.key == "apiKey") {
                    if (au.value != passwordEditApi->text().toStdString()) {
                        isValueChanged = true;
                    }
                    au.value = passwordEditApi->text().toStdString();
                }
            }
        }
        model.multiAuths = auths;
        if (isEdit == true) {
            // 编辑模型
            bool isCheckSelected = privateModelConfigPage->checkModelSelection(model.capability, model.modelName);
            KyInfo() << "Editing modelname" << model.modelName << "isCheckSelected" << isCheckSelected << "isValueChanged" << isValueChanged;
            bool isCloseEditWindow = privateModelConfigPage->editSelectPrivateModel(model, false, false, isValueChanged, this);
            if (!isCloseEditWindow) {
                return;
            }
        } else {
            // 添加模型
            if (m_capability == AiCapability::Nlp) {
                // 对于NLP模型，需要根据来源组分别判断
                if (m_sourceGroup == "localNetwork") {
                    // 局域网NLP模型
                    if (privateModelConfigPage->m_nlpLocalNetworkModelItemMap.count() == 0) {
                        // 局域网组中没有模型，设为选中
                        bool ret = privateModelConfigPage->setSelectPrivateModelConfig(model);
                        if (ret == true) {
                            privateModelConfigPage->addModelItem(model, m_sourceGroup);
                            KyInfo() << model.modelName << " 添加选中模型成功";
                        } else {
                            KyInfo() << "nlp already exist";
                            labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                            return;
                        }
                    } else {
                        // 局域网组中已有模型，只更新验证信息和UI
                        bool ret = privateModelConfigPage->addPrivateModel(model);
                        if (ret == true) {
                            privateModelConfigPage->addModelItem(model, m_sourceGroup);
                            KyInfo() << model.modelName << " 更新模型验证信息成功";
                        } else {
                            KyInfo() << "nlp already exist";
                            labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                            return;
                        }
                    }
                } else {
                    // 公有云NLP模型
                    if (privateModelConfigPage->m_nlpPublicCloudModelItemMap.count() == 0) {
                        // 公有云组中没有模型，设为选中
                        bool ret = privateModelConfigPage->setSelectPrivateModelConfig(model);
                        if (ret == true) {
                            privateModelConfigPage->addModelItem(model, m_sourceGroup);
                            KyInfo() << model.modelName << " 添加选中模型成功";
                        } else {
                            KyInfo() << "nlp already exist";
                            labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                            return;
                        }
                    } else {
                        // 公有云组中已有模型，只更新验证信息和UI
                        bool ret = privateModelConfigPage->addPrivateModel(model);
                        if (ret == true) {
                            privateModelConfigPage->addModelItem(model, m_sourceGroup);
                            KyInfo() << model.modelName << " 更新模型验证信息成功";
                        } else {
                            KyInfo() << "nlp already exist";
                            labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                            return;
                        }
                    }
                }
            } else {
                // 对于非NLP模型，使用原来的逻辑
                if (privateModelConfigPage->getModelItemMap(m_capability).count() == 0) {
                    // 第一个添加的模型，设为选中
                    bool ret = privateModelConfigPage->setSelectPrivateModelConfig(model);
                    if (ret == true) {
                        privateModelConfigPage->addModelItem(model, m_sourceGroup);
                        KyInfo() << model.modelName << " 添加选中模型成功";
                    } else {
                        KyInfo() << "nlp already exist";
                        labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                        return;
                    }
                } else {
                    // 已有选中模型，只更新验证信息和UI
                    bool ret = privateModelConfigPage->addPrivateModel(model);
                    if (ret == true) {
                        privateModelConfigPage->addModelItem(model, m_sourceGroup);
                        KyInfo() << model.modelName << " 更新模型验证信息成功";
                    } else {
                        KyInfo() << "nlp already exist";
                        labelErrorCustomName->setText(tr("The model type already exists. Cannot be repeated!"));
                        return;
                    }
                }
            }
        }
    }
    labelErrorCustomName->setText("");
    labelErrorModelName->setText("");
    labelErrorApiKey->setText("");
    labelErrorRequestUrl->setText("");
    return QDialog::accept();
}

void PrivateModelConfigDialog::moveToCenter() {
    KyInfo() << "moveToCenter";
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