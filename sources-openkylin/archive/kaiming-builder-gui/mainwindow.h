#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QScopedPointer>

#include <packageframe.h>
#include <globalinfo.h>
#include "logic/KMBuilderApi.h"

class MainWindowPrivate;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

    void centerToScreen();

    void closeBtn();

protected:
    void changeEvent(QEvent *event);

private:
    Q_DECLARE_PRIVATE(MainWindow)
    QScopedPointer<MainWindowPrivate> d_ptr;
};
#endif // MAINWINDOW_H
