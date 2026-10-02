#include "androiditem.h"

#include <QFont>
#include <QVariant>
#include <QEvent>
#include <QFontMetrics>
#include <QDebug>

const int ITEM_ICON_WIDTH = 42;
const int ITEM_ICON_HEIGHT = 42;
const int ITEM_W = 205;
const int ITEM_H = 80;
const QString ITEM_ICON_PATH = ":/filemanageview/";
const QStringList ICON_LIST = {"pictureIcon.png", "videoIcon.png", "musicIcon.png",     "documentIcon.png",
                               "weChatIcon.png",  "qqIcon.png",    "mobileFileIcon.png"};

AndroidItem::AndroidItem(AndroidItem::Type type, QWidget *parent) : QPushButton(parent), m_type(type)
{
    setProperty("useButtonPalette", true);
    setFixedSize(ITEM_W, ITEM_H);
    setFocusPolicy(Qt::NoFocus);
    connect(this, &QPushButton::clicked, this, [=]() { Q_EMIT sigBtnClicked(m_type); });
    // 项按钮
    m_itemIconLabel = new QLabel(this);
    m_itemIconLabel->setFixedSize(ITEM_ICON_WIDTH, ITEM_ICON_HEIGHT);
    m_itemIconLabel->setStyleSheet("background-color:transparent;");
    // 项名称
    m_itemNameLabel = new QLabel(this);
    QFont font;
    font.setBold(true);
    m_itemNameLabel->setFont(font);
    m_itemNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_itemNameLabel->setMinimumHeight(30);
    m_itemNameLabel->setStyleSheet("background-color:transparent;");

    // 每一项总数
    m_itemCountLabel = new QLabel(this);
    m_itemCountLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_itemNameLabel->setMinimumHeight(30);
    font.setBold(false);
    font.setPointSizeF(12);
    m_itemCountLabel->setFont(font);
    m_itemCountLabel->setStyleSheet("background-color:transparent;color:#8F9399;");

    QVBoxLayout *labelVLayout = new QVBoxLayout();
    labelVLayout->setSpacing(0);
    labelVLayout->setMargin(0);
    labelVLayout->addStretch();
    labelVLayout->addWidget(m_itemNameLabel);
    labelVLayout->addSpacing(4);
    labelVLayout->addWidget(m_itemCountLabel);
    labelVLayout->addStretch();

    QHBoxLayout *mainHLayout = new QHBoxLayout();
    mainHLayout->setContentsMargins(19, 10, 19, 10);
    mainHLayout->setSpacing(0);
    mainHLayout->addWidget(m_itemIconLabel);
    mainHLayout->addSpacing(11);
    mainHLayout->addLayout(labelVLayout);
    mainHLayout->addStretch();
    setLayout(mainHLayout);
    initType();
}
void AndroidItem::initType()
{
    setItemCount(0);
    switch (m_type) {
        case AndroidItem::Type::Picture: {
            m_itemNameLabel->setText(tr("Picture"));
        } break;
        case AndroidItem::Type::Video:
            m_itemNameLabel->setText(tr("Video"));
            break;
        case AndroidItem::Type::Music:
            m_itemNameLabel->setText(tr("Music"));
            break;
        case AndroidItem::Type::Doc:
            m_itemNameLabel->setText(tr("Document"));
            break;
        case AndroidItem::Type::WeChat:
            m_itemNameLabel->setText(tr("WeChat"));
            break;
        case AndroidItem::Type::QQ:
            m_itemNameLabel->setText(tr("QQ"));
            break;
        case AndroidItem::Type::File:
            m_itemNameLabel->setText(tr("Mobile"));
            setItemCount();
            break;
    }
    m_itemIconLabel->setPixmap(
        QIcon(ITEM_ICON_PATH + ICON_LIST.value(m_type)).pixmap(ITEM_ICON_WIDTH, ITEM_ICON_HEIGHT));
}

// 设置项总数
void AndroidItem::setItemCount(int count)
{
    QString text = "";
    if (count == -1) {
        text = tr("All File");
    } else {
        text = QString("%1 ").arg(count) + QString(tr("items"));
    }
    m_itemCountLabel->setText(text);
}

void AndroidItem::setTheme(PublicAttributes::Theme theme)
{
    switch (theme) {
        case PublicAttributes::Theme::Light: {
            setStyleSheet("QPushButton "
                          "{border:0px;border-radius:8px;background-color:#F6F6F6;}"
                          "QPushButton:Hover"
                          "{border:0px;border-radius:8px;background-color:#E6F1FE;}"
                          "QPushButton:Pressed "
                          "{border:0px;border-radius:8px;background-color:#E1E1E1;}");
            break;
        }
        case PublicAttributes::Theme::Dark: {
            setStyleSheet("QPushButton "
                          "{border:0px;border-radius:8px;background-color:#626267;}"
                          "QPushButton:Hover"
                          "{border:0px;border-radius:8px;background-color:#373737;}"
                          "QPushButton:Pressed "
                          "{border:0px;border-radius:8px;background-color:#414141;}");
            break;
        }
    }
}

void AndroidItem::changeFontSize(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize - 2);
    m_itemCountLabel->setFont(font);
    font.setPointSizeF(fontSize);
    font.setBold(true);
    QFontMetrics fm1(m_itemNameLabel->font());
    int oldWidth = fm1.width(m_itemNameLabel->text());
    m_itemNameLabel->setFont(font);

    if (m_type == Type::File) {
        QFontMetrics fm2(m_itemNameLabel->font());
        int w = fm2.width(m_itemNameLabel->text()) - oldWidth;
        if (w == 0) {
            if (fm2.width(m_itemNameLabel->text()) + 100 > ITEM_W) {
                setFixedWidth(fm2.width(m_itemNameLabel->text()) + 100);
            }
        } else {
            setFixedWidth(width() + w);
        }
    }
}
