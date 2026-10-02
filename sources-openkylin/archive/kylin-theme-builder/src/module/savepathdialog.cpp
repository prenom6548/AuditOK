/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "savepathdialog.h"


SavePathDialog::SavePathDialog(QWidget *parent) :
            kdk::KDialog(parent)
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setFixedSize(QSize(424,186));
    this->setFocus();

    QWidget*mainwindow = mainWidget();
    QGridLayout *layout = new QGridLayout(mainwindow);
    m_pathLabel = new QLabel;
    QString path = getSavePath();
    QString showpath = tr("Storage Path:")+path;
    QFont f(m_pathLabel->font());//获取控件字体font
    QFontMetrics fm(f);
    QString str1= fm.elidedText(showpath, Qt::ElideRight, 250);//三个参数分别为：需要省略的字符串，省略方式，长度

    m_pathLabel->setText(str1);
    m_pathLabel->setToolTip(path);
    m_modifyBtn = new QPushButton;
    m_modifyBtn->setFixedSize(QSize(88,33));
    m_modifyBtn->setText(tr("Modify"));
    m_confirmBtn = new QPushButton;
    m_confirmBtn->setFixedSize(QSize(88,33));
    m_confirmBtn->setText(tr("Confirm"));
    m_confirmBtn->setEnabled(false);

    layout->addWidget(m_pathLabel,0,0);
    layout->addWidget(m_modifyBtn,0,1);
    layout->addWidget(m_confirmBtn,1,1);
    layout->setContentsMargins(24, 0, 24, 0);

    connect(m_modifyBtn,&QPushButton::clicked,this,&SavePathDialog::onModifyBtnClicked);
    connect(m_confirmBtn,&QPushButton::clicked,this,[=](){
        emit savePathUpdate(m_savepath);
        this->close();
    });
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *gSetting = new QGSettings("org.ukui.style", QByteArray(), this);
        connect(gSetting, &QGSettings::changed, this, [=](const QString &key) {
            if ("systemFontSize" == key) {
                updateLabelShow(path);
            }
        });
    }
}

QString SavePathDialog::getSavePath()
{
    QString dirPath = QDir::homePath() + "/.cache/theme-build";
    QString filePath = dirPath + "/savepath.conf";

    QFile file(filePath);

    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        QString savePath = stream.readAll();
        file.close();

        return savePath;
    } else {
        qDebug() << "Error: File does not exist or cannot be opened.";
    }
}

void SavePathDialog::updateLabelShow(QString savepath)
{
    QString path = savepath;
    QString showpath = tr("Storage Path:")+path;
    QFont f(m_pathLabel->font());
    QFontMetrics fm(f);
    QString str1= fm.elidedText(showpath, Qt::ElideRight, 250);

    m_pathLabel->setText(str1);
    m_pathLabel->setToolTip(path);
}

void SavePathDialog::onModifyBtnClicked()
{
    QString newFilePath = QFileDialog::getExistingDirectory(this, tr("Select Directory"),QDir::homePath());
    if(newFilePath != nullptr){
        m_savepath = newFilePath;
        m_confirmBtn->setEnabled(true);
        updateLabelShow(newFilePath);
    }else{
        qDebug()<<"savepath is null";
    }
}
