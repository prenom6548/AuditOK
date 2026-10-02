#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QHBoxLayout>
#include "kom-ukui-gsettings.h"
#include "text_label.h"

/*
 * 分为底层和上层两个 QLabel，如果使用单个 QLabel 的 paintEvent 来实现的话，无法支持 html 文本的渲染，因为 paintEvent 只能绘制简单的文本。
 * 所以在底层的 paintEvnet 函数中获取正确的控件大小，并且设置上层的 QLabel 文本
 */

namespace K {

const static int DEFAULT_PIXEL_SIZE = 14;    // 默认字号为 14 pixel size

TextLabel::TextLabel(QWidget *parent)
    : QLabel(parent),
      internal_label_(new QLabel(this)),
      pixel_size_(DEFAULT_PIXEL_SIZE)
{
    auto hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->addWidget(internal_label_);
    setLayout(hbox_layout);

    connect(kom::UkuiGsettings::getInstance(), &kom::UkuiGsettings::fontSizeChange, this, [this]() {
        SetPixelSize(pixel_size_);
    });
}

TextLabel::~TextLabel() {

}

void TextLabel::SetText(const QString &text) {
    text_ = text;
    update();
}

void TextLabel::SetPixelSize(double pixel_size) {
    if (pixel_size != DEFAULT_PIXEL_SIZE) {
        pixel_size_ = pixel_size;
        QFont font;
        font.setPointSizeF(AdaptPixelSizeToPointSize(pixel_size));
        internal_label_->setFont(font);
        update();
    }
}

void TextLabel::HandleText(std::function<QString(QString)> handle_text_func) {
    handle_text_func_ = handle_text_func;
}

double TextLabel::AdaptPixelSizeToPointSize(double pixel_size) {
    static int default_point_size = 10;

    // pixel size 转 point size
    QWidget widget;
    double dpi = widget.logicalDpiX();
    if (dpi < 1)
        dpi = 96.0;
    double point_size = pixel_size * 72 / dpi;

    // 适配 point size 到当前字号
    return point_size / default_point_size * kom::UkuiGsettings::getInstance()->getFontSize().toDouble();
}

void TextLabel::paintEvent(QPaintEvent *event) {
    // 超长省略，显示 tooltip
    QString show_text;
    auto font = internal_label_->font();
    QFontMetrics fm(font);
    if (fm.width(text_) - internal_label_->width() > 0) {
        show_text = fm.elidedText(text_, Qt::ElideRight, internal_label_->width());
        internal_label_->setToolTip(text_);
    } else {
        show_text = text_;
        internal_label_->setToolTip("");
    }

    // 处理截断后的文本
    if (handle_text_func_)
        show_text = handle_text_func_(show_text);
    internal_label_->setText(show_text);
}

}
