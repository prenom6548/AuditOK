#ifndef SCROLLAREA_H
#define SCROLLAREA_H

#include <QScrollArea>

class ScrollArea:public QScrollArea
{
    Q_OBJECT
public:
    ScrollArea(QWidget *parent = nullptr);
    ~ScrollArea();
protected:
    bool focusNextPrevChild(bool next) override;
};

#endif // SCROLLAREA_H
