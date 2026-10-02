#ifndef MODESELECT_H
#define MODESELECT_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QList>
#include <QStringList>
#include <QButtonGroup>

namespace KInstaller{

class chiceBtn:public QPushButton
{
    Q_OBJECT
public:
    chiceBtn(QWidget *parent = nullptr,int nline = 2):QPushButton(parent) {
      QHBoxLayout* lay=new QHBoxLayout;
      lay->setContentsMargins(0, 0, 25, 0);
      this->setLayout(lay);
      lay->addItem(new QSpacerItem(24, 80, QSizePolicy::Minimum, QSizePolicy::Minimum));//bug134334界面实现与其他界面不一致

      mainTitle=new QLabel;
      mainTitle->setObjectName("chiceBtn_mainTitle");
      mainTitle->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
      mainTitle->setWordWrap(true);

//      if(1 == nline)
//      {
//          mainTitle->setFixedHeight();
//      }
//      else
      if(2 == nline)
      {
          subTitle=new QLabel;
          subTitle->setObjectName("chiceBtn_subTitle");
          subTitle->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
          subTitle->setWordWrap(true);//开启自动换行
      }

      QVBoxLayout* lay1=new QVBoxLayout;
      lay1->addWidget(mainTitle);
      if(2 == nline)
          lay1->addWidget(subTitle);
//      if(1 == nline)
//          lay1->setContentsMargins();
      lay->addLayout(lay1);

      iconLable=new QLabel;
      //iconLable->setPixmap(QPixmap(":/res/svg/checkpic.svg"));
      iconLable->setPixmap(QIcon(":/res/svg/checkpic.svg").pixmap(28,28));
      lay->addWidget(iconLable,1,Qt::AlignRight);
      iconLable->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
      iconLable->setVisible(false);
      setCheckable(true);
      setChecked(false);

      connect(this,&chiceBtn::clicked,[=] () {
          setChecked(true);
        iconLable->setVisible(true);clickedSignal(this);
      });
      this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    }
    void setBntValue(bool value)
    {
        setChecked(value);
        iconLable->setVisible(value);
    }
    void setTitleStr(quint8 order,QString str)
    {
     if(order==1)
         mainTitle->setText(str);
     else if(order==2)
         subTitle->setText(str);
    }
    QLabel *mainTitle;
    QLabel *subTitle;
    QLabel *iconLable;
    quint8 bnt_id=0;

signals:
    void clickedSignal(chiceBtn* bnt);

};


class modeselect:public QWidget
{
     Q_OBJECT
public:
    modeselect(QWidget *parent = nullptr);
    void initUI();
    void addStyleSheet();

    QLabel *m_mainTitle;
    QList<chiceBtn*>bntList;
    chiceBtn* currentBnt=nullptr;
    void initFocusPolicy();                     //初始化焦点序列
signals:
    void modeSelect(chiceBtn *bnt);
    void enableNextButton(bool value);
    void changeNextButton();
    void reSetFocusPolicySignal();
public slots:
    void translateStr();
    void changemodel(chiceBtn *bnt);
    void checkactive(QWidget *p);

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
public:
    bool m_firstfocusget=false;//首次焦点获取标志(首次获取时隐藏边框)
};
}
#endif // MODESELECT_H
