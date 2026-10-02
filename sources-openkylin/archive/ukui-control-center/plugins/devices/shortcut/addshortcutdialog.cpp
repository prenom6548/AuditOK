/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "addshortcutdialog.h"
#include "realizeshortcutwheel.h"
#include <QApplication>
#include <QShowEvent>
#include <QTimer>
#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <QFileSystemWatcher>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#define DEFAULTPATH "/usr/share/applications/"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

AddShortcutDialog::AddShortcutDialog(QList<KeyEntry> generalEntries,
                                     QList<KeyEntry> windowEntries,
                                     QList<KeyEntry> customEntries, QWidget *parent) :
    KDialog(parent),
    gsPath(""),
    m_systemEntry(generalEntries),
    m_windowEntry(windowEntries),
    m_customEntry(customEntries),
    keyIsAvailable(false) {

    initUi();
    m_oriShortcutLineText.clear();
    editSeq  = QKeySequence("");
    editName = "";
    keyIsAvailable  = 0;
    execIsAvailable = false;
    nameIsAvailable = false;
    execIcon = new QLabel(m_execLineEdit);
    execIcon->move(execIcon->x() + 8, execIcon->y());
    execIcon->setFixedSize(24, 36);
    m_execLineEdit->setTextMargins(32, m_execLineEdit->textMargins().top(),\
                                        m_execLineEdit->textMargins().right(),\
                                        m_execLineEdit->textMargins().bottom());
    initSetup();
    slotsSetup();
    limitInput();

}

AddShortcutDialog::~AddShortcutDialog()
{
    m_oriShortcutLineText.clear();
}

void AddShortcutDialog::initUi() {
    setWindowIcon("ukui-control-center");
    setFixedSize(480, 310);

    m_execLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_execLabel, "Shortcut", "addDialogExecLabel", "exec label of addShortcutDialog");
    m_execLabel->setText(tr("Exec"));
    m_execLabel->setFixedWidth(48);

    m_execLineEdit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_execLineEdit, "Shortcut", "addDialogLineEdit", "exec lineedit of addShortcutDialog");

    m_openBtn = new kdk::KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_openBtn, "Shortcut", "addDialogOpenButton", "open button of addShortcutDialog");
    m_openBtn->setText(tr("Open"));
    m_openBtn->setFixedWidth(84);
    // 程序路径只读，Tab/初始焦点落到「选择」
    m_openBtn->setFocusPolicy(Qt::StrongFocus);

    m_execHorizontalLayout = new QHBoxLayout();
    m_execHorizontalLayout->addWidget(m_execLabel);
    m_execHorizontalLayout->addWidget(m_execLineEdit);
    m_execHorizontalLayout->addSpacing(8);
    m_execHorizontalLayout->addWidget(m_openBtn);

    m_nameLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_nameLabel, "Shortcut", "addDialogNameLabel", "name label of addShortcutDialog");
    m_nameLabel->setText(tr("Name"));
    m_nameLabel->setFixedWidth(48);

    m_nameLineEdit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_nameLineEdit, "Shortcut", "addDialogNameLineEdit", "name lineedit of addShortcutDialog");
    m_nameLineEdit->setFocusPolicy(Qt::StrongFocus);

    m_nameHorizontalLayout = new QHBoxLayout();
    m_nameHorizontalLayout->setContentsMargins(0,0,0,0);
    m_nameHorizontalLayout->addWidget(m_nameLabel);
    m_nameHorizontalLayout->addWidget(m_nameLineEdit);

    m_infoLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_infoLabel, "Shortcut", "addDialogTipLabel", "tip label of addShortcutDialog");
    m_infoLabel->setText(tr("TextLabel"));
    QFont ft;
    ft.setPixelSize(12);
    m_infoLabel->setFont(ft);
    m_infoLabel->setFontColor(Qt::red);

    m_infoHorizontalLayout = new QHBoxLayout();
    m_infoHorizontalLayout->setSpacing(0);
    m_infoHorizontalLayout->setContentsMargins(56, 0, 0, 0);

    m_infoHorizontalLayout->addWidget(m_infoLabel);

    m_keyLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_keyLabel, "Shortcut", "addDialogCancelButton", "tip label1 of addShortcutDialog");
    m_keyLabel->setText(tr("Key"));
    m_keyLabel->setFixedWidth(48);

    m_keyHorizontalLayout = new QHBoxLayout();
    m_keyHorizontalLayout->setContentsMargins(0,0,0,0);
    m_keyHorizontalLayout->addWidget(m_keyLabel);

    m_info2Label = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_info2Label, "Shortcut", "addDialogCancelButton", "tip label2 of addShortcutDialog");
    m_info2Label->setText(tr("TextLabel"));
    m_info2Label->setFont(ft);
    m_info2Label->setFontColor(Qt::red);

    m_info2HorizontalLayout = new QHBoxLayout();
    m_info2HorizontalLayout->setSpacing(0);
    m_info2HorizontalLayout->setContentsMargins(56, 0, 0, 0);
    m_info2HorizontalLayout->addWidget(m_info2Label);

    m_cancelBtn = new kdk::KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_cancelBtn, "Shortcut", "addDialogCancelButton", "cancel button of addShortcutDialog");
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFixedWidth(96);
    m_cancelBtn->setFocusPolicy(Qt::StrongFocus);
    m_cancelBtn->setAutoDefault(false);

    m_certainBtn = new kdk::KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_certainBtn, "Shortcut", "addDialogCertainButton", "certain button of addShortcutDialog");
    m_certainBtn->setText(tr("Save"));
    m_certainBtn->setFixedWidth(96);
    m_certainBtn->setCheckable(true);
    m_certainBtn->setAutoDefault(false);
    m_certainBtn->setFocusPolicy(Qt::StrongFocus);

    m_openBtn->setAutoDefault(false);

    m_bottomHorizontalLayout = new QHBoxLayout();
    m_bottomHorizontalLayout->addStretch();
    m_bottomHorizontalLayout->addWidget(m_cancelBtn);
    m_bottomHorizontalLayout->addSpacing(8);
    m_bottomHorizontalLayout->addWidget(m_certainBtn);


    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(24, 8, 24, 24);
    mainLayout->setSpacing(0);

    mainLayout->addLayout(m_execHorizontalLayout);
    mainLayout->addSpacing(24);
    mainLayout->addLayout(m_nameHorizontalLayout);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(m_infoHorizontalLayout);
    mainLayout->addLayout(m_keyHorizontalLayout);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(m_info2HorizontalLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(m_bottomHorizontalLayout);
    mainWidget()->setLayout(mainLayout);

    m_execLabel->setAccessibleName(tr("Exec"));
    m_execLabel->setBuddy(m_openBtn);
    m_execLineEdit->setAccessibleName(tr("Exec"));
    m_openBtn->setAccessibleName(tr("Open"));
    m_nameLabel->setAccessibleName(tr("Name"));
    m_nameLabel->setBuddy(m_nameLineEdit);
    m_nameLineEdit->setAccessibleName(tr("Name"));
    m_cancelBtn->setAccessibleName(tr("Cancel"));
    m_certainBtn->setAccessibleName(tr("Save"));
}

void AddShortcutDialog::initSetup()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Add Shortcut"));

    m_execLineEdit->setReadOnly(true);
    m_execLineEdit->setFocusPolicy(Qt::NoFocus);
    m_execLineEdit->setEnabled(false);
    m_info2Label->setText("");
    m_infoLabel->setText("");
    m_certainBtn->setDisabled(true);
    m_certainBtn->setBackgroundColorHighlight(true);
    m_shortcutLine = new ShortcutLine(m_systemEntry, m_windowEntry, m_customEntry);
    m_shortcutLine->setAddDialogEdit(true);
    m_shortcutLine->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_keyHorizontalLayout->addWidget(m_shortcutLine);
    m_shortcutLine->setMinimumWidth(280);
    m_shortcutLine->setPlaceholderText(tr("Please enter a shortcut"));
    m_keyLabel->setAccessibleName(tr("Key"));
    m_keyLabel->setBuddy(m_shortcutLine);
    m_shortcutLine->setAccessibleName(tr("Key"));

    m_shortcutLine->installEventFilter(this);

    m_nameLineEdit->installEventFilter(this);
    static const int MAX_LENGTH = 256;
    m_nameLineEdit->setMaxLength(MAX_LENGTH);
    connect(m_shortcutLine, &ShortcutLine::shortCutAvailable, this, [=](const int &flag) {
        shortCutAvailableSlot(flag);
    });

    if (m_shortcutLine->sizeHint().height() > 30 && m_shortcutLine->sizeHint().height() < 40) {
        this->setFixedHeight(310);
    } else if (m_shortcutLine->sizeHint().height() >= 40 && m_shortcutLine->sizeHint().height() < 50) {
        this->setFixedHeight(358);
    }

    fixAddDialogTabOrder();
}

void AddShortcutDialog::shortCutAvailableSlot(const int &flag) {
    if (flag == 0 || (flag == -2 && editSeq == m_shortcutLine->keySequence())) {  //快捷键正常
        keyIsAvailable = 3;
    } else if(flag == -2) { //快捷键冲突
        keyIsAvailable = 1;
    } else {               //快捷键不可用
        keyIsAvailable = 2;
    }
    qDebug() << Q_FUNC_INFO << "flag" << flag << "certainBtn isEnabled" << m_certainBtn->isEnabled();
    if (flag == 4 && m_certainBtn->isEnabled()) {
        m_certainBtn->clicked(true);
        m_certainBtn->setProperty("isImportant", true);
    } else {
        refreshCertainChecked(3);
    }
}

void AddShortcutDialog::slotsSetup()
{
    connect(m_openBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        openProgramFileDialog();
    });

    connect(m_execLineEdit, &QLineEdit::textChanged, [=](QString text){
        if (mExec.endsWith("desktop")
            || (!g_file_test(mExec.toLatin1().data(),
                             G_FILE_TEST_IS_DIR)
                && g_file_test(mExec.toLatin1().data(), G_FILE_TEST_IS_EXECUTABLE))) {
            execIsAvailable = true;
        } else {
            execIsAvailable = false;
        }

        refreshCertainChecked(1);
    });

    connect(m_nameLineEdit, &QLineEdit::textChanged, [=](){
        QStringList customName;
        QString text = m_nameLineEdit->text();
        if (text.isEmpty()) {
            nameIsAvailable = false;
        } else {
            if (m_customEntry.isEmpty()) {
                nameIsAvailable = true;
            } else {
                    for (KeyEntry& ckeyEntry : m_customEntry) {
                        customName << ckeyEntry.nameStr;
                        if (customName.contains(text) && text != editName) {
                            nameIsAvailable = false;
                        } else {
                            nameIsAvailable = true;
                        }
                    }
            }
        }
        refreshCertainChecked(2);
    });

    connect(m_cancelBtn, &QPushButton::clicked, [=] {
        close();
    });
    connect(m_certainBtn, &QPushButton::clicked, [=] {
        qDebug() << Q_FUNC_INFO << gsPath << m_nameLineEdit->text() << selectedfile << m_shortcutLine->keySequence().toString();
        emit shortcutInfoSignal(gsPath, m_nameLineEdit->text(), selectedfile,
                                m_shortcutLine->keySequence().toString());

        close();
    });

    connect(this, &AddShortcutDialog::finished, [=] {
        gsPath = "";
        m_nameLineEdit->clear();
        m_execLineEdit->clear();
        // 勿在此 setFocus 到名称框：下次打开会被抢成「名称」为首焦
    });
}

void AddShortcutDialog::setTitleText(QString text)
{

}

void AddShortcutDialog::setUpdateEnv(QString path, QString name, QString exec)
{
    gsPath = path;
    m_nameLineEdit->setText(name);
    m_execLineEdit->setText(exec);
}

void AddShortcutDialog::limitInput()
{
    // 大小写字母数字中文
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5]+");
    QRegExpValidator *regValidator = new QRegExpValidator(rx);
    // 输入限制
    // m_nameLineEdit->setValidator(regValidator);
    // 字符长度限制
    // m_nameLineEdit->setMaxLength(10);
}

QString AddShortcutDialog::keyToSequence(QString key) {
    QString oldKey = key;
    key.replace(" ", "");
    if (key.contains("Win")) {
        key.replace("Win", "Meta");
    }

    if (key.contains("Super")) {
        key.replace("Super", "Meta");
    }

    if (key.contains("Start")) {
        key.replace("Start", "Meta");
    }
    if (key.contains("<")) {
        key.replace("<", "");
    }

    if (key.contains(">")) {
        key.replace(">", "+");
    }
    qDebug() << Q_FUNC_INFO << "update" << oldKey << "to" << key;
    return key;
}

QString AddShortcutDialog::keyToLib(QString key)
{
    if (key.contains("+")) {
        QStringList keys = key.split("+");
        if (keys.count() == 2) {
            QString lower = keys.at(1);
            QString keyToLib = "<" + keys.at(0) + ">" + lower.toLower();

            return keyToLib;
        } else if (keys.count() == 3) {
            QString lower = keys.at(2);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + lower.toLower();

            return keyToLib;
        } else if (keys.count() == 4) {
            QString lower = keys.at(2);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" +  "<" + keys.at(2) + ">" + lower.toLower();

            return keyToLib;
        }
    }

    return key;
}

void AddShortcutDialog::openProgramFileDialog()
{
    QString filters = tr("Desktop files(*)");
    QFileDialog fd(this);

    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8;// 最大添加U盘数，可以自己定义
    QString home_path = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home_path + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList file_list = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
        QFileInfo fi = file_list.at(i);
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher m_fileSystemWatcher(&fd);
    m_fileSystemWatcher.addPath("/media/" + home_path + "/");
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
        QDir m_wmntDir(path);
        m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
            QFileInfo m_fi = m_wfilist.at(i);
            mntUrlList << QUrl("file://" + m_fi.filePath());
        }
        fd.setSidebarUrls(usb_list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
        fd.setSidebarUrls(usb_list);
    });

    fd.setDirectory(DEFAULTPATH);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select desktop"));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));
    fd.setSidebarUrls(usb_list + mntUrlList);

    if (fd.exec() != QDialog::Accepted)
        return;

    selectedfile = fd.selectedFiles().first();

    QString exec = selectedfile.section("/", -1, -1);

    GDesktopAppInfo * textinfo = g_desktop_app_info_new_from_filename(selectedfile.toUtf8().constData());
    QString appname = g_app_info_get_name(G_APP_INFO(textinfo));
    const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(textinfo)));
    setIcon(QString(QLatin1String(iconname)));

    mExec = exec;
    m_execLineEdit->setText(appname);
    m_nameLineEdit->setText(appname);
}

void AddShortcutDialog::refreshCertainChecked(int triggerFlag)
{
    if (!m_nameLineEdit->text().isEmpty()) {
        m_infoLabel->setText("");
    }

    if (!m_shortcutLine->text().isEmpty()) {
        m_info2Label->setText("");
    }

    if (!execIsAvailable || keyIsAvailable != 3 || !nameIsAvailable) {
        m_certainBtn->setDisabled(true);
        m_certainBtn->setProperty("isImportant", true);
        switch (triggerFlag) {
        case 1:
            if (!execIsAvailable) {
                m_info2Label->setText(tr("Invalid application"));  //程序无效
            } else if (keyIsAvailable == 1 && !m_shortcutLine->text().isEmpty()) {
                m_info2Label->setText(tr("Shortcut conflict"));  //快捷键冲突
            } else if (keyIsAvailable == 2 && !m_shortcutLine->text().isEmpty()) {
                m_info2Label->setText(tr("Invalid shortcut"));  //快捷键无效
            }
            if (!nameIsAvailable && !m_nameLineEdit->text().isEmpty()) {
                m_infoLabel->setText(tr("Name repetition"));  //名称重复
            }
            break;
        case 2:
            if (!nameIsAvailable && !m_nameLineEdit->text().isEmpty()) {
                m_infoLabel->setText(tr("Name repetition"));  //名称重复
            }
            if (keyIsAvailable == 1 && !m_shortcutLine->text().isEmpty()) {
                m_info2Label->setText(tr("Shortcut conflict"));  //快捷键冲突
            } else if (keyIsAvailable == 2 && !m_shortcutLine->text().isEmpty()) {
                m_info2Label->setText(tr("Invalid shortcut"));  //快捷键无效
            } else if (!execIsAvailable && !m_execLineEdit->text().isEmpty()) {
                m_info2Label->setText(tr("Invalid application"));  //程序无效
            }
            break;
        case 3:
            if (keyIsAvailable == 1) {
                m_info2Label->setText(tr("Shortcut conflict"));  //快捷键冲突
            } else if (keyIsAvailable == 2) {
                m_info2Label->setText(tr("Invalid shortcut"));  //快捷键无效
            } else if (!execIsAvailable && !m_execLineEdit->text().isEmpty()) {
                m_info2Label->setText(tr("Invalid application"));  //程序无效
            }
            if (!nameIsAvailable && !m_nameLineEdit->text().isEmpty()) {
                m_infoLabel->setText(tr("Name repetition"));  //名称重复
            }
            break;
        default:
            m_info2Label->setText(tr("Unknown error"));  //未知问题，不会触发
            break;
        }

    } else {
        m_certainBtn->setDisabled(false);
        qDebug() << Q_FUNC_INFO << "certainBtn setDisabled" << m_certainBtn->isEnabled();
    }
}

bool AddShortcutDialog::conflictWithGlobalShortcuts(const QKeySequence &keySequence)
{
    QHash<QKeySequence, QList<KGlobalShortcutInfo>> clashing;
    for (int i = 0; i < keySequence.count(); ++i) {
        QKeySequence keys(keySequence[i]);

        if (!KGlobalAccel::isGlobalShortcutAvailable(keySequence)) {
            clashing.insert(keySequence, KGlobalAccel::getGlobalShortcutsByKey(keys));
        }
    }

    if (clashing.isEmpty()) {
        return false;
    } else {
        qDebug() << "conflict With Global Shortcuts";
    }

    return true;
}

bool AddShortcutDialog::conflictWithStandardShortcuts(const QKeySequence &seq)
{
    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(seq);
    if (ssc != KStandardShortcut::AccelNone) {
        qDebug() << "conflict With Standard Shortcuts";
        return true;
    }

    return false;
}

bool AddShortcutDialog::conflictWithSystemShortcuts(const QKeySequence &seq)
{
    QString systemKeyStr = keyToLib(seq.toString());

    if (systemKeyStr.contains("Ctrl")) {
        systemKeyStr.replace("Ctrl", "Control");
    }
    for (KeyEntry& ckeyEntry : m_systemEntry) {
        if (systemKeyStr == ckeyEntry.valueStr) {
            qDebug() << "conflictWithSystemShortcuts" << seq;
            return true;
        }
    }
    return false;
}

bool AddShortcutDialog::conflictWithCustomShortcuts(const QKeySequence &seq)
{
    QString customKeyStr = keyToLib(seq.toString());

    for (KeyEntry& ckeyEntry : m_customEntry) {
        if (customKeyStr == ckeyEntry.bindingStr) {
            qDebug() << "conflictWithCustomShortcuts" << seq;
            return true;
        }
    }
    return false;
}

bool AddShortcutDialog::isKeyAvailable(const QKeySequence &seq)
{
    QString keyStr = seq.toString();

    if (!keyStr.contains("+")) {
        qDebug() << "is not Available";
        return false;
    } else if (keyStr.contains("Num") || keyStr.contains("Space")
               || keyStr.contains("Meta") || keyStr.contains("Ins") || keyStr.contains("Home")
               || keyStr.contains("PgUp") || keyStr.contains("Del") || keyStr.contains("End")
               || keyStr.contains("PgDown") || keyStr.contains("Print")
               || keyStr.contains("Backspace") || keyStr.contains("ScrollLock")
               || keyStr.contains("Return") || keyStr.contains("Enter")
               || keyStr.contains("Tab") || keyStr.contains("CapsLock")
               || keyStr.contains("Left") || keyStr.contains("Right")
               || keyStr.contains("Up") || keyStr.contains("Down")
               || keyStr.contains("Clear Grab")) {
        qDebug() << "is not Available";
        return false;
    } else {
        QStringList keys = keyStr.split("+");
        if (keys.count() == 4) {
            qDebug() << "is not Available";
            return false;
        } else {
            QString key = keys.at(keys.count() - 1);
            if (!key.contains(QRegExp("[A-Z]")) && !key.contains(QRegExp("[a-z]"))
                && !key.contains(QRegExp("[0-9]"))) {
                qDebug() << "is not Available";
                return false;
            }
        }
    }

    return true;
}

void AddShortcutDialog::setExecText(const QString &text)
{
    selectedfile = text;
    QString exec = selectedfile.section("/", -1, -1);

    GDesktopAppInfo * textinfo = g_desktop_app_info_new_from_filename(selectedfile.toUtf8().constData());
    QString appname = g_app_info_get_name(G_APP_INFO(textinfo));
    const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(textinfo)));
    setIcon(QString(QLatin1String(iconname)));

    mExec = exec;
    m_execLineEdit->setText(appname);
}

void AddShortcutDialog::setNameText(const QString &text)
{
    editName = text;
    m_nameLineEdit->setText(text);
}

void AddShortcutDialog::setKeyText(const QString &text)
{
    m_oriShortcutLineText = text;
    QString showText = text;
    showText = showText.replace("<","");
    showText = showText.replace(">"," + ");
    QString endStr = showText.mid(showText.length() - 1, 1);
    if (showText != tr("Null") && showText != "Null" && showText != "") {
        showText = showText.mid(0, showText.length() - 1) + endStr.toUpper();
    } else {
        showText = tr("Null");
    }
    QString newShowText = showText;
    if (newShowText.contains(" + ")) {
        newShowText.replace("+", " ");
    }
    if (newShowText.contains("Win")) {
        newShowText.replace("Win", "Super");
    }
    m_shortcutLine->setText(newShowText.replace("+", "   "));
    if (showText.contains("Win")) {
        showText.replace("Win", "meta");
    }
    if (showText.contains("Super")) {
        showText.replace("Super", "meta");
    }
    qDebug() << Q_FUNC_INFO << text << showText << newShowText;
    QString toSeq = keyToSequence(showText);
    QKeySequence seq(toSeq);
    editSeq = seq;
    m_shortcutLine->setKeySequence(seq);
    qDebug() << Q_FUNC_INFO << "key" << toSeq << "to KeySequence" << m_shortcutLine->keySequence().toString();
}

void AddShortcutDialog::setSourceEnable(bool enabled) {
    m_openBtn->setEnabled(enabled);
}

void AddShortcutDialog::setKeyIsAvailable(const int key) {
    keyIsAvailable = key;
}

void AddShortcutDialog::setIcon(const QString &iconname)
{
    QString iconPath = iconname;
    QFileInfo iconFile = QFileInfo(iconPath);
    QIcon appicon;

    if (appicon.hasThemeIcon(iconname)) {
        appicon = QIcon::fromTheme(iconname);
        execIcon->setPixmap(QPixmap::fromImage(appicon.pixmap(24,24).toImage()));
    } else {
        if (!iconFile.exists()) {
            iconPath  = QString("/usr/share/pixmaps/" + iconname + ".png");
            iconFile = QFileInfo(iconPath);
            if (!iconFile.exists()) {
                iconPath = QString(":/img/plugins/autoboot/desktop.png");
            }
        }
        execIcon->setPixmap(QPixmap(iconPath).scaled(QSize(24,24), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    }
}

void AddShortcutDialog::fixAddDialogTabOrder()
{
    // 选择 → 名称 → 按键 → 取消 → 确定；标题栏关闭仅鼠标可点，不进 Tab
    const QList<QWidget *> order = addDialogTabOrderList();
    if (QPushButton *closeBtn = closeButton()) {
        closeBtn->setFocusPolicy(Qt::ClickFocus);
    }
    for (int i = 0; i + 1 < order.size(); ++i) {
        QWidget::setTabOrder(order.at(i), order.at(i + 1));
    }
}

QList<QWidget *> AddShortcutDialog::addDialogTabOrderList() const
{
    QList<QWidget *> order;
    auto append = [&order](QWidget *w) {
        if (!w || !(w->focusPolicy() & Qt::TabFocus) || !w->isEnabled()) {
            return;
        }
        if (order.contains(w)) {
            return;
        }
        order.append(w);
    };
    append(m_openBtn);
    append(m_nameLineEdit);
    append(m_shortcutLine);
    append(m_cancelBtn);
    append(m_certainBtn);
    return order;
}

bool AddShortcutDialog::focusNextPrevChild(bool next)
{
    // 强制内容区 Tab 顺序，避免 KDialog 标题栏控件插进链中
    const QList<QWidget *> order = addDialogTabOrderList();
    if (order.size() < 2) {
        return KDialog::focusNextPrevChild(next);
    }
    m_pendingInitialOpenFocus = false;
    QWidget *fw = QApplication::focusWidget();
    int idx = order.indexOf(fw);
    if (idx < 0) {
        order.first()->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
        return true;
    }
    const int n = order.size();
    const int target = next ? (idx + 1) % n : (idx - 1 + n) % n;
    order.at(target)->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
    return true;
}

void AddShortcutDialog::focusOpenOrName()
{
    // TabFocusReason 才能画出键盘焦点框；已是目标控件时须先 clear 再设
    QWidget *w = (m_openBtn && m_openBtn->isEnabled())
            ? static_cast<QWidget *>(m_openBtn)
            : static_cast<QWidget *>(m_nameLineEdit);
    if (!w) {
        return;
    }
    if (QApplication::focusWidget() == w) {
        w->clearFocus();
    }
    w->setFocus(Qt::TabFocusReason);
}

void AddShortcutDialog::showEvent(QShowEvent *e)
{
    KDialog::showEvent(e);
    fixAddDialogTabOrder();
    m_pendingInitialOpenFocus = m_openBtn && m_openBtn->isEnabled();
    if (m_openBtn && m_openBtn->isEnabled()) {
        setFocusProxy(m_openBtn);
    } else if (m_nameLineEdit) {
        setFocusProxy(m_nameLineEdit);
    }
    focusOpenOrName();
    // 下一事件循环再钉一次，挡住 show 后 KDialog/LineEdit 抢焦
    QTimer::singleShot(0, this, [this]() {
        if (m_pendingInitialOpenFocus) {
            focusOpenOrName();
        }
    });
    // 短时后不再抢焦，避免挡用户 Tab/点击
    QTimer::singleShot(100, this, [this]() {
        m_pendingInitialOpenFocus = false;
    });
}

bool AddShortcutDialog::event(QEvent *e)
{
    // 激活常晚于 show，且多用 ActiveWindow（无焦点框），再补一次 Tab 焦
    if (e->type() == QEvent::WindowActivate && m_pendingInitialOpenFocus) {
        QTimer::singleShot(0, this, [this]() {
            if (m_pendingInitialOpenFocus) {
                focusOpenOrName();
            }
        });
    }
    return KDialog::event(e);
}

bool AddShortcutDialog::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusOut) {
        if (o == m_shortcutLine) {
            if (m_shortcutLine->text().isEmpty()) {
                if (m_oriShortcutLineText.isEmpty()) {
                    m_info2Label->setText(tr("Shortcut cannot be empty"));
                    m_certainBtn->setDisabled(true);
                } else {
                    m_info2Label->setText("");
                    setKeyText(m_oriShortcutLineText);
                    if (!m_nameLineEdit->text().isEmpty()) {
                        m_certainBtn->setDisabled(false);
                    }
                }
            }
        } else if (o == m_nameLineEdit) {
            if (m_nameLineEdit->text().isEmpty()) {
                m_infoLabel->setText(tr("Name cannot be empty"));
            }
        }
    } else if (e->type() == QEvent::Resize) {
        if (o == m_shortcutLine) {
            if (m_shortcutLine->sizeHint().height() > 30 && m_shortcutLine->sizeHint().height() < 40) {
                this->setFixedHeight(310);
            } else if (m_shortcutLine->sizeHint().height() >= 40 && m_shortcutLine->sizeHint().height() < 50) {
                this->setFixedHeight(358);
            }
        }
    }
    return KDialog::eventFilter(o, e);
}
