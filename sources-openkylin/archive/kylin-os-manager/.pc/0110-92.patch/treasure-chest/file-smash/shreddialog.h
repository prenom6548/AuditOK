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

class QLabel;
class QPushButton;
class QLineEdit;
class QComboBox;
// class ShredManager;
using namespace kdk;

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

public slots:
    void onSelectButtonClicked();
    void onShredButtonClicked();
    void onCacelButtonClicked();
    //    void onSelecteComboActivated(int index);
    void onCloseButtonClicked();
    //    void onMinButtonClicked();
    //    void progressbarFlash();
    void slotSureDialog(bool);

private:
    QLabel *mLabelTipIcon = nullptr;
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
};
