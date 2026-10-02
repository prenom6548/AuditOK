#ifndef LANGUAGEFRAME_H
#define LANGUAGEFRAME_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QTranslator>
#include "../PluginService/ui_unit/comboxdelegate.h"
#include "../PluginService/ui_unit/comboxlistmodel.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginService/ksystemsetting_unit.h"
#include <QEvent>
#include <QPushButton>
#include "pushbuttoncheck.h"
#include <QPair>
#include "scalablelabel.h"
#include <QScrollArea>

//#define LANGUAGE_ZH_CN
//#define LANGUAGE_EN_US
//#define LANGUAGE_BO_CN
////#define LANGUAGE_RU_RU
//#define LANGUAGE_ZH_HK
//#define LANGUAGE_UG_CN
//#define LANGUAGE_KK_CN
//#define LANGUAGE_KY_CN

using namespace KServer;
namespace KInstaller {

class LanguageFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit LanguageFrame(QWidget *parent = nullptr);
    ~LanguageFrame();

public:    
    void initUI();                              //加载界面控件
    void translateStr();                        //更新需要翻译的对象字符串
    void addStyleSheet();                       //添加窗口样式
    void readSettingIni();                      //读写systemsetting.ini文件
    void writeSettingIni();                     //读写systemsetting.ini文件

    void initFocusPolicy();                     //初始化焦点序列
    QList<QPushButton*> m_LanguageBtnList;

public slots:
    void changeLanguage(PushButtonCheck* obj);
    void clickNextButton();

signals:
    void changeLanguage_sig(QString objname);

private:
    void changeEvent(QEvent *event) override;       //改变语言事件

    QTranslator* m_pQtTranslator = nullptr;
    QTranslator* m_pCurTranslator = nullptr;

    QComboBox* m_languageBox;
    ScalableLabel* m_mainTitle;

    QString m_curSystenlocal;               //当前选中的语言名称
    ComboxListModel* m_model;               //model
    ItemLanguage m_comboxItem;              //列表项

    QLabel* m_kylinpng;
    QLabel* m_titleOS;
    QLabel* m_littleOS;
    QList<QPair<QString, QPair<PushButtonCheck*,QString>>> emulang;
    QScrollArea * m_pBtnScroll;
    bool m_firstfocusget=false;//首次焦点获取标志(首次获取时隐藏边框)
    // QWidget interface
protected:
    void showEvent(QShowEvent *event);

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
protected:
    void keyPressEvent(QKeyEvent * event);
};
}
#endif // LANGUAGEFRAME_H
