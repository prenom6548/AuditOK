#include "widget.h"
#include "kdirtabbar.h"
#include <QBoxLayout>
#include <QDateTime>
#include <QDebug>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QPainter>
#include <QPoint>
#include <QPushButton>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleOptionButton>
#include <QTextEdit>
#include <QToolButton>
#include <QWidget>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    KDirTabBar *bar = new KDirTabBar(this);
    QVBoxLayout *myLayout = new QVBoxLayout();
    QFileDialog *fileDialog = new QFileDialog(this);
    ;
    QPushButton *button1 = new QPushButton("open files", this);
    QLabel *label = new QLabel();
    QString path = QDir::currentPath();
    QFileInfo fileInfo(path);
    QList<QToolButton *> btns;
    KDirTabBar *bar2 = new KDirTabBar(this);

    bar->setCurrentPath(fileInfo.absoluteFilePath());
    label->setText(bar->currentPath());
    btns = bar->buttons();
    btns[btns.count() - 1]->setIcon(QIcon()); // 设置按钮的图标
    btns[btns.count() - 2]->setEnabled(false); // 设置按钮是否可用
    bar->setIcon(btns.count() - 2, QIcon::fromTheme("folder-pictures")); // 设置按钮的图标

    connect(bar, &KDirTabBar::tabBarClicked, [=]() {
        label->setText(bar->currentPath());
    });
    connect(button1, &QPushButton::clicked, [=] {
        fileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        fileDialog->setFileMode(QFileDialog::ExistingFiles);
        bar2->setCurrentPath(fileDialog->getExistingDirectory());
    });

    setLayout(myLayout);
    myLayout->addWidget(bar);
    myLayout->addStretch();
    myLayout->addWidget(label);
    myLayout->addStretch();
    myLayout->addWidget(button1);
    myLayout->addStretch();
    myLayout->addWidget(bar2);
    myLayout->addStretch();
}

Widget::~Widget()
{
}
