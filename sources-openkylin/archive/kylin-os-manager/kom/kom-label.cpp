#include <QPainter>
#include <QDebug>
#include <QFontMetrics>
#include <QFont>
#include <QVariant>
#include <QBoxLayout>
#include <QPaintDevice>
#include <QApplication>
#include "kom-ukui-gsettings.h"
#include "kom-utils.h"
#include "kom-label_p.h"
#include "kom-label.h"

namespace kom {

KomLabelPrivate::KomLabelPrivate(KomLabel *q)
    : q_ptr(q),
      m_label(new QLabel(q_ptr)),
      m_isBold(false),       // 默认不是粗体
      m_fontPixelSize(14),   // 默认 14 pixel size
      m_transparency(1) {    // 默认不透明
    QHBoxLayout *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(m_label);
    q_ptr->setLayout(hbox_layout);
}

KomLabelPrivate::~KomLabelPrivate() {}

void KomLabelPrivate::adaptText(void) {
    Q_Q(KomLabel);

    // 设置字体
    QFont font;
    font.setBold(m_isBold);
    font.setPointSizeF(kom::KomUtils::adaptFontSize(m_fontPixelSize));
    q_ptr->QLabel::setFont(font);
    m_label->setFont(font);

    // 设置调色板
    auto palette = qApp->palette();
    auto color = palette.color(QPalette::WindowText);
    color.setAlphaF(m_transparency);
    palette.setColor(QPalette::WindowText, color);
    q_ptr->QLabel::setPalette(palette);
    m_label->setPalette(palette);

    QString showText;
    QFontMetrics fm(font);
    if (fm.horizontalAdvance(m_text) - q_ptr->QLabel::width() > 0) {
        showText = fm.elidedText(m_text, Qt::ElideRight, q_ptr->QLabel::width());
        if (showText == m_text) {
            m_label->setToolTip("");
        } else {
            m_label->setToolTip(m_text);
        }
    } else {
        showText = m_text;
        m_label->setToolTip("");
    }

    m_label->setText(showText);
}

void KomLabelPrivate::setAlignment(Qt::Alignment alignment) {
    q_ptr->QLabel::setAlignment(alignment);
    m_label->setAlignment(alignment);
}

KomLabel::KomLabel(QWidget *parent) : QLabel(parent), d_ptr(new KomLabelPrivate(this)) {
    connect(UkuiGsettings::getInstance(), &UkuiGsettings::fontSizeChange, this, [=](QVariant fontSize) {
        Q_D(KomLabel);
        setText(d->m_text);
    });
}

KomLabel::~KomLabel() {}

void KomLabel::setAlignment(Qt::Alignment alignment) {
    Q_D(KomLabel);
    d->setAlignment(alignment);
}

void KomLabel::setBold(bool enable) {
    Q_D(KomLabel);
    d->m_isBold = enable;
}

void KomLabel::setFontSize(int pixelSize) {
    Q_D(KomLabel);
    d->m_fontPixelSize = pixelSize;
}

void KomLabel::setTransparency(float transparency) {
    Q_D(KomLabel);
    d->m_transparency = transparency;
}

void KomLabel::setText(const QString &text) {
    Q_D(KomLabel);
    d->m_text = text;
    QLabel::setText(text);    // 底层 label 设置原始文本，以便让底层 label 控件伸展到合适的大小
    update();    // 刷新底层 label 控件，同时适配上层 label 控件的文本显示
}

void KomLabel::paintEvent(QPaintEvent *e) {
    Q_D(KomLabel);
    d->adaptText();
}

}
