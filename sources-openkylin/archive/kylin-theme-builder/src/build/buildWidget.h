#ifndef BUILDWIDGET_H
#define BUILDWIDGET_H

#include "build.h"
#include "buildresultwidget.h"
#include "../fileProcess/historyinfoload.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QThread>

class buildWidget : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit buildWidget(QWidget *parent = nullptr);

signals:
    void gohomesignals();

private:
    QVBoxLayout *m_buildLayout;
    QLabel *m_buildMessage;
    QProgressBar *m_buildProgressBar;

};

#endif // BUILDWIDGET_H
