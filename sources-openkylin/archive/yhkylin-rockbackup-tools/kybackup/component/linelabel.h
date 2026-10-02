#ifndef LINELABEL_H
#define LINELABEL_H

#include <QLabel>
#include <QSize>
#include <QColor>
#include "../../common/mydefine.h"

/**
 * @brief 目前仅支持横线
 */
class LineLabel : public QLabel
{
    Q_OBJECT
public:
    LineLabel(QWidget* parent = nullptr, QColor color = QColor(COLOR_GRAY));
    virtual ~LineLabel();

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QColor m_color;
};

#endif // LINELABEL_H
