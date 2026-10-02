#ifndef TIPWINDOW_H
#define TIPWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QDialog>

class TipWindow : public QDialog
{
    Q_OBJECT
public:
    explicit TipWindow(QWidget *parent = nullptr);
    QWidget *mainWid = nullptr;
    QLabel *tipLabel = nullptr;
    QHBoxLayout *hLayout = nullptr;
    void setTipWindow();

signals:
};

#endif // TIPWINDOW_H
