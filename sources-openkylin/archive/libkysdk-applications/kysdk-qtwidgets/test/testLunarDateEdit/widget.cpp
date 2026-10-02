#include "widget.h"
#include "klunardateedit.h"
#include <QVBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->setFixedSize(300, 150);
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_pLunarCheckBox = new QCheckBox(this);
    m_pLunarCheckBox->setObjectName(QString::fromUtf8("checkBox"));
    m_pLunarCheckBox->setGeometry(QRect(120, 80, 82, 21));
    m_pLunarCheckBox->setChecked(false);
    m_pLunarCheckBox->setText("Lunar");

    m_pLunarDateEdit = new KLunarDateEdit(this);
    m_pLunarDateEdit->setFixedSize(140, 36);

    layout->addWidget(m_pLunarCheckBox);
    layout->addWidget(m_pLunarDateEdit);

    // 用法，拿到DateEdit指针，然后像正常DateEdit控件一样用，农历会跟随刷新
    m_pLunarDateEdit->getDateEdit()->setDate(QDate::currentDate());

    QMetaObject::connectSlotsByName(this);
}

Widget::~Widget()
{
}

void Widget::on_checkBox_clicked(bool checked)
{
    m_pLunarDateEdit->setShowLunar(checked);
}
