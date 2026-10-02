#ifndef APPCHECKPAGE_H
#define APPCHECKPAGE_H

#include <QWidget>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCheckBox>
#include <QList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "../PluginService/ksystemsetting_unit.h"
#include "QStringList"
#include "QDebug"



namespace KInstaller {
class AppCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    //path:传递的描述文件所在的目录
    explicit AppCheckBox(QString filepath,QWidget *parent = nullptr):QCheckBox(parent)
    {

         this->setObjectName("App_Cell");
        init_ok=false;
        FileCfg=QString(filepath);

        QFileInfo fil(FileCfg);
        if(fil.exists()==false)
            return;


        QSettings* seting=new QSettings(FileCfg,QSettings::IniFormat);
        seting->setIniCodec("UTF-8");
        QStringList lst=seting->allKeys();
        if(lst.contains("Name")==false &&
                lst.contains("Type")==false &&
                lst.contains("Icon"))
        {
            return;

        }
        app_packge_name=fil.dir().dirName();
        app_name=seting->value("Name").toString();
        app_kind=seting->value("Type").toString();
        app_icon=seting->value("Icon").toString();
        if(lst.contains("Default"))
        app_default=seting->value("Default").toString();


        setIcon(QIcon(fil.dir().path()+'/'+app_icon));
        this->setIconSize(QSize(32,32));
       // this->setFixedSize(32,32);

        QFile file(":/new/prefix1/res/qss/APPCheckBox.css");
        file.open(QFile::ReadOnly);
        QTextStream filetext(&file);
        QString stylesheet = QString("");
        stylesheet = filetext.readAll();
        file.close();
        this->setStyleSheet(stylesheet);
        if(app_default.compare("yes")==0)
            this->setChecked(true);
        init_ok=true;
    }



   void translater(QString langue){
         QSettings* seting=new QSettings(FileCfg,QSettings::IniFormat);
         seting->setIniCodec("UTF-8");
         QString key="Name_"+langue.section('_',0,0);
         QString name=seting->value(key).toString();
         if(name.isEmpty()==false)
         {
          setText(name);
         }
         else
             setText(app_name);
    }





    QString app_name;//安装名
    QString app_kind;//app种类
    QString app_icon;//图标名
    QString app_default="no";//默认
    QString app_packge_name;
    QString FileCfg;//配置文件
    bool init_ok;
};





class appcheckpage : public QWidget
{
    Q_OBJECT
public:
    explicit appcheckpage(QWidget *parent = nullptr, int nAPPNum = -1);
    qint8 AddAppCheckBox(QString FliePaht);//本页已满返回0，添加成功返回1，添加失败返回-1
    void transfer(QString lang);

    QVBoxLayout *mlayout;
    QList<AppCheckBox*> lst;
    QLabel* m_mainTitle;
    QLabel* m_subTitle;
    QWidget* LeftCells;
    QWidget* RightCells;
    QVBoxLayout* LeftLayout;
    QVBoxLayout* RightLayout;
    int m_nAPPNum;
    QWidget* m_centerCell;
    QVBoxLayout* m_centerLayout;
signals:

};
}
#endif // APPCHECKPAGE_H
