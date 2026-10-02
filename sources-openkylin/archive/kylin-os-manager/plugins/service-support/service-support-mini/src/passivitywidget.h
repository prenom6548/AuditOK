#ifndef PASSIVITY_WIDGET_H
#define PASSIVITY_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QProcess>
#include <kwidget.h>
#include <kballontip.h>
#include <kdialog.h>
#include "feedbackcommon.h"

class PassivityWidget: public kdk::KWidget, public FeedBackCommon {
    Q_OBJECT
public:
    PassivityWidget(QWidget *parent = nullptr);
    void run();

private Q_SLOTS:
    void creatFinish(FeedBackFinishType type, QString str);
    void creatFinishNoUI(FeedBackFinishType type, QString str);
    void on_UpdateBlacklist(bool ok);

private:
    void initUI();
};

#endif
