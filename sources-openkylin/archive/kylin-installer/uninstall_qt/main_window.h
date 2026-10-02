/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QMouseEvent>
#include <QPainter>
#include <qcoreevent.h>

#include "parts_title.h"
#include "parts_result.h"
#include "tool_thread.h"
#include "debinfowidget.h"
#include <kwindowsystem.h>
#include <QToolButton>
#include "./utils/DebUtil.h"

#define APP_XML_FILE_PATH "/opt/kre/config/applications_v2.xml"

#define ORG_UKUI_STYLE            "org.ukui.style"
#define GSETTING_KEY              "systemFontSize"

class main_window : public QWidget
{
   Q_OBJECT

public:
   main_window(QString , QString , QString ,QString, QWidget *parent = 0);
   ~main_window();
   void init(QString , QString , QString, QString );
   void create_interface_main();
   void creare_interface_mainbox(QString desktop);
   QString getIconPath(QString appName,QString desktop);
   void create_interrupt();
   QString getAppCnName(QString appName);
   void create_interruputbox();
   void gsettingInit();
   void dispaly_result(int i_result);
   void dispaly_resultbox(int i_result);
   void set_uninstall_lable_status(int status);
   void delete_android_soft_desktop();
   QStringList getDesktopList(QString appName);
   QStringList analysis_app_xml(QString xml_path);
   QString get_version(QString pkg_name);
   bool polkit();
   /*int judge_pkg_position(QString pkg_name);*/
   QString get_local_pkg_version(QString pkg_name);
   QString get_local_Android_pkg_version(QString pkg_name);
  // int judge_app_whether_run(QString pkg_name);
   int get_icon_from_desktop(QString desktop , char *p_icon_out , char *p_name_out , char *p_exec_out);
   QString setAutoWrap(const QFontMetrics& font, const QString& text, int nLabelSize);


   //重置界面
   QWidget         *m_pOsInstallWidget;
   //titleWidget     *m_pOstitleInstallWidget;
   debInfoWidget   *m_pOsDebMainWidget;
   QVBoxLayout     *m_pOsVboxLayoutWidget;
   QPushButton     *m_pOsInstallButton;
   QPushButton     *m_cansel_InstallButton;
   QLabel          *m_installed_tips;

   QLabel          *debIconLabel;
   QLabel          *debtextLabel;
   QToolButton     *cancelbutton;
   QToolButton     *uninstallbutton;
   QPushButton     *closeButton;

   QHBoxLayout     *iconTextLayout;
   QWidget         *iconWidget;
   QWidget         *buttonWidget;
   QHBoxLayout     *buttonLayout;

   QVBoxLayout     *allLayout;
   QWidget         *allWidget;

   QString labeltext;

   QStringList desktopList;



protected:
   void mouseMoveEvent(QMouseEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void paintEvent(QPaintEvent *event);
   void mouseReleaseEvent(QMouseEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event);

private:
   parts_title *p_title;
   debInfoWidget *p_core;
   parts_result *p_result;
   tool_thread *p_thread;
   QLabel *p_uninstall_label;
   QPushButton *p_uninstall;
   QHBoxLayout *p_hlayout_1;
   QHBoxLayout *p_hlayout_2;
   QVBoxLayout *p_vlayout_1;

   QGSettings  *m_pGsettingFontSize;
   QString m_app_name;   /*uninstaller pakgname*/
   QString m_runtime;
   QString m_version;
   QString m_zhname;
   QString m_icon_path;
   QString m_name;   /*display app name*/
   QString m_exec;
   QString desktopname;

   QPoint coordinate;

   bool m_isDragging =false;

   QPoint m_dragStartPosition;

signals:

public slots:
   void slot_close_interface();
   void slot_minisize_interface();
   void slot_uninstall();
};

#endif // MAIN_WINDOW_H
