/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "kylineditbutton.h"
#include "mytitlebar.h"
#include "toolkits.h"
#include "utils.h"
#include "./shredqthread.h"
#include "filewipe.h"
//#include "shredmanager.h"
#include <QObject>
#include <QStringList>
#include <QCloseEvent>
#include <QBitmap>
#include <QFileDialog>
#include <QDir>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>
#include <QDialog>
#include <QSettings>
#include <QProgressBar>
#include <QTimer>
#include "kwidget.h"
#include "kalertdialog.h"
#include "ksuredialog.h"
#include "kpushbutton.h"
#include <QFrame>
#include <QPaintEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QUrl>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <QTreeWidget>
#include "kballontip.h"
#include "shredfailinfodialog.h"
#include "kdragwidget.h"
class QLabel;
class QPushButton;
class QLineEdit;
class QComboBox;
// class ShredManager;
using namespace kdk;

class TableDivider : public QFrame
{
    Q_OBJECT
public:
    TableDivider(QWidget *parent = nullptr);
    ~TableDivider() = default;

Q_SIGNALS:
    void sendFiles(QList<QUrl> file_list);

protected:
    void paintEvent(QPaintEvent *event);
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

class Divider : public QFrame
{
    Q_OBJECT
public:
    Divider(QWidget *parent = nullptr);
    ~Divider() = default;

Q_SIGNALS:
    void sendFiles(QList<QUrl> fileList);
    void openFileDialog();

protected:
    void paintEvent(QPaintEvent *event);
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

class ShredDialog : public KWidget
// class ShredDialog : public QDialog
{
    Q_OBJECT
public:
    //  ShredDialog(ShredManager *plugin, QDialog *parent = 0);
    explicit ShredDialog(QWidget *parent = 0);
    ~ShredDialog();
    void setLanguage();
    void initConnect();
    //  void initTitleBar();
    //  QString getCurrrentSkinName();
    //  void resetSkin();

    void moveCenter();

protected:
    void closeEvent(QCloseEvent *event);
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event);

public Q_SLOTS:
    void onSelectButtonClicked();
    void onShredButtonClicked();
    void onCacelButtonClicked();
    //    void onSelecteComboActivated(int index);
    void onCloseButtonClicked();
    //    void onMinButtonClicked();
    //    void progressbarFlash();
    void slotSureDialog(bool);
    void slotShredFileFail(QStringList failFileName, QStringList successFileName, QStringList allFailFiles);

private:
    QPushButton *mLabelTipIcon = nullptr;
    QLabel *mLabelTipText = nullptr;
    QLineEdit *mLabelFilePath = nullptr;
    QPushButton *shred_btn = nullptr;
    QPushButton *cacel_btn = nullptr;
    QPushButton *mBtnAddFile = nullptr;

    KAlertDialog *alertDialog = nullptr;
    KSureDialog *sureDialog = nullptr;

    QLabel *tipLabel = nullptr;
    //    ShredManager *process_plugin;
    //    MyTitleBar *title_bar;

    QPoint dragPosition;
    //    QProgressBar *progressbar;
    QLabel *barLabel;

    //    QTimer myTimer;

    ShredQThread *myThread;
    QThread *thread;



    bool mousePressed;
    int i = 0;
    //    QSettings *shredSettings;
    bool refreshInfoContents(QUrl url);
    void queryFileType(GFileInfo *new_info);
    void acceptDrogFiles(QList<QUrl> fileList);
    void acceptDrogOrChooseFile(QString filePath);
    void addFileToTable(QString filepath);
    // void acceptDrogFilesString(QStringList fileList);
    void needOpenFileDialog();
    void tipWidgetInit(QString msg, kdk::TipType type);
    void tipWidgetMove();

    QString m_iconName = "";
    bool m_canDelete = true;
    bool m_canWrite = true;
    bool m_canRead = true;
    bool m_canExcute = true;
    bool m_isDir = false;
    bool m_isVolume = false;
    bool m_isSymbolLink = false;

    KBallonTip *m_shredingTip = nullptr;
    //    KBallonTip *m_shredFinishTip = nullptr;
    //    KBallonTip *m_shredingTip = nullptr;
    //    KBallonTip *m_shredFinishTip = nullptr;
    QMenu *m_imageMenu = nullptr;
    Divider *m_divider = nullptr;
    // KDragWidget *m_divider = nullptr;
    TableDivider *m_tableDivider = nullptr;
    QTreeWidget *m_fileTable = nullptr;
    QTimer *m_timer = nullptr;
    void insertFileData(QFileInfo fileInfo);
    void initFileTable();
    void changeBtnState();
    // void onItemChanged(QTreeWidgetItem *item, int indexCol);
    void onItemCLicked(QTreeWidgetItem *item, int indexCol);
    QStringList getAllSelectedFiles();
    void removeSelectedFiles(QStringList selectedFiles);
    void controlUiDisplay();
    bool fileHasAdded(QString filepath);
    void customContextMenu(const QPoint &pos);
    void selectAll();
    int m_backItemRow = -1;
    bool m_backItemSelected = true;
    void setTextToolTips(QTreeWidgetItem *item, int indexCol);
    void addFileToItemShowTip(QList<QUrl> fileList);
    // void addFileToItemShowTipString(QStringList fileList);
    QStringList getAllFailFiles(QStringList allFails, bool onlyKeyPath = false);
    void showFailDialog(QStringList failFilesPath);
    QStringList m_allFailFilePath = {};
    //    ShredFailInfoDialog *m_failTipDialog = nullptr;
    void initDbus();
    QStringList m_keyPath = {};
    QStringList m_defaultKeyPath = {
        "/usr/sbin/useradd",
        "/usr/sbin/userdel",
        "/usr/sbin/adduser",
        "/usr/sbin/deluser",
        "/usr/bin/peony",
        "/usr/bin/mount",
        "/usr/bin/umount",
        "/usr/sbin/ksc-virus-daemon",
        "/",
        "/opt",
        "/home",
        "/data",
        "/usr/lib/x86_64-linux-gnu/nemo/extensions-3.0/libsafe-context-menu-nemo.so",
        "/usr/lib/aarch64-linux-gnu/caja/extensions-2.0/libsafe-context-menu-caja.so",
        "/usr/lib/aarch64-linux-gnu/caja/extensions-2.0/libsafe-context-menu.so",
        "/usr/lib/mips64el-linux-gnu/caja/extensions-2.0/libsafe-context-menu-caja.so",
        "/usr/lib/x86_64-linux-gnu/nautilus/extensions-3.0/libsafe-context-menu-nautilus.so",
        "/usr/lib64/nautilus/extensions-3.0/libsafe-context-menu-nautilus.so",
        "/usr/lib64/caja/extensions-2.0/libsafe-context-menu-caja.so",
        "/usr/lib/caja/extensions-2.0/libsafe-context-menu-caja.so",
        "/usr/lib/x86_64-linux-gnu/nemo/extensions-3.0/libsafe-context-menu.so",
        "/usr/lib/mips64el-linux-gnu/caja/extensions-2.0/libsafe-context-menu.so",
        "/usr/lib/x86_64-linux-gnu/nautilus/extensions-3.0/libsafe-context-menu.so",
        "/usr/lib64/nautilus/extensions-3.0/libsafe-context-menu.so",
        "/usr/lib64/caja/extensions-2.0/libsafe-context-menu.so",
        "/usr/lib/caja/extensions-2.0/libsafe-context-menu.so",
        "/usr/lib/peony-qt-extensions/libsafe-context-menu.so",
        "/usr/lib/peony-qt-extensions/libpeony-qt-menu-plugin-qax.so",
        "/usr/lib/x86_64-linux-gnu/peony-extensions/libsafe-context-menu.so",
        "/usr/lib/aarch64-linux-gnu/peony-extensions/libsafe-context-menu.so",
        "/usr/lib/mips64el-linux-gnu/peony-extensions/libsafe-context-menu.so",
        "/usr/lib/aarch64-linux-gnu/caja/extensions-2.0/libcaja-qax-menu.so",
        "/usr/lib64/caja/extensions-2.0/libcaja-qax-menu.so",
        "/usr/lib/x86_64-linux-gnu/nemo/extensions-3.0/libnemo-qax-menu.so",
        "/usr/lib64/nemo/extensions-3.0/libnemo-qax-menu.so",
        "/usr/lib64/nautilus/extensions-3.0/libnautilus-qax-menu.so",
        "/usr/lib/x86_64-linux-gnu/caja/extensions-2.0/libwps-print.so",
        "/usr/lib64/caja/extensions-2.0/libwps-print.so",
        "/usr/lib/aarch64-linux-gnu/caja/extensions-2.0/libwps-print.so",
        "/usr/lib/mips64el-linux-gnu/caja/extensions-2.0/libwps-print.so",
        "/etc/crossover.conf",
        "/opt/apps",
        "/opt/apps-third",
        "/opt/third"};
    void getKeyPathFromSys(int sourceType);
    QStringList m_choseKeyPathList = {};
    int m_failDirNum = 0;
//只需要遍历关键路径的时候，需要这个数量，因为设计要求只有文件夹的时候，需要显示空的失败文件列表，但需要显示失败弹窗

Q_SIGNALS:
    void signalShowAddFileTip(QList<QUrl> fileList);
    void signalShowAddTip(QString filePath);
    // void signalShowAddFileTipStringList(QStringList fileList);
};
