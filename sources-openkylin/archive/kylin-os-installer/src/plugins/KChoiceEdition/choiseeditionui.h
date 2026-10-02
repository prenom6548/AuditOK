#ifndef CHOISEEDITIONUI_H
#define CHOISEEDITIONUI_H
#include <QLabel>
#include <QScrollArea>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QMap>
#include <QKeyEvent>
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "editiondirinfo.h"

//#include "../KChoiceLanguage/pushbuttoncheck.h"

using namespace KInstaller;

class choiseEditionUI : public MiddleFrameManager
{
    Q_OBJECT

public:
    explicit choiseEditionUI(QWidget *parent = nullptr);
    void initUI();
    void initAllConnect();
    void translateStr();
    void readEditionsFile();
    void addStyleSheet();
    void initFocusPolicy();                     //初始化焦点序列
public slots:
    void clickNextButton();
//    void nextBtnAvailable();
    void getSelect(editionDirInfo* getChoiceButt);

signals:
    void changeButton_sig(editionDirInfo* objname);

private:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);

public:
    QGridLayout *m_mainLayout;
    QLabel* m_mainTitle;
    QScrollArea* m_pScroll;
    QWidget* m_psWidget;
    QVBoxLayout* m_vtlayout;

    QString m_strSelectValue;
    editionDirInfo *m_strSelectButt;                     //当前选中的按钮
    QList<editionDirInfo*> m_EditionsList;               //安装版本列表
};

#endif // CHOISEEDITIONUI_H
