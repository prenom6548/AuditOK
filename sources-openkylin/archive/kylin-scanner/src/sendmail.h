/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef SENDMAIL_H
#define SENDMAIL_H


#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QFrame>
#include <QDialog>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QProcess>
#include <QPaintEvent>
#include <QBitmap>
#include <QPixmap>
#include <QFileInfo>
#include <QFileIconProvider>
#include <qmath.h>
#include <QTextBrowser>
#include  <ukui-log4qt.h>
#include "globalsignal.h"
#include "include/common.h"
#include "include/theme.h"
#include "custom_push_button.h"
#ifdef signals
#undef signals
#endif
extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
}

#define MailType "x-scheme-handler/mailto"
#define DesktopFilePath "/usr/share/applications/"
#define MailPicturePath					"~/.config/kylin-scanner/images/mail.jpg"

#define NoMailWindowWidth 380
#define NoMainWindowHeight 216

#define SendMailWindowWidth 380
#define SendMainWindowHeight 176
#define UKUI_THEME_GSETTING_PATH "org.ukui.style"


typedef struct _Applist {
    char *appid;
} AppList;
AppList *getAppIdList(const char *contentType);

typedef struct _AppInfo {
    GAppInfo *item;
} Appinfo;
Appinfo *_getAppList(const char *contentType);

class NoMailDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NoMailDialog(QWidget *parent = nullptr);
    ~NoMailDialog();

    void initWindow();
    void initLayout();
    void initConnect();

private:
    QPushButton *m_noMailCloseButton = nullptr;
    QHBoxLayout *m_noMailTitleHBoxLayout = nullptr;

    QLabel *m_noMaillogoLabel = nullptr;
    QLabel *m_noMailtitleLabel = nullptr;
    QHBoxLayout *m_noMailLogoTitleHBoxLayout = nullptr;

    QTextBrowser *m_noMailInfoLabel = nullptr;
    QHBoxLayout *m_noMailInfoHBoxLayout = nullptr;

    kdk::KPushButton *m_cancelButton = nullptr;
    kdk::KPushButton *m_installButton = nullptr;
    QHBoxLayout *m_noMailButtonsHBoxLayout = nullptr;

    QVBoxLayout *m_mainVBoxLayout = nullptr;
    QGSettings *m_themeData = nullptr;

Q_SIGNALS:
   void noMailWindowClose();

public slots:

    void closeNoMailWindow();
    void fontSizeChangedSlot();
    void installMailApp();
};

class SendMailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendMailDialog(QWidget *parent = nullptr);
    ~SendMailDialog();

    void initWindow();
    void initLayout();
    void initConnect();

    void setMailSelectComboboxItems();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel *m_titleTextLabel = nullptr;
    QPushButton *m_closeButton = nullptr;
    QHBoxLayout *m_titleHBoxLayout = nullptr;

    QLabel *m_mailSelectLabel = nullptr;
    QComboBox *m_mailSelectCombobox = nullptr;
    QHBoxLayout *m_mailSelectHBoxLayout = nullptr;

    CustomPushButton *m_cancelButton = nullptr;
    CustomPushButton *m_confirmButton = nullptr;
    QHBoxLayout *m_buttonsHBoxLayout = nullptr;

    QVBoxLayout *m_mainVBoxLayout = nullptr;

    QList<QString> m_desktopName;

Q_SIGNALS:
    void sendMailWindowClose();

public slots:
    void closeSendMailWindowSlot();
    void openSelectMailClientSlot(QString name);

};


#endif // SENDMAIL_H
