#include "basebinary.h"

BaseBinary::BaseBinary(QString value)
{
    m_labValue = value;

    init();
    initLayout();
}


void BaseBinary::init()
{
    // 创建控件
    for (int i = 0; i < 4; i++) {
        QLabel *btn = new QLabel(this);
        btn->setObjectName("kylin-calculator_QPushButton_btn");
        btn->setFont(QFont("SourceHanSansCN"));
        btn->setText(m_flag0);
        btn->setStyleSheet("color:#8C8C8C;font-size:14px;");
        btn->setEnabled(false);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_btnList.push_front(btn);
//        connect(btn, &QPushButton::clicked, this, &BaseBinary::onClicked);
    }

    m_lab = new QLabel(this);
    m_lab->setObjectName("kylin-calculator_QLabel_m_lab");
    m_lab->setFont(QFont("SourceHanSansCN"));
    m_lab->setText(m_labValue);
    m_lab->setStyleSheet("font-size:14px;");
    m_lab->setAlignment(Qt::AlignRight | Qt::AlignTop);

}

void BaseBinary::initLayout()
{
    m_hlayout = new QHBoxLayout();
    m_hlayout->addStretch();
    for (int i = 0; i < 4; i++) {
        m_hlayout->addWidget(m_btnList[i]);
    }
    m_hlayout->setSpacing(0);
    m_hlayout->setMargin(0);

    m_spaceLayout = new QHBoxLayout();
    m_spaceLayout->addStretch();
    m_spaceLayout->addWidget(m_lab);
    m_spaceLayout->addSpacing(3);

    m_vlayout = new QVBoxLayout();
    m_vlayout->addLayout(m_hlayout);
    m_vlayout->addLayout(m_spaceLayout);
    m_vlayout->addStretch();
    m_vlayout->setSpacing(0);
    m_vlayout->setMargin(0);

    this->setLayout(m_vlayout);
}

void BaseBinary::setData(QString value)
{
    int valueIndex = value.size() - 1;
    int listIndex = m_btnList.size() - 1;
    while (valueIndex >= 0) {
        m_btnList[listIndex]->setText(value.at(valueIndex));
        m_btnList[listIndex]->setStyleSheet("color:#8C8C8C;font-size:14px;");
        listIndex--;
        valueIndex--;
    }
}

QString BaseBinary::data()
{
    QString value;
    for (int i = 0; i < m_btnList.size(); i++) {
        value.append(m_btnList[i]->text());
    }

    return value;
}

void BaseBinary::clear()
{
    for (int i = 0; i < m_btnList.size(); i++) {
        m_btnList[i]->setText(m_flag0);
        m_btnList[i]->setStyleSheet("color:#8C8C8C;font-size:14px;");
    }
}

void BaseBinary::onClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (m_flag0 == btn->text()) {
        btn->setText(m_flag1);
    } else if (m_flag1 == btn->text()) {
        btn->setText(m_flag0);
    }
    btn->setStyleSheet("font-size:14px;");
}
