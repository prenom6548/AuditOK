#ifndef DETAILDIALOG_H
#define DETAILDIALOG_H
/*点击appwidget详情的弹窗*/
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QLineEdit>
//#include <ukcc/widgets/fixlabel.h>
#include "ukcc/widgets/fixlabel.h"
#include "appupdate.h"

class Detaildialog : public QDialog
{
    Q_OBJECT
public:
    explicit Detaildialog(QWidget *parent = nullptr);

    void logUi();
    ~Detaildialog();

    QLabel *logAppIcon;
    QLabel *logAppName;
    QLabel *logAppVerson;

    QWidget *logWidget;
    QWidget *logAppWidget;

    QHBoxLayout *logAppHLayout;
    QVBoxLayout *logAppVLayout;

    QVBoxLayout *logVLayout;

    QVBoxLayout *logMainLayout;

    QTextEdit *logContent;
    FixLabel *installContent;
    QFrame *contentFrame;

protected:
//    void mousePressEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
//    bool eventFilter(QObject *, QEvent *);
//    bool eventFilter(QObject *obj,QEvent *event);
};



#endif // DETAILDIALOG_H
