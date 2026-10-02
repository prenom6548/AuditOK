/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#include "ipitem.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QValidator>
#include <QEvent>
#include <QKeyEvent>
#include <QRegExp>
#include <QGSettings>

IpItem::IpItem(QFrame *parent)
    : SettingsItem(parent)
    , m_titleLabel(new QLabel)
{
    //m_titleLabel->setFixedWidth(120);
    m_titleLabel->adjustSize();
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_titleLabel->setFixedWidth(150);

//    m_dot1 = new QLabel;
//    m_dot1->setObjectName("dot1");
//    m_dot1->setAlignment(Qt::AlignCenter);
//    m_dot1->setText(".");

//    m_dot2 = new QLabel;
//    m_dot2->setObjectName("dot2");
//    m_dot2->setAlignment(Qt::AlignCenter);
//    m_dot2->setText(".");

//    m_dot3 = new QLabel;
//    m_dot3->setObjectName("dot3");
//    m_dot3->setAlignment(Qt::AlignCenter);
//    m_dot3->setText(".");

//    m_ipEdit1 = new QLineEdit;
//    m_ipEdit1->setFixedSize(50, 36);
//    m_ipEdit1->setObjectName("ipEdit1");
//    m_ipEdit1->setAlignment(Qt::AlignCenter);
//    m_ipEdit1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    connect(m_ipEdit1, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

//    m_ipEdit2 = new QLineEdit;
//    m_ipEdit2->setFixedSize(50, 36);
//    m_ipEdit2->setObjectName("ipEdit2");
//    m_ipEdit2->setAlignment(Qt::AlignCenter);
//    m_ipEdit2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    connect(m_ipEdit2, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

//    m_ipEdit3 = new QLineEdit;
//    m_ipEdit3->setFixedSize(50, 36);
//    m_ipEdit3->setObjectName("ipEdit3");
//    m_ipEdit3->setAlignment(Qt::AlignCenter);
//    m_ipEdit3->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    connect(m_ipEdit3, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

//    m_ipEdit4 = new QLineEdit;
//    m_ipEdit4->setFixedSize(50, 36);
//    m_ipEdit4->setObjectName("ipEdit4");
//    m_ipEdit4->setAlignment(Qt::AlignCenter);
//    m_ipEdit4->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    connect(m_ipEdit4, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

//    QRegExp regExp("(2[0-5]{2}|2[0-4][0-9]|1?[0-9]{1,2})");
//    QRegExpValidator *validator = new QRegExpValidator(regExp, this);
//    m_ipEdit1->setValidator(validator);
//    m_ipEdit2->setValidator(validator);
//    m_ipEdit3->setValidator(validator);
//    m_ipEdit4->setValidator(validator);

//    m_ipEdit1->installEventFilter(this);
//    m_ipEdit2->installEventFilter(this);
//    m_ipEdit3->installEventFilter(this);
//    m_ipEdit4->installEventFilter(this);

//    QStringList qssStyle;
//    qssStyle.append(QString("QFrame#ipFrame{border:1px solid %1;border-radius:%2px;}").arg("#A6B5B8").arg(5));
//    qssStyle.append(QString("QLabel#dot1,#dot2,#dot3{min-width:15px;background-color:%1;}").arg("#FFFFFF"));
//    qssStyle.append(QString("QLineEdit{background-color:%1;border:none;}").arg("#FFFFFF"));
//    qssStyle.append(QString("QLineEdit#ipEdit1{border-top-left-radius:%1px;border-bottom-left-radius:%1px;}").arg(5));
//    qssStyle.append(QString("QLineEdit#ipEdit4{border-top-right-radius:%1px;border-bottom-right-radius:%1px;}").arg(5));
//    frame->setStyleSheet(qssStyle.join(""));

    QFrame *frame = new QFrame;
    frame->setObjectName("ipFrame");
    QRegExp regExp("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    QRegExpValidator *validator = new QRegExpValidator(regExp, this);
    m_ipEdit = new QLineEdit;
    m_ipEdit->setFixedSize(220,36);
    m_ipEdit->setObjectName("ipEdit");
    m_ipEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_ipEdit->setValidator(validator);
    connect(m_ipEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

    frame->setStyleSheet("QFrame#ipFrame{border:1px solid #A6B5B8;} QLineEdit{border-radius:6px;}");
//    m_ipEdit->setStyleSheet("QLineEdit{border-radius:6px;}");

    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_titleLabel);
    layout->addSpacing(30);
//    layout->addWidget(m_ipEdit1);
//    layout->addWidget(m_dot1);
//    layout->addWidget(m_ipEdit2);
//    layout->addWidget(m_dot2);
//    layout->addWidget(m_ipEdit3);
//    layout->addWidget(m_dot3);
//    layout->addWidget(m_ipEdit4);
    layout->addWidget(m_ipEdit);
    layout->addStretch();

    setLayout(layout);
//    setFixedSize(450, 52);
//setFixedHeight(36);
//    m_ipEdit1->setText("128");
//    m_ipEdit2->setText("128");
//    m_ipEdit3->setText("0");
//    m_ipEdit4->setText("1");
    m_ipEdit->setText("128.128.0.1");
    connect(m_ipEdit, &QLineEdit::textChanged, [=]() {emit BtnEnable();}); 
}

void IpItem::setTitle(const QString &title)
{
    QFont ft;
    QFontMetrics fm(ft);
    QString elided_text = fm.elidedText(title, Qt::ElideRight, 150);
    m_titleLabel->setText(elided_text);
    if (fm.width(title) > m_titleLabel->width()) {
        m_titleLabel->setToolTip(title);
    }
    const QByteArray styleid("org.ukui.style");
    if (QGSettings::isSchemaInstalled(styleid)) {
        QGSettings *stylesettings = new QGSettings(styleid, QByteArray(), this);
        connect(stylesettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "systemFontSize" || key == "systemFont") {
                QFont ft;
                QFontMetrics fm(ft);
                QString elided_text1 = fm.elidedText(title, Qt::ElideRight, 150);
                m_titleLabel->setText(elided_text1);
                if (fm.width(title) > m_titleLabel->width()) {
                    m_titleLabel->setToolTip(title);
                }
            }
        });
    }
}

//bool IpItem::eventFilter(QObject *watched, QEvent *event)
//{
//    if (event->type() == QEvent::KeyPress) {
//        QLineEdit *txt = (QLineEdit *)watched;
//        if (txt == m_ipEdit1 || txt == m_ipEdit2 || txt == m_ipEdit3 || txt == m_ipEdit4) {
//            QKeyEvent *key = (QKeyEvent *)event;
//            if (key->text() == ".") {
//                this->focusNextChild();
//            }

//            if (key->key() == Qt::Key_Backspace) {
//                if (txt->text().length() <= 1) {
//                    this->focusNextPrevChild(false);
//                }
//            }
//        }
//    }

//    return QWidget::eventFilter(watched, event);
//}

void IpItem::textChanged(const QString &text)
{
    m_ip = m_ipEdit->text();
//    int len = text.length();
//    int value = text.toInt();

//    if (len == 3) {
//        if (value >= 100 && value <= 255) {
//            this->focusNextChild();
//        }
//    }

//    m_ip = QString("%1.%2.%3.%4").arg(m_ipEdit1->text()).arg(m_ipEdit2->text()).arg(m_ipEdit3->text()).arg(m_ipEdit4->text());
}

QString IpItem::getIP() const
{
    return this->m_ip;
}

void IpItem::setIP(const QString &ip)
{
    QRegExp regExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    if (!regExp.exactMatch(ip)) {
        return;
    }

    if (this->m_ip != ip) {
        this->m_ip = ip;
        m_ipEdit->setText(ip);
//        QStringList list = ip.split(".");
//        m_ipEdit1->setText(list.at(0));
//        m_ipEdit2->setText(list.at(1));
//        m_ipEdit3->setText(list.at(2));
//        m_ipEdit4->setText(list.at(3));
    }
}

void IpItem::clear()
{
    m_ipEdit->clear();
    m_ipEdit->setText("128.128.0.1");
    m_ipEdit->setFocus();
//    m_ipEdit1->clear();
//    m_ipEdit2->clear();
//    m_ipEdit3->clear();
//    m_ipEdit4->clear();
//    m_ipEdit1->setFocus();
}
