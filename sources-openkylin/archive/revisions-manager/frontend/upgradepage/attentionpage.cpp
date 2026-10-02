#include "attentionpage.h"

attentionpage::attentionpage(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void attentionpage::initui()
{
    main = new QVBoxLayout();
    titlelayout = new QHBoxLayout();
    title = new QLabel();
    content = new QTextEdit();
    btn = new QPushButton();

    title->setText("注意事项");
    btn->setText("返回");
    btn->setFixedSize(120,48);

    /*load license*/
    QString path = ":/res/Notify";
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        qDebug() << "license Can not open";
    QTextStream in(&file);
    QString m_fileStr = in.readAll();
    content->setText(m_fileStr);
    file.close();
    content->setFixedSize(400,380);
    content->setReadOnly(true);
    content->viewport()->setAutoFillBackground(true);
    content->viewport()->setBackgroundRole(QPalette::Window);

    titlelayout->addStretch();
    titlelayout->addWidget(title);
    titlelayout->addStretch();

    main->addLayout(titlelayout);
    main->addSpacing(10);
    main->addWidget(content,0,Qt::AlignHCenter);
    main->addStretch();
    main->addSpacing(10);
    main->addWidget(btn,0,Qt::AlignHCenter);
    main->addSpacing(40);

    btn->setFocusPolicy(Qt::NoFocus);
    this->setLayout(main);
}

void attentionpage::dbusconnection()
{
    qDebug()<<"提示界面生成";
    connect(btn,SIGNAL(clicked(bool)),this,SLOT(returntofirst(bool)));
}

void attentionpage::returntofirst(bool)
{
    emit returnsignal();
    this->close();
}
