#include "dependencyfixdialog.h"
/*该文件是在需要修复依赖时进行弹窗*/
dependencyfixdialog::dependencyfixdialog(int mode)
{
    updatemode = mode;
    initui();
    dbusconnect();
    setModal(true);
}

void dependencyfixdialog::initui()
{
    this->setWindowFlags(Qt::Dialog|Qt::CustomizeWindowHint);
    this->setMinimumSize(444,158);
//        this->setFixedSize(600,120);
        QIcon icon = QIcon::fromTheme("dialog-warning");
        QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(16, 16)));
        warning = new QLabel(this);
        warning->setPixmap(pixmap);

        attention = new FixLabel(this);
//        attention->adjustSize();
        attention->setFixedHeight(50);
        attention->setText("There exits dependency conficts in this updates, please remove some packages to finish the update.");

        details = new KBorderlessButton(this);
        details->setText(tr("show details"));
        QPalette palette_datailInfo = details->palette();
        palette_datailInfo.setColor(QPalette::ButtonText,QColor(55,144,250));
        details->setPalette(palette_datailInfo);

        QWidget *btnwidget = new QWidget();
        QHBoxLayout *btnlayout = new QHBoxLayout();
        btnwidget->setLayout(btnlayout);
        yesbtn = new QPushButton(this);
        yesbtn->setText(tr("uninstall and update"));
        nobtn = new QPushButton(this);
        nobtn->setText(tr("cancel"));

        btnlayout->addWidget(nobtn);
        btnlayout->addWidget(yesbtn,Qt::AlignRight);

        QHBoxLayout *allbtnlayout = new QHBoxLayout();
        allbtnlayout->addWidget(details);
        allbtnlayout->addStretch(10);
        allbtnlayout->addWidget(btnwidget,Qt::AlignRight);

        QHBoxLayout *labellayout = new QHBoxLayout();
        labellayout->addWidget(warning);
        labellayout->addWidget(attention);

        QVBoxLayout *alllayout = new QVBoxLayout();
        alllayout->addLayout(labellayout);
        alllayout->addStretch(40);
        alllayout->addLayout(allbtnlayout,0);

        alllayout->setContentsMargins(25,5,10,5);
        this->setLayout(alllayout);
}

void dependencyfixdialog::dbusconnect()
{
    connect(yesbtn,&QPushButton::clicked,this,&dependencyfixdialog::fixdependency);
    connect(nobtn,&QPushButton::clicked,this,&dependencyfixdialog::nofixdependency);
    connect(details,&KBorderlessButton::clicked,this,&dependencyfixdialog::showdetails);
}

void dependencyfixdialog::fixdependency()
{
    if(updatemode==updateAll)
    {
        updatemode=0;
        emit updatealldependsolveacceptsignal(true);
    }
    else if(updatemode==updatePart)
    {
        updatemode=0;
        versionlabelflag=1;
        emit updatedependsolveacceptsignal();
    }
    else if(updatemode==updateSystem)
    {
        updatemode=0;
        emit disupdatedependsolveacceptsignal();
    }
    this->close();
}

void dependencyfixdialog::nofixdependency()
{
//    emit updatedependsolvecancelsignal();
    this->close();
}

void dependencyfixdialog::showdetails()
{

    emit updatedependshowdetailssignal(pkgname,description,deletereason,updatemode);
    qDebug()<<"send the signal";
    this->close();
}

int dependencyfixdialog::updatedeletepkglist(QStringList pkgname,QStringList description,QStringList deletereason)
{
    qDebug()<<pkgname<<pkgname<<pkgname<<pkgname;
//    DeletePkgListWig *firstWidget = NULL;
    bool flag = true;
    int size=0;

    QList<QString>::Iterator it = pkgname.begin(),itend = pkgname.end();
    int i = 0;
    for (;it != itend; it++,i++)
    {
        qDebug() << "updatedeletepkglist函数：获取到的包列表：" << pkgname[i];
        size++;
    }
    return size;
}


void dependencyfixdialog::closeEvent(QCloseEvent *event)
{
    qDebug()<<"clsoe the dependencyfixdialog";
    emit closedpendencyfixdialog();
}
