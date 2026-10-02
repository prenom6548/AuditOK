#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "kaboutdialog.h"

class AboutDialog : public kdk::KAboutDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();
};

#endif // ABOUTDIALOG_H
