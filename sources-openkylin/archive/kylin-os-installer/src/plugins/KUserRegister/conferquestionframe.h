#ifndef CONFERQUESTION_H
#define CONFERQUESTION_H
#include <QObject>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QEvent>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QKeyEvent>

#include "QTextEdit"
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "ksystemsetting_unit.h"

namespace KInstaller {


class conferUi: public QObject//
{
    Q_OBJECT
public:
    conferUi(QWidget *parent = nullptr):QObject(parent){
        qlab1=new QLabel(parent);
        question1 = new QComboBox(parent);
        clab1=new QLabel(parent);
        context1 = new QLineEdit(parent);
       connect(question1,&QComboBox::currentTextChanged,this,&conferUi::qClickedMe);
       connect(context1,&QLineEdit::textChanged,this,&conferUi::cClickedMe);
    }
    ~conferUi(){}
    QLabel* qlab1;
    QComboBox* question1;
    QLabel* clab1;
    QLineEdit* context1;
    qint8 is_ok=0;//-1:没有填好问题，1：填好问题
signals:
    void ClickedSignal(conferUi* p);
public slots:
    void qClickedMe(QString str)
    {
        qDebug()<<"qClickedMe(QString str";
        if(str.isEmpty())
            context1->setDisabled(true);
        else
            context1->setDisabled(false);

            context1->setText("");
    }

    void cClickedMe(QString str){
         if(str.isEmpty())
             is_ok=0;
         else
             is_ok=1;
         emit ClickedSignal(this);

         qDebug()<<"cClickedMe(QString str";
     }
};


class conferquestion : public QWidget
{
    Q_OBJECT
public:
    explicit conferquestion(QWidget *parent = nullptr);
    ~conferquestion();
    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void setNextBtnStyle(); //检测下一步按钮的状态显示
    void setBoardTabOrder();

    QList<conferUi*>Lst;
    QCheckBox* later = nullptr;
    QLabel* m_mainTitle = nullptr;
    QLabel* m_subTitle = nullptr;
    QStringList questionlist;
    quint8 anserconunt=0;

signals:
    void enableNextButton(bool value);

public slots:
    void conferCheck( conferUi* confer);
    void conferLater();
    void translateStr();
     void checkactive(QWidget *p);

private:
   void changeEvent(QEvent *event);




    bool NextFlag = false;
};
}
#endif // CONFERQUESTION_H
