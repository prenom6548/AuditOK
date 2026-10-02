/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "advancedwidget.h"
#include "messagebox.h"
#include "customwidget.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QDBusInterface>
#include <QDBusReply>
#include <QPushButton>
#include <QDebug>
#include <QFile>
#include <QSlider>
#include <QFrame>
#include <unistd.h>

#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 42

class HLine : public QFrame
{
public:
    HLine(QWidget *parent) 
        : QFrame(parent) {
        setFrameShape(QFrame::HLine);
        setFixedHeight(1);
        setStyleSheet("QFrame{background: rgb(220,220,220); border: 0px;}");
    }
};

AdvancedWidget::AdvancedWidget(QWidget * parent)
    : SettingScrollContent(parent)
    , mClearUserData(false)
    , mUninstallProcess(nullptr)
    , mUninstallProgress(nullptr)
    , mPref(new Preferences)
{
    m_title->setText(tr("Advanced Setting"));

    // ---------------- 虚拟定位
    auto *gpsBtn = new CustomWidget<QPushButton>(tr("Virt Position"), 130, BUTTON_WIDTH, BUTTON_HEIGHT);
    connect(gpsBtn, &QPushButton::clicked, this, [] {
        if (QFile::exists("/usr/bin/kylin-kmre-gps")) {
            QProcess::startDetached("/usr/bin/kylin-kmre-gps");
        }
    });

    // ----------------- 滚轮灵敏度
    auto *scrollLbl = new CustomWidget<QLabel>(tr("Scroll Sensitivity"), BUTTON_WIDTH, BUTTON_WIDTH, 0);

    QLabel *slowLbl = new QLabel(tr("Slow"));
    QLabel *fastLbl = new QLabel(tr("Fast"));

    QSlider *sensitivitySld = new QSlider;
    sensitivitySld->setOrientation(Qt::Horizontal);
    sensitivitySld->setValue(mPref->mScrollSensitivity);
    sensitivitySld->setMaximum(mPref->mMaxScrollSensitivity);
    sensitivitySld->setMinimum(mPref->mMinScrollSensitivity);
    connect(sensitivitySld, &QSlider::sliderReleased, this, [=] {
        mPref->updateScrollSensitivity(sensitivitySld->value());
    });

    auto *scrollBtn = new CustomWidget<QPushButton>(tr("Default"), 100, BUTTON_WIDTH - 40, BUTTON_HEIGHT);
    connect(scrollBtn, &QPushButton::clicked, this, [=] {
        sensitivitySld->setValue(DEFAULT_SCROLL_SENSITIVITY);
        mPref->updateScrollSensitivity(DEFAULT_SCROLL_SENSITIVITY);
    });

    QHBoxLayout *scrollLayout = new  QHBoxLayout;
    scrollLayout->addWidget(scrollLbl, 0, Qt::AlignLeft);
    scrollLayout->addSpacing(20);
    scrollLayout->addWidget(slowLbl, 0, Qt::AlignLeft);
    scrollLayout->addWidget(sensitivitySld, 0, Qt::AlignLeft);
    scrollLayout->addWidget(fastLbl, 0, Qt::AlignLeft);
    scrollLayout->addWidget(scrollBtn, 0, Qt::AlignRight);
    scrollLayout->addSpacing(10);

    // ----------------- 关闭KMRE环境
    const QString origString2 = tr("Close Kmre");
    auto [elided2, elideText2] = kmre::utils::getElideText(origString2, 130);
    QPushButton *closeKmreBtn = new QPushButton(elideText2);
    if (elided2) {
        closeKmreBtn->setToolTip(origString2);
    }
    closeKmreBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect(closeKmreBtn, SIGNAL(clicked()), this, SLOT(slotCloseKmre()));

    // ----------------- 卸载KMRE环境
    const QString origString3 = tr("Uninstall Kmre");
    auto [elided3, elideText3] = kmre::utils::getElideText(origString3, 130);    
    QPushButton *uninstallKmreBtn = new QPushButton(elideText3);
    if (elided3) {
        uninstallKmreBtn->setToolTip(origString3);
    }
    uninstallKmreBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect(uninstallKmreBtn, SIGNAL(clicked()), this, SLOT(slotUninstallKmre()));

    QCheckBox *clearUserDataCkb = new QCheckBox(tr("Clear User Data"));
    clearUserDataCkb->setChecked(false);
    clearUserDataCkb->setFixedHeight(BUTTON_HEIGHT);
    connect(clearUserDataCkb, &QCheckBox::clicked, this, [&] (bool checked) {
        mClearUserData = checked;
    });

    QHBoxLayout *uninstallLayout = new  QHBoxLayout;
    uninstallLayout->addWidget(uninstallKmreBtn, 0, Qt::AlignLeft);
    uninstallLayout->addSpacing(20);
    uninstallLayout->addWidget(clearUserDataCkb, 0, Qt::AlignLeft);
    uninstallLayout->addStretch();

    // --------------------------------------------------------------------
    QVBoxLayout *layout = new  QVBoxLayout;
    //layout->setContentsMargins(2, 20, 2, 20);
    //layout->addSpacing(20);
    layout->addWidget(new HLine(this));
    layout->addSpacing(10);
    layout->addWidget(gpsBtn);
    layout->addSpacing(10);
    layout->addWidget(new HLine(this));
    layout->addSpacing(10);
    layout->addLayout(scrollLayout);
    layout->addSpacing(10);
    layout->addWidget(new HLine(this));
    layout->addSpacing(10);
    layout->addWidget(closeKmreBtn);
    layout->addSpacing(10);
    layout->addWidget(new HLine(this));
    layout->addSpacing(10);
    layout->addLayout(uninstallLayout);
    layout->addSpacing(10);

    QFrame *centralWidget = new QFrame;
    centralWidget->setLayout(layout);
    
    setContent(centralWidget);
}

void AdvancedWidget::init()
{
    
}

void AdvancedWidget::slotCloseKmre()
{
    if (KylinUI::MessageBox::question(this, tr(""), tr("Are you sure to close kmre environment?"))) {
        QDBusInterface kmreDbus("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
        kmreDbus.call("StopContainer", kmre::utils::getUserName(), (int32_t)getuid());

        exit(0);
    }
}

void AdvancedWidget::slotUninstallKmre()
{
    if (KylinUI::MessageBox::question(this, tr(""), tr("Are you sure to uninstall kmre environment?"))) {
        if (!mUninstallProcess) {
            mUninstallProcess = new QProcess();
            connect(mUninstallProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadProgress()));
            connect(mUninstallProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotFinished(int, QProcess::ExitStatus)));
        }

        if (mUninstallProcess->state() == QProcess::NotRunning) {
            QStringList command;
            command << "/bin/bash" << "/usr/share/kmre/rm_kmre.sh";
            command << kmre::utils::getUserName();
            command << QString::number((int32_t)getuid());
            if (mClearUserData) {
                command << "clear";
            }

            mUninstallProcess->start("/bin/pkexec", command);

            if (!mUninstallProgress) {
                mUninstallProgress = new QProgressDialog(tr("Uninstalling Kmre"), "", 0, 100, this);
                mUninstallProgress->setWindowTitle(tr("Uninstall Kmre"));
                mUninstallProgress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                mUninstallProgress->setWindowModality(Qt::WindowModal);
                mUninstallProgress->setCancelButton(NULL);
                mUninstallProgress->setMinimumDuration(500);
                mUninstallProgress->setFixedWidth(300);
                mUninstallProgress->setValue(0);
            }
        }
    }
}

void AdvancedWidget::slotReadProgress()
{
    QString output = mUninstallProcess->readAllStandardOutput();
    QStringList infoList = output.split("\n", QString::SkipEmptyParts);

    for (auto &info : infoList) {
        qDebug() << info;

        if (info.startsWith("[Progress]")) {
            bool ok;
            int value = info.remove("[Progress]").toInt(&ok);
            if (mUninstallProgress && ok) {
                mUninstallProgress->setValue(value);
            }
        }
        else if (info.startsWith("[Message]")) {
            QString msg = info.remove("[Message]");
            if (mUninstallProgress && (!msg.isEmpty())) {
                mUninstallProgress->setLabelText(msg);
            }
        }
    }
}

void AdvancedWidget::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Uninstall finished, exitCode: " << exitCode << ", exitStatus: " << exitStatus;

    if (exitCode == 0) {// uninstall successfully
        exit(0);
    }
    else {
        if ((exitCode == 127) && (exitStatus == QProcess::NormalExit)) {
            // process canceled, do nothing
        }
        else {
            KylinUI::MessageBox::warning(this, tr("Error"), tr("Error occured! Uninstall kmre failed! "));
        }

        if (mUninstallProgress) {
            mUninstallProgress->close();
            delete mUninstallProgress;
            mUninstallProgress = nullptr;
        }
    }
}
