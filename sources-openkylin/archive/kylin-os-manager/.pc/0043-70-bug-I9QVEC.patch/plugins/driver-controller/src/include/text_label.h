#ifndef K_LABEL_H
#define K_LABEL_H

#include <functional>
#include <QLabel>
#include <QString>

namespace K {

class TextLabel: public QLabel {
    Q_OBJECT
public:
    TextLabel(QWidget *parent = nullptr);
    ~TextLabel();
    void SetText(const QString &text);
    void HandleText(std::function<QString(QString)> handle_text_func);
    void SetPixelSize(double pixel_size);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double AdaptPixelSizeToPointSize(double pixel_size);

    QString text_;
    std::function<QString(QString)> handle_text_func_;
    QLabel *internal_label_;
    double pixel_size_;
};

}

#endif
