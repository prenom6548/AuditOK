#include "customexpandbutton.h"

CustomExpandButton::CustomExpandButton(QList<QPushButton *> pushBtnList, QWidget *parent)
    : QWidget(parent),
      m_pushBtnList(pushBtnList),
      m_mainVLayout(new QVBoxLayout()),
      m_btnHLayout(new QHBoxLayout()),
      m_directBtn(new kdk::KBorderlessButton()){
    initGui();
    initConnect();
}

void CustomExpandButton::initGui() {
    if (m_pushBtnList.length() > 1){
        m_directBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
        m_btnHLayout->setSpacing(0);
        m_btnHLayout->setContentsMargins(0, 0, 0, 0);
        m_btnHLayout->addWidget(m_pushBtnList[0]);
        m_btnHLayout->addSpacing(-20);
        m_btnHLayout->addWidget(m_directBtn);
        m_mainVLayout->setSpacing(0);
        m_mainVLayout->setContentsMargins(0, 0, 0, 0);
        m_mainVLayout->addLayout(m_btnHLayout);
        for (int i = 1; i < m_pushBtnList.length(); ++i) {
            m_mainVLayout->addWidget(m_pushBtnList[i]);
            m_pushBtnList[i]->hide();
        }
        this->setLayout(m_mainVLayout);
    }else{
        m_mainVLayout->addWidget(m_pushBtnList[0]);
        this->setLayout(m_mainVLayout);
    }
}

void CustomExpandButton::initConnect()
{
    connect(m_directBtn, &QPushButton::clicked, [=](){
        if(m_directBtnStatus == false){
            for (int i = 1; i < m_pushBtnList.length(); ++i) {
                m_pushBtnList[i]->show();
            }
            m_directBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
            m_directBtnStatus = true;
        }else if(m_directBtnStatus){
            for (int i = 1; i < m_pushBtnList.length(); ++i) {
                m_pushBtnList[i]->hide();
            }
            m_directBtn->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
            m_directBtnStatus = false;
        }
    });
}

