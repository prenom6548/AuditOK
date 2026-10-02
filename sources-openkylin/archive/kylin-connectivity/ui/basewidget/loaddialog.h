#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <kdialog.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QList>
#include "loadanimation.h"

class LoadDialog : public QDialog
{
    Q_OBJECT
public:
    LoadDialog(QWidget *parent = nullptr);

    void setHighLight();

private:
    LoadAnimation *m_loadAnimation = nullptr;
Q_SIGNALS:
};

#endif // LOADDIALOG_H
