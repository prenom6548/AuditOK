#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#include <QWidget>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
