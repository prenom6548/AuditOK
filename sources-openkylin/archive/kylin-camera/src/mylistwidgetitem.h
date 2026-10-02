#ifndef MYLISTWIDGETITEM_H
#define MYLISTWIDGETITEM_H

#include <QListWidgetItem>
#include <QMenu>
#include "button.h"
class myListWidgetItem : public QListWidgetItem
{
public:
    myListWidgetItem();
    void QMouseEvent();
};

#endif // MYLISTWIDGETITEM_H
