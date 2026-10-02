#ifndef GRIDLINEPAINT_H
#define GRIDLINEPAINT_H

#include <QWidget>
#include "currentdeviceinfo.h"

class GridLinePaint : public QWidget
{
    Q_OBJECT
public:
    explicit GridLinePaint(QWidget *parent = nullptr);

Q_SIGNALS:

protected:
    virtual void paintEvent(QPaintEvent *event);

};

#endif // GRIDLINEPAINT_H
