#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QRect>
#include <QColor>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    MyLabel(QWidget* parent = nullptr);
    MyLabel(const QString& text, QWidget* parent = nullptr, Qt::Alignment align = Qt::AlignCenter);
    virtual ~MyLabel();

    const QString & getOriginalText() const { return m_text; }
    void setDeplayText(const QString& text) { m_text = text; this->setText(text);}
    void setElidedText(const QString& text, Qt::TextElideMode mode = Qt::ElideRight);
    void setMaxLenText(const QString& text, int width);
    void setFontColor(QColor color);
    void setFontSize(int size);
    void setFontWordWrap(bool on);
    void setIsOriginal(bool isOriginal) { m_isOriginal = isOriginal; }

protected:
    // 主要因为字体变化而重绘，故暂不重写paintEvent，改为在构造方法中监控主题的字体（家族和大小）变化
//     void paintEvent(QPaintEvent *event);

private:
    void setMaxLenText();

private:
    QString m_text;
    bool m_bWordWrap = false;
    int m_width = 0;
    int m_height = 0;
    int m_maxWidth = 0;
    bool m_isOriginal = false;
    QRect m_rect;
    QColor m_fontColor;

    // 背景及字体颜色是否自动跟随主题
    bool m_bAutoTheme;
};

#endif // MYLABEL_H
