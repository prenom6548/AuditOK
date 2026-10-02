#ifndef UISELFSERVICE_H
#define UISELFSERVICE_H

#include <QWidget>
#include "../constant.h"

class UiSelfService : public QWidget
{
    Q_OBJECT
public:
    UiSelfService(QWidget *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event);
    QStringList m_objectNameStrList;
};

#endif // UISELFSERVICE_H
