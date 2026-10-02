/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "myfileselect.h"
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include "filefilterproxymodelforbackup.h"
#include "../messageboxutils.h"

MyFileSelect::MyFileSelect(QWidget* parent) :
    QFileDialog(parent)
{
    this->setFileMode(QFileDialog::ExistingFiles);
    // 主题和文管重构了QFileDialog，下面代码已经失效
    /*
    this->setViewMode(QFileDialog::Detail);
    this->setFilter(QDir::System | QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    FileFilterProxyModeForBackup *proxy = new FileFilterProxyModeForBackup;
    this->setProxyModel(proxy);

    QDialogButtonBox* button = this->findChild<QDialogButtonBox*>("buttonBox");
    disconnect(button, SIGNAL(accepted()), this, SLOT(accept()));
    connect(button, SIGNAL(accepted()), this, SLOT(goAccept()));

    QLineEdit *fileNameEdit = this->findChild<QLineEdit*>("fileNameEdit");
    connect(this, &MyFileSelect::currentChanged, this, [=](const QString& path){
        fileNameEdit->setText(path.section('/', -1));
    });
    */
}

bool MyFileSelect::checkSelectedFiles()
{
    QStringList selectFiles = selectedFiles();
    for (const QString & fileName : selectFiles) {
        // 防命令注入
        // 1、形如：mkdir '`id&>id_bak_test.txt`'中的文件夹名称
        // 2、形如：$()的文件夹名称
        // 3、形如：${}的文件夹名称
        // 4、包含[;、&、|]等可以包含并执行系统命令或用于连续执行系统命令的符号
        if (    fileName.contains(QRegularExpression(".*`.*`.*"))
             || fileName.contains(QRegularExpression(".*\\$\\(.*\\).*"))
             || fileName.contains(QRegularExpression(".*\\$\\{.*\\}.*"))
             || fileName.contains(QRegularExpression("[;&|]+"))) {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(this, QObject::tr("Warning"), QObject::tr("Path can not include symbols that such as : ``,$(),${},;,&,|,etc."), QObject::tr("OK"));
            return false;
        }
    }

    return true;
}

void MyFileSelect::goAccept()
{
    if (checkSelectedFiles())
        accept();
}

MyFileSelect::~MyFileSelect()
{}

// 主题和文管重构了QFileDialog，需要使用下面代码绑定accept信号
void MyFileSelect::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)

    if (this->isVisible()) {
        for (QWidget* widget : this->findChildren<QWidget*>()) {
            if (widget->objectName() == "acceptButton") {
                QPushButton * btn = qobject_cast<QPushButton*>(widget);
                disconnect(btn, &QPushButton::clicked, 0, 0);
                connect(btn, &QPushButton::clicked, [=](bool checked){
                    Q_UNUSED(checked)
                    this->goAccept();
                });
            }
        }
    }
}

