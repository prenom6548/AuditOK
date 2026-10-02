#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>

class TestDialog : public QDialog
{
    Q_OBJECT
public:
    TestDialog(QWidget *parent = nullptr);
};

#endif // TESTDIALOG_H
