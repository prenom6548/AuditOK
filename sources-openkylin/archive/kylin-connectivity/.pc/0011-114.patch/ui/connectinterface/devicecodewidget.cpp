#include "devicecodewidget.h"

#include "highlight-effect.h"

const int BUTTON_WIDTH = 56;
const int BUTTON_HEIGHT = 56;

DeviceCodeWidget::DeviceCodeWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
}

void DeviceCodeWidget::initUI()
{
    QVBoxLayout *mainVLayout = new QVBoxLayout();
    mainVLayout->setMargin(0);
    this->setLayout(mainVLayout);

    QHBoxLayout *connectCodeHLayout = new QHBoxLayout();
    QHBoxLayout *describeHLayout = new QHBoxLayout();
    QHBoxLayout *lineEditHLayout = new QHBoxLayout();
    QHBoxLayout *startConnectHLayout = new QHBoxLayout();

    QFont font;
    font.setBold(true);
    QLabel *connectCodeLab = new QLabel(this);
    connectCodeLab->setText(
        tr("Please enter the connection code of the other device"));  // Please enter the other party's connection code
    connectCodeLab->setFont(font);

    connectCodeHLayout->addStretch();
    connectCodeHLayout->addWidget(connectCodeLab);
    connectCodeHLayout->addStretch();
    connectCodeHLayout->setSpacing(0);

    QLabel *describeLab = new QLabel(this);
    describeLab->setText(tr("The connection code of the device can be obtained on the homepage of the other party's "
                            "'multi terminal collaboration' application"));
    QPalette palette = describeLab->palette();
    palette.setColor(QPalette::Text, QColor("#67676C"));
    describeLab->setPalette(palette);
    describeLab->setMinimumWidth(950);
    describeLab->setWordWrap(true);
    describeLab->setAlignment(Qt::AlignCenter);

    describeHLayout->addStretch();
    describeHLayout->addWidget(describeLab);
    describeHLayout->addStretch();

    lineEditHLayout->addStretch();
    for (int i = 0; i < MaxCode::Num; i++) {
        m_codeLines[i] = new DeviceCodeItem(this);
        lineEditHLayout->addWidget(m_codeLines[i]);
        connect(m_codeLines[i], &DeviceCodeItem::sigTextInput, this, &DeviceCodeWidget::slotTextInput);
        connect(m_codeLines[i], &DeviceCodeItem::sigBackspace, this, &DeviceCodeWidget::slotBackspace);
    }
    lineEditHLayout->addStretch();
    m_codeLines[0]->setInput();
    m_codeLines[0]->setHead();
    m_codeLines[MaxCode::Num - 1]->setEnd();
    connect(m_codeLines[MaxCode::Num - 1], &DeviceCodeItem::returnPressed, this, &DeviceCodeWidget::slotSendDeviceCode);

    m_startConnectBtn = new QPushButton(this);
    m_startConnectBtn->setFixedSize(160, 48);
    m_startConnectBtn->setText(tr("Connection"));  // Start Connection
    m_startConnectBtn->setFocusPolicy(Qt::NoFocus);
    m_startConnectBtn->setProperty("isImportant", true);
    m_startConnectBtn->setEnabled(false);

    startConnectHLayout->addStretch();
    startConnectHLayout->addWidget(m_startConnectBtn);
    startConnectHLayout->addStretch();

    mainVLayout->addStretch();
    mainVLayout->addLayout(connectCodeHLayout);
    mainVLayout->addSpacing(16);
    mainVLayout->addLayout(describeHLayout);
    mainVLayout->addSpacing(40);
    mainVLayout->addLayout(lineEditHLayout);
    mainVLayout->addSpacing(61);
    mainVLayout->addLayout(startConnectHLayout);
    mainVLayout->addStretch();

    connect(m_startConnectBtn, &QPushButton::clicked, this, &DeviceCodeWidget::slotSendDeviceCode);
}

void DeviceCodeWidget::setInput()
{
    m_codeLines[m_currentCodeIndex]->setInput();
}

void DeviceCodeWidget::slotSendDeviceCode()
{
    QString code = "";
    for (int i = 0; i < MaxCode::Num; i++) {
        QString str = m_codeLines[i]->text().remove(" ");
        code.append(str);
    }
    qInfo() << "The currently entered device code is " << code;
    Q_EMIT sigDeviceCode(code);
}

void DeviceCodeWidget::slotBackspace()
{
    if (m_currentCodeIndex > 0) {
        m_codeLines[m_currentCodeIndex]->clearText();
        m_codeLines[m_currentCodeIndex]->exitInput();
        if (m_currentCodeIndex < MaxCode::Num) {
            m_startConnectBtn->setEnabled(false);
        }
        m_currentCodeIndex--;
        m_codeLines[m_currentCodeIndex]->setInput();
        m_codeLines[m_currentCodeIndex]->clearText();
    }
}

void DeviceCodeWidget::slotTextInput(QString)
{
    if (m_currentCodeIndex <= MaxCode::Num - 1) {
        if (m_currentCodeIndex < MaxCode::Num - 1) {
            m_codeLines[m_currentCodeIndex]->exitInput();
        }
        m_currentCodeIndex++;
        if (m_currentCodeIndex == MaxCode::Num) {
            m_currentCodeIndex--;
            m_startConnectBtn->setEnabled(true);
            return;
        }
        m_codeLines[m_currentCodeIndex]->setInput();
    }
}