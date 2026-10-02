#ifndef EDITIONDIRINFO_H
#define EDITIONDIRINFO_H
#include <QString>
#include <QMap>
#include <QPushButton>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include "pushbuttoncheck.h"

// class PushButtonCheck : public QPushButton
// {
//     Q_OBJECT
// public:
//     explicit PushButtonCheck(QString str, QWidget *parent = nullptr){
//         QHBoxLayout* layout = new QHBoxLayout(this);
//         m_labelpix = new QLabel(this);
//         m_labelpix->setPixmap(QPixmap(":/res/png/clicked.svg").scaled(24, 24));
//         this->setLayout(layout);
// //        layout->setAlignment(this, Qt::AlignLeft);
//         this->setText(str);
//         layout->addStretch();
//         layout->addWidget(m_labelpix);
//         this->setCheckable(true);

//         m_labelpix->setVisible(false);
//         connect(this,&PushButtonCheck::clicked,this,&PushButtonCheck::clicked_slot);
//     }

// public:
//     QLabel* m_labelpix;

//     void setLabelShow(bool flag){
//         m_labelpix->setVisible(flag);
//         if(flag) {
//             this->setChecked(true);
//             //this->setFocus();
//         }else
//             this->setChecked(false);
//     }
// //   void addStyleSheet();

//  public slots:
//     void clicked_slot(){
//         emit clicked_obj(this);
//     }
//     void other_clicked_slot(QString objname){
//         qDebug()<<"objectname===="<<objectName()<<"======"<<objname;
//         if(this->objectName().compare(objname)!=0)
//         {
//             setLabelShow(false);
//         }
//         else
//         {
//             setLabelShow(true);
//         }
//     }
// signals:
//     void clicked_obj(PushButtonCheck* obj);
// };

class editionDirInfo: public QObject//某个版本
{
    Q_OBJECT

public:
    editionDirInfo();
    void translater(QString langue);
    void setChecked(bool isSet);

signals:
    void checkButton_sig(editionDirInfo* objname);

public slots:
    void other_clicked_slot(editionDirInfo*  objname);

public:
    //conf配置文件内容
    QString m_edition;
    QMap<QString,QString> m_LangtoNameMap;//此版本全部显示语言列表。key为语言标志如zh_CN，value为从conf文件内读取出的对应此语言的名称
    QString m_script;
    QString m_default;

    //扩展内容
    QString m_filepath;//镜像内路径（含conf配置文件名）
//    QPushButton* m_EditionButton=NULL;
    PushButtonCheck* m_EditionButton=NULL;


};

#endif // EDITIONDIRINFO_H
