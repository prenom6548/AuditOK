#ifndef APPVERSIONERRORWARNINGWIDGET_H
#define APPVERSIONERRORWARNINGWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QIcon>
#include <QComboBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <klabel.h>
#include <QApplication>

#include "kaglobal.h"

#include "libukcc/widgets/SettingWidget/addbutton.h"
#include "libukcc/widgets/SettingWidget/settinggroup.h"
#include "ukui-log4qt.h"

#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/kmessagebox.h>
#include <aiconfig/modelconfig.h>

class AppVersionErrorWarningWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppVersionErrorWarningWidget(QWidget *parent = nullptr);
    void initLayout();
    void loadSoftwareStore();//加载软件商店
    void updateTips(QString tips);
private:
    QHBoxLayout *m_mainHLayout;
    QLabel *m_iconLabel;
    QLabel *m_tipsLabel;
    QPushButton  *m_configButton;

};

#endif // APPVERSIONERRORWARNINGWIDGET_H
