#ifndef CIRCLELABEL_H
#define CIRCLELABEL_H

#include <QLabel>
#include <QPaintEvent>
#include <QColor>
#include "../../common/mydefine.h"

class CircleLable : public QLabel
{
    Q_OBJECT
public:
    CircleLable(const QString& text, QWidget* parent = nullptr, int size = 24, QColor backgroundColor = QColor(COLOR_GRAY));
    virtual ~CircleLable();
    void setText(const QString& text) {
        m_text = text;
        repaint();
    }
    void setTextColor(QColor color) {
        m_textColor = color;
        m_oldTextColor = m_textColor;
        repaint();
    }
    void setBackgroundColor(QColor backgroundColor);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QString m_text;
    QColor m_textColor;
    QColor m_oldTextColor;
    QColor m_backgroundColor;
    // 是否自动跟随主题
    bool m_bAutoTheme;
};

#endif // CIRCLELABEL_H
