#ifndef BUILDRESULTWIDGET_H
#define BUILDRESULTWIDGET_H

#include "../fileProcess/fileprocess.h"
#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <kaboutdialog.h>
#include <QHBoxLayout>
#include <QUrl>
#include <QDesktopServices>
#include <QIcon>

class buildResultWidget : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit buildResultWidget(QWidget *parent = nullptr);

signals:
    void gohomesignals();

private:
};

#endif // BUILDRESULTWIDGET_H
