#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QTreeWidget>
#include <QMouseEvent>

class CustomTreeWidget : public QTreeWidget {
public:
    CustomTreeWidget(QWidget *parent = nullptr);

    void customSort();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif //CUSTOMTREEWIDGET_H
