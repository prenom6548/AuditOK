#include <QDebug>
#include <QHBoxLayout>

#include "basedevice.h"
#include "iconhelper.h"
#include "toolform.h"

ToolForm::ToolForm(QWidget *parent) : QWidget(parent)
{
    m_menuBtn = new QPushButton(this);
    m_homeBtn = new QPushButton(this);
    m_returnBtn = new QPushButton(this);
    // 固定高度
    setFixedHeight(40);
    setAutoFillBackground(true);

    // 菜单按钮
    m_menuBtn->setFixedHeight(30);
    IconHelper::Instance()->SetIcon(m_menuBtn, QChar(0xf096), 15);
    m_menuBtn->setIconSize(QSize(16, 16));
    m_menuBtn->setProperty("isWindowButton", 0x1);
    m_menuBtn->setProperty("useIconHighlightEffect", 0x2);
    m_menuBtn->setFlat(true);
    m_menuBtn->setContentsMargins(0, 0, 0, 0);
    m_menuBtn->setFocusPolicy(Qt::NoFocus);
    // home按钮
    m_homeBtn->setFixedHeight(30);
    IconHelper::Instance()->SetIcon(m_homeBtn, QChar(0xf1db), 15);
    m_homeBtn->setIconSize(QSize(16, 16));
    m_homeBtn->setProperty("isWindowButton", 0x1);
    m_homeBtn->setProperty("useIconHighlightEffect", 0x2);
    m_homeBtn->setFlat(true);
    m_homeBtn->setContentsMargins(0, 0, 0, 0);
    m_homeBtn->setFocusPolicy(Qt::NoFocus);
    // 返回按钮
    m_returnBtn->setFixedHeight(30);
    IconHelper::Instance()->SetIcon(m_returnBtn, QChar(0xf053), 15);
    m_returnBtn->setIconSize(QSize(16, 16));
    m_returnBtn->setProperty("isWindowButton", 0x1);
    m_returnBtn->setProperty("useIconHighlightEffect", 0x2);
    m_returnBtn->setFlat(true);
    m_returnBtn->setContentsMargins(0, 0, 0, 0);
    m_returnBtn->setFocusPolicy(Qt::NoFocus);

    // 信号连接槽
    connectInit();

    // 整体布局
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(m_returnBtn);
    layout->addStretch();
    layout->addWidget(m_homeBtn);
    layout->addStretch();
    layout->addWidget(m_menuBtn);
    layout->addStretch();
    setLayout(layout);
}

ToolForm::~ToolForm() {}

void ToolForm::setDevice(BaseDevice *device)
{
    if (!device) {
        return;
    }
    m_device = device;
}


// 连接槽
void ToolForm::connectInit()
{
    connect(m_menuBtn, &QPushButton::clicked, this, &ToolForm::onClicked);
    connect(m_homeBtn, &QPushButton::clicked, this, &ToolForm::onClicked);
    connect(m_returnBtn, &QPushButton::clicked, this, &ToolForm::onClicked);
}


// 处理点击信号
void ToolForm::onClicked()
{
    QPushButton *clickBtn = qobject_cast<QPushButton *>(sender());
    if (clickBtn == m_menuBtn) {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postAppSwitch();
    } else if (clickBtn == m_homeBtn) {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postGoHome();
    } else if (clickBtn == m_returnBtn) {
        if (!m_device) {
            return;
        }
        Q_EMIT m_device->postGoBack();
    }
}
