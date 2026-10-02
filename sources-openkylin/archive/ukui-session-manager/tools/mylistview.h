#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include <QObject>
#include <QListView>

class MyListView : public QListView
{
    Q_OBJECT
public:
    MyListView(QWidget *parent = nullptr);
    ~MyListView();
};

#endif // MYLISTVIEW_H
