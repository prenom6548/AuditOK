#include "importactivationfilespage.h"
#include <QStyle>
#include <QApplication>
#include <QFont>
#include <QFileDialog>
#include<QDebug>
#include<QFile>
#include<QMessageBox>

importactivationfiles::importactivationfiles(QWidget *parent): QWidget(parent)
{
    initui();
    initconnections();
}

importactivationfiles::~importactivationfiles()
{
    ;
}

void importactivationfiles::initui()
{

    systemactivationtitle = new QLabel(this);
    systemactivationtitle->setText("系统激活");
    QFont systemactivationtitlefont;
    systemactivationtitlefont.setBold(true);
    systemactivationtitle->setFont(systemactivationtitlefont);
//    systemactivationtitle->setStyleSheet("QLabel{background: #FFFFFF}");
    activationtiptitle = new QLabel(this);
    activationtiptitle->setText("本次升级不会保存当前版本的激活状态，请导入正确的授权文件，并在升级完成后通过“扫码激活”方式完成系统激活。");
    activationtiptitle->setWordWrap(true);
    activationtiptitle->setFixedWidth(436);
//    activationtiptitle->setStyleSheet("QLabel{background: #FFFFFF}");
    importactivationfilesbutton = new QPushButton(this);
    importactivationfilesbutton->setText("导入授权文件");
    importactivationfilesbutton->setFixedSize(432,48);
    importactivationfilesbutton->setFocusPolicy(Qt::NoFocus);
//    QStyle *style = QApplication::style();
    kyinfolabel = new QLabel(this);
    kyinfolabel->setText(".kyinfo");
//    kyinfolabel->setFixedSize(70,20);
//    kyinfolabel->setFixedWidth(50);
//    kyinfolabel->setStyleSheet("QLabel{background: #FFFFFF}");
    kyinfolabel->hide();
    kyinfodeletebutton = new QPushButton(this);
    kyinfodeletebutton->setFlat(true);
//    kyinfodeletebutton->setFixedSize(20,20);

    kyinfodeletebutton->setFocusPolicy(Qt::NoFocus);
//    kyinfodeletebutton->setStyleSheet("QPushButton{background: #FFFFFF}");
    auto kyinfodeleteicon = this->style()->standardIcon(QStyle::SP_TrashIcon);
    kyinfodeletebutton->setIcon(kyinfodeleteicon);
    kyinfodeletebutton->hide();
    licenselabel = new QLabel(this);
    licenselabel->setText("LICENSE");
//    licenselabel->setFixedWidth(50);
//    licenselabel->setStyleSheet("QLabel{background: #FFFFFF}");
    licenselabel->hide();
    licensedeletebutton = new QPushButton(this);
    licensedeletebutton->setFlat(true);
//    licensedeletebutton->setFixedSize(20,20);

    licensedeletebutton->setFocusPolicy(Qt::NoFocus);
//    licensedeletebutton->setStyleSheet("QPushButton{background: #FFFFFF}");
    auto licensedeleteicon = this->style()->standardIcon(QStyle::SP_TrashIcon);
    licensedeletebutton->setIcon(licensedeleteicon);
    licensedeletebutton->hide();
    upgradebutton = new QPushButton(this);
    upgradebutton->setText("升级");
    upgradebutton->setEnabled(false);
    upgradebutton->setFixedSize(432,48);
    upgradebutton->setFocusPolicy(Qt::NoFocus);
//    upgradebutton->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626;font-size:18px}");
    laststepbutton = new QPushButton(this);
    laststepbutton->setText("上一步");
    laststepbutton->setFixedSize(432,48);
    laststepbutton->setFocusPolicy(Qt::NoFocus);

    systemactivationtitlelayout = new QHBoxLayout();
    systemactivationtitlelayout->addWidget(systemactivationtitle,0,Qt::AlignCenter);
    activationtiplayout = new QHBoxLayout();
    activationtiplayout->addWidget(activationtiptitle);
    importactivationfilesbuttonlayout = new QHBoxLayout();
    importactivationfilesbuttonlayout->addWidget(importactivationfilesbutton);
    kyinfolayout = new QHBoxLayout();
    kyinfolayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
    kyinfolayout->addWidget(kyinfolabel);
    kyinfolayout->addWidget(kyinfodeletebutton);
    kyinfolayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
    licenselayout = new QHBoxLayout();
    licenselayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
    licenselayout->addWidget(licenselabel);
    licenselayout->addWidget(licensedeletebutton);
    licenselayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));
    upgradebuttonlayout = new QHBoxLayout();
    upgradebuttonlayout->addWidget(upgradebutton);
    laststepbuttonlayout = new QHBoxLayout();
    laststepbuttonlayout->addWidget(laststepbutton);
    mainlayout = new QVBoxLayout();
    mainlayout->addSpacing(20);
    mainlayout->addLayout(systemactivationtitlelayout);
    mainlayout->addSpacing(20);
    mainlayout->addLayout(activationtiplayout);
    mainlayout->addSpacing(20);
    mainlayout->addLayout(importactivationfilesbuttonlayout);
    mainlayout->addSpacing(20);
    mainlayout->addLayout(kyinfolayout);
    mainlayout->addSpacing(10);
    mainlayout->addLayout(licenselayout);
//    mainlayout->addWidget(spacefiller1);
    mainlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    mainlayout->addLayout(upgradebuttonlayout);
    mainlayout->addLayout(laststepbuttonlayout);
    this->setLayout(mainlayout);
}

void importactivationfiles::initconnections()
{
    connect(importactivationfilesbutton,&QPushButton::clicked,this,&importactivationfiles::chooseactivationfiles);
    connect(kyinfodeletebutton,&QPushButton::clicked,this,&importactivationfiles::deletekyinfofile);
    connect(licensedeletebutton,&QPushButton::clicked,this,&importactivationfiles::deletelicensefile);
    connect(upgradebutton,&QPushButton::clicked,this,&importactivationfiles::upgrade);
    connect(laststepbutton,&QPushButton::clicked,this,&importactivationfiles::laststep);
    dbus = dbusconnect::getInstance();
}

void importactivationfiles::chooseactivationfiles()
{
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setWindowTitle("选择激活文件");
    dialog->setDirectory(QDir::homePath());
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setNameFilter("*kyinfo* *LICENSE*");
    dialog->setOption(QFileDialog::DontUseNativeDialog,false);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    dialog->setViewMode(QFileDialog::Detail);
    if (dialog->exec())
    {
        QStringList activationfiles = dialog->selectedFiles();
        qDebug()<<activationfiles;
        for(int i=0;i<activationfiles.size();++i)
        {
            bool licensefilecheckresult = checklicensefileformat(activationfiles.at(i));
            if (licensefilecheckresult)
            {
                ;
            }
            else
            {
                QMessageBox box(QMessageBox::Information, NULL, QString("非法的文件名:")+activationfiles.at(i),QMessageBox::NoButton,this);
                QPushButton *closeButton = box.addButton("关闭", QMessageBox::AcceptRole);
                box.setDefaultButton(closeButton);
                box.exec();
                break;
            }
            if(QFile::exists(activationfiles.at(i)))
            {
                QFileInfo fileinfo = QFileInfo(activationfiles.at(i));
                QString path = activationfiles.at(i);
                if(fileinfo.fileName() == ".kyinfo")
                {
                    kyinfo = path;
                    kyinfodeletebutton->show();
                    kyinfolabel->setToolTip(path);
                    kyinfolabel->show();
                }
                else if (fileinfo.fileName() == "LICENSE")
                {
                    license = path;
                    licensedeletebutton->show();
                    licenselabel->setToolTip(path);
                    licenselabel->show();
                }
            }
        }
    }
    else
    {
        qDebug()<<"file dialog exec failed";
    }
    if(kyinfolabel->isVisible()  && licenselabel->isVisible())
    {
        upgradebutton->setEnabled(true);
//        upgradebutton->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF;font-size:18px}");
    }
}

void importactivationfiles::upgrade()
{
    if(kyinfolabel->isVisible() && licenselabel->isVisible())
    {
        dbus->interface->call("setActivationFiles",kyinfo,license);
    }
    emit nextpage();
}

void importactivationfiles::laststep()
{
    emit lastpage();
}

void importactivationfiles::deletekyinfofile()
{
    if(kyinfodeletebutton->isVisible())
    {
        kyinfodeletebutton->hide();
    }
    if(kyinfolabel->isVisible())
    {
        kyinfolabel->hide();
    }
    upgradebutton->setEnabled(false);
//    upgradebutton->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626;font-size:18px}");
}

void importactivationfiles::deletelicensefile()
{
    if(licensedeletebutton->isVisible())
    {
        licensedeletebutton->hide();
    }
    if(licenselabel->isVisible())
    {
        licenselabel->hide();
    }
    upgradebutton->setEnabled(false);
//    upgradebutton->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626;font-size:18px}");
}

bool importactivationfiles::checklicensefileformat(QString path)
{
    QFileInfo fileInfo(path);
    if(fileInfo.isFile() && fileInfo.exists())
    {
        if (fileInfo.fileName().contains("kyinfo")||fileInfo.fileName().contains("LICENSE"))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}



