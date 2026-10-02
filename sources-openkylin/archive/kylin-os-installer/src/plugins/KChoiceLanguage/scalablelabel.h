#ifndef SCALABLELABEL_H
#define SCALABLELABEL_H

#include <QLabel>
#include <QFont>

class ScalableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ScalableLabel(const QString &text = "", QWidget *parent = nullptr);
    // 设置固定高度并自动调整字体
    void setFixedHeight(int h);

protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void setText(const QString &text);

private:
    QFont originalFont;  // 存储原始字体，避免多次缩放导致失真
    int minFontSize = 6;  // 最小字体大小，避免字体过小无法阅读

    void adjustFont();
};

#endif // SCALABLELABEL_H
