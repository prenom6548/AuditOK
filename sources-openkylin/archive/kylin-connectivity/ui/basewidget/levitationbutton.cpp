#include "levitationbutton.h"
#include <QPainter>
#include <QStyleOption>
#include <QBitmap>
#include <QHBoxLayout>

const int ICON_W = 16;
const int ICON_H = 16;

LevitationButton::LevitationButton(QWidget *parent) : QPushButton(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);
    m_icon = new QLabel(this);
    m_icon->setFixedSize(ICON_W, ICON_H);
    m_text = new QLabel(this);
    m_text->setStyleSheet("color:#FFFFFF;text-align:center;");
    m_text->setMargin(0);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_icon);
    layout->addSpacing(8);
    layout->addWidget(m_text);
    layout->setContentsMargins(16, 16, 16, 16);
    setLayout(layout);

    setStyleSheet("QPushButton"
                  "{border:1px;border-radius:24px;background-color:#373737;}"
                  "QPushButton:Hover"
                  "{border:1px;border-radius:24px;background-color:#5F5F5F;}"
                  "QPushButton:Pressed"
                  "{border:1px;border-radius:24px;background-color:#414141;}");
}

void LevitationButton::setIcon(QString iconName)
{
    m_icon->setPixmap(QPixmap(iconName));
}

void LevitationButton::setText(QString text)
{
    m_text->setText(text);
}

void LevitationButton::changeFontSize(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize);

    m_text->setFont(font);
    QFontMetrics fontmts = QFontMetrics(font);
    int width = fontmts.width(m_text->text()) + ICON_W + 8 + 32;
    int height = fontmts.height() + 28;
    setFixedSize(width, height);
    QString sizeStr = QString::number(height / 2);
    setStyleSheet(QString("QPushButton"
                          "{border:1px;border-radius:%1px;background-color:#373737;}"
                          "QPushButton:Hover"
                          "{border:1px;border-radius:%2px;background-color:#5F5F5F;}"
                          "QPushButton:Pressed"
                          "{border:1px;border-radius:%3px;background-color:#414141;}")
                      .arg(sizeStr, sizeStr, sizeStr));
}