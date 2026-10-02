#include "binarykeyboary.h"

BinaryKeyboary::BinaryKeyboary()
{
    init();
}

void BinaryKeyboary::init()
{
    // 创建二进制显示控件
    for (int i = 0; i < 16; i++) {
        BaseBinary *baseBinary = new BaseBinary(QString::number(i*4));
        baseBinary->setObjectName("kylin-calculator_BaseBinary_baseBinary");
        m_baseList.push_front(baseBinary);
    }

    // 布局
    m_topLayoout = new QHBoxLayout();
    m_bottomLayout = new QHBoxLayout();

    for (int i = 0; i < 16; i++) {
        if (i < 8) {
            m_topLayoout->addWidget(m_baseList[i]);
        } else {
            m_bottomLayout->addWidget(m_baseList[i]);
        }
    }
    m_topLayoout->setSpacing(12);
    m_bottomLayout->setSpacing(12);
    m_topLayoout->setContentsMargins(4, 0, 4, 0);
    m_bottomLayout->setContentsMargins(4, 0, 4, 0);

    m_vlayout = new QVBoxLayout();
    m_vlayout->addLayout(m_topLayoout);
    m_vlayout->addSpacing(0);
    m_vlayout->addLayout(m_bottomLayout);
    m_vlayout->setMargin(2);

    this->setLayout(m_vlayout);
}

void BinaryKeyboary::setData(QString value)
{
    int valueIndex = value.size() - 1;
    int listIndex = m_baseList.size() - 1;
    while (valueIndex >= 0) {
        QString str = value.mid(valueIndex - 3, 4);
        m_baseList[listIndex]->setData(str);

        listIndex--;
        valueIndex = valueIndex - 4;
    }
}

QString BinaryKeyboary::data()
{
    QString value;
    for (int i = 0;i < m_baseList.size(); i++) {
        value.append(m_baseList[i]->data());
    }

    return value;
}

void BinaryKeyboary::clear()
{
    for (int i = 0;i < m_baseList.size(); i++) {
        m_baseList[i]->clear();
    }
}
