#ifndef PICTUREVIEWPAGE_H
#define PICTUREVIEWPAGE_H


#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QGridLayout>
#include <QListWidget>
#include <QDir>
#include <QStackedLayout>
#include <QScrollBar>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListView>
//#include "camerapage.h"

class PictureViewPage:public QWidget
{
    Q_OBJECT
//  Q_DECLARE_METATYPE(ItemData)
public:

    PictureViewPage(QWidget *parent = 0);
    QPixmap *map = nullptr;
    QListWidget *listWidget = nullptr;
    QListView *listview = nullptr;
    QPushButton *deleteBtn = nullptr;
    void createListWidget(QString &);
    void initListWidget();
    void initListView();
public Q_SLOTS:
//  void  itemclicked(QListWidgetItem*);

private:

    QLabel *text = nullptr;
    QWheelEvent *wheel = nullptr;
    QListWidgetItem *item = nullptr;
    QHBoxLayout *hlayout = nullptr;
    QWidget *title = nullptr;
    QWidget *btnpage = nullptr;
    QVBoxLayout *vlayout = nullptr;
    QGridLayout *grid = nullptr;
    QHBoxLayout *btnlayout = nullptr;
    QLabel *imageLabel = nullptr;
    QStackedLayout *stackWidgetLayout = nullptr;
};

#endif // PICTUREVIEWPAGE_H
