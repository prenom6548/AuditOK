#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QHBoxLayout>
#include <QTimer>
#include <QColor>
#include <QPalette>
#include <QGSettings/QGSettings>
#include <QApplication>
#include "utils.h"
#include "text_label.h"

/*
 * 分为底层和上层两个 QLabel，如果使用单个 QLabel 的 paintEvent 来实现的话，无法支持 html 文本的渲染，因为 paintEvent 只能绘制简单的文本。
 * 底层和上层的两个 QLabel 有相同的文本属性，只不过在底层的 paintEvnet 函数中获取正确的控件大小，在对上层的 QLabel 文本做处理
 */

namespace K {

const static int DEFAULT_PIXEL_SIZE = 14;    // 默认字号为 14 pixel size

TextLabel::TextLabel(QWidget *parent)
    : QLabel(parent),
      internal_label_(new QLabel(this)),
      pixel_size_(DEFAULT_PIXEL_SIZE),
      is_bold_(false),
      font_color_enum_(FontColor::kNone)
{
    auto hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->addWidget(internal_label_);
    setLayout(hbox_layout);

    // 监听字体，字号，主题变化
    auto *gsettings = new QGSettings("org.ukui.style", "", this);
    connect(gsettings, &QGSettings::changed, this, [this](QString key)  {
        // 设置字体和字号
        if (key == "system-font" ||
            key == "systemFont" ||
            key == "system-font-size" ||
            key == "systemFontSize")
        {
            SetPixelSize(pixel_size_, is_bold_);
        }

        // 当主题改变时，会重置调色板的颜色，导致之前设置的字体颜色失效。
        // 此处监听主题改变，当主题改变后重设字体颜色
        if (key == "style-name" || key == "styleName") {
            // 优先级: font_color_ -> font_color_enum
            if (font_color_.isValid()) {
                SetFontColor(font_color_);
            } else if (font_color_enum_ != FontColor::kNone) {
                SetFontColor(font_color_enum_);
            } else {
                // 没有给控件设置过字体颜色，不做处理
                ;
            }
        }
    });

    // 处理控制面板切换字体后不会同步更新的问题
    QTimer::singleShot(0, this, [this]() {
        SetPixelSize(pixel_size_, is_bold_);
    });
}

TextLabel::~TextLabel() {

}

void TextLabel::SetText(const QString &text) {
    text_ = text;
    setText(text);
    update();
}

void TextLabel::SetPixelSize(double pixel_size, bool is_bold) {
    pixel_size_ = pixel_size;
    is_bold_ = is_bold;
    QFont font;
    font.setPointSizeF(Utils::AdaptPixelSizeToPointSize(pixel_size));
    font.setBold(is_bold);
    setFont(font);
    internal_label_->setFont(font);
    update();
}

void TextLabel::SetFontColor(const QColor &color) {
    font_color_ = color;
    auto p = internal_label_->palette();
    p.setColor(QPalette::Text, color);
    setPalette(p);
    internal_label_->setPalette(p);
}

void TextLabel::SetFontColor(const enum FontColor color) {
    font_color_enum_ = color;

    QColor target_color;
    switch (color) {
        case FontColor::kPlaceholderText: {
            target_color = Utils::CurrentPlaceholderTextColor();
            break;
        }
        case FontColor::kText: {
            target_color = qApp->palette().color(QPalette::ColorRole::Text);
            break;
        }
        default: {
            qCritical() << "Text label set font color: unsupported specified color";
            break;
        }
    }

    if (target_color.isValid()) {
        auto p = internal_label_->palette();
        p.setColor(QPalette::Text, target_color);
        setPalette(p);
        internal_label_->setPalette(p);
    }
}

void TextLabel::HandleText(std::function<QString(QString)> handle_text_func) {
    handle_text_func_ = handle_text_func;
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
