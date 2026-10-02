#ifndef WIDGET_H
#define WIDGET_H

#include "kdirtabbar.h"
#include "widget.h"
#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void getchildFolds(const QString &foldPath, QStringList &folds, QStringList &foldsName);
};
#endif // WIDGET_H
