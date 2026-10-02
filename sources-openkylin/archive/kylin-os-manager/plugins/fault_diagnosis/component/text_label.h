#ifndef K_LABEL_H
#define K_LABEL_H

#include <functional>
#include <QLabel>
#include <QString>
#include <QColor>

namespace K {

class TextLabel: public QLabel {
    Q_OBJECT
public:
    enum class FontColor {
        kNone = 0,
        kPlaceholderText,
        kText,
    };

    TextLabel(QWidget *parent = nullptr);
    ~TextLabel();
    void SetText(const QString &text);
    void HandleText(std::function<QString(QString)> handle_text_func);
    void SetPixelSize(double pixel_size, bool is_bold = false);
    void SetFontColor(const QColor &color);
    void SetFontColor(const enum FontColor color);
    void SetAlignment(Qt::Alignment alignment);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString                         text_;
    std::function<QString(QString)> handle_text_func_;
    QLabel *                        internal_label_;
    double                          pixel_size_;
    bool                            is_bold_;
    QColor                          font_color_;
    enum FontColor                  font_color_enum_;
};

}

#endif
