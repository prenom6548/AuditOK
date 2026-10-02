#include "advancedoptions.h"
#include <QDebug>
#include<QFont>
#include<QDBusReply>
#include<QDir>
advancedoptions::advancedoptions(QWidget *parent): QWidget(parent)
{
    initui();
    initconnections();
    initapplist();
}
advancedoptions::~advancedoptions()
{
    ;
}
void advancedoptions::initui()
{
    //all widgets
    advancedoptiontitle = new QLabel(this);
    advancedoptiontitle->setText("高级设置");
    QFont advancedoptiontitlefont = QFont();
    advancedoptiontitlefont.setBold(true);
    advancedoptiontitle->setFont(advancedoptiontitlefont);

    KeepedThirdPartPackagesTitle = new QLabel(this);
    KeepedThirdPartPackagesTitle->setText("以下勾选的应用将不需要在新系统中兼容：");
    KeepedThirdPartPackages = new QListWidget(this);
    KeepedThirdPartPackages->setFocusPolicy(Qt::NoFocus);
    KeepedThirdPartPackages->setFixedSize(432,238);
    KeepedThirdPartPackages->setSelectionMode(QAbstractItemView::NoSelection);

    SelectAll = new KBorderlessButton(this);
    SelectAll->setText("全选");
    ReverseAll = new KBorderlessButton(this);
    ReverseAll->setText("反选");
    okbutton = new QPushButton(this);
    okbutton->setText("确定");
    okbutton->setFixedSize(120,48);
    okbutton->setFocusPolicy(Qt::NoFocus);
    cancelbutton = new QPushButton(this);
    cancelbutton->setText("取消");
    cancelbutton->setFixedSize(120,48);
    cancelbutton->setFocusPolicy(Qt::NoFocus);


    //layouts
    mainlayout = new QVBoxLayout(this);

    advancedoptionlayout = new QHBoxLayout();
    advancedoptionlayout->addSpacing(10);
    advancedoptionlayout->addWidget(advancedoptiontitle,Qt::AlignLeft);
    mainlayout->addSpacing(20);
    mainlayout->addLayout(advancedoptionlayout);


    SelectAllLayout = new QHBoxLayout();
    SelectAllLayout->addWidget(SelectAll);
    SelectAllLayout->setContentsMargins(14,0,0,0);
    ReverseAllLayout = new QHBoxLayout();
    ReverseAllLayout->addWidget(ReverseAll);
    SelectAllReverseAllLayout = new QHBoxLayout();
    SelectAllReverseAllLayout->addLayout(SelectAllLayout);
    SelectAllReverseAllLayout->addLayout(ReverseAllLayout);
    SelectAllReverseAllLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Minimum));

    KeepedThirdPartPackagestitlelayout = new QHBoxLayout();
    KeepedThirdPartPackagestitlelayout->addWidget(KeepedThirdPartPackagesTitle,Qt::AlignLeft);
    KeepedThirdPartPackagestitlelayout->setContentsMargins(14,0,0,0);
    KeepedThirdPartPackageslayout = new QHBoxLayout();
    KeepedThirdPartPackageslayout->addWidget(KeepedThirdPartPackages);

    thirdpartlayout = new QVBoxLayout();
    thirdpartlayout->addLayout(KeepedThirdPartPackagestitlelayout);
    thirdpartlayout->addSpacing(10);
    thirdpartlayout->addLayout(SelectAllReverseAllLayout);
    thirdpartlayout->addLayout(KeepedThirdPartPackageslayout);

    mainlayout->addSpacing(20);
    mainlayout->addLayout(thirdpartlayout);

    okcancelbuttonlayout = new QHBoxLayout();
    okcancelbuttonlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::MinimumExpanding ,QSizePolicy::Expanding));
    okcancelbuttonlayout->addWidget(okbutton,Qt::AlignLeft);
    okcancelbuttonlayout->addWidget(cancelbutton,Qt::AlignLeft);
    okcancelbuttonlayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::Expanding));
    mainlayout->addLayout(okcancelbuttonlayout);
    mainlayout->addSpacing(20);
    this->setLayout(mainlayout);

}

void advancedoptions::initconnections()
{

    connect(KeepedThirdPartPackages,&QListWidget::itemDoubleClicked,this,&advancedoptions::edititem);
    connect(okbutton,&QPushButton::clicked,this,&advancedoptions::ok);
    connect(cancelbutton,&QPushButton::clicked,this,&advancedoptions::cancel);
    connect(SelectAll,&QPushButton::clicked,this,&advancedoptions::selectall);
    connect(ReverseAll,&QPushButton::clicked,this,&advancedoptions::reverseall);
    dbus = dbusconnect::getInstance();
}

void advancedoptions::initapplist()
{
    QDBusReply<QStringList> ret = dbus->interface->call("getAppList");
    if (ret.isValid())
    {
        for(int i = 0;i<ret.value().length();++i)
        {
            QListWidgetItem * qlwitem = new QListWidgetItem(ret.value().at(i));
            qlwitem->setCheckState(Qt::CheckState::Unchecked);
            KeepedThirdPartPackages->addItem(qlwitem);
            if(i%2 == 0)
            {
                QPalette palette = QApplication::palette();
                QColor color = palette.color(QPalette::Window);
                QBrush brush(color);
                qlwitem->setBackground(brush);
            }
        }
    }
}


void advancedoptions::additem()
{
    qDebug()<<"add item";
}

void advancedoptions::removeitem()
{
    int row = KeepedThirdPartPackages->currentRow();
    qDebug()<<"remove item row"<<row;
    KeepedThirdPartPackages->takeItem(row);
}

void advancedoptions::edititem(QListWidgetItem *item)
{
}

void advancedoptions::ok()
{
    QStringList applist;
    selecteditemsname.clear();
    for(int i=0;i<KeepedThirdPartPackages->count();++i)
    {
        QListWidgetItem *item = KeepedThirdPartPackages->item(i);
        if(item->checkState())
        {
            applist.append(item->text());
            selecteditemsname.append(item->text());
        }
    }
    dbus->interface->call("setMoveAppList",applist);
    emit exitadvancedoptionsettings();
}

void advancedoptions::cancel()
{
    emit exitadvancedoptionsettings();
}

void advancedoptions::setupselecteditems(){
    for(int i=0;i<KeepedThirdPartPackages->count();++i)
    {
        QListWidgetItem *item = KeepedThirdPartPackages->item(i);
        if (selecteditemsname.contains(item->text()))
        {
            item->setCheckState(Qt::CheckState::Checked);
        }
        else
        {
            item->setCheckState(Qt::CheckState::Unchecked);
        }
    }
}

void advancedoptions::selectall()
{
     for(int i=0;i<KeepedThirdPartPackages->count();++i)
      {
           QListWidgetItem *item = KeepedThirdPartPackages->item(i);
           item->setCheckState(Qt::CheckState::Checked);
      }
}

void advancedoptions::reverseall()
{
    for(int i=0;i<KeepedThirdPartPackages->count();++i)
    {
        QListWidgetItem *item = KeepedThirdPartPackages->item(i);
        if(item->checkState() == Qt::CheckState::Checked)
        {
             item->setCheckState(Qt::CheckState::Unchecked);
        }
        else
        {
            item->setCheckState(Qt::CheckState::Checked);
        }
    }
}
