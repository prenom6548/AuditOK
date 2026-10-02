#ifndef APPCHECKFRAME_H
#define APPCHECKFRAME_H
#include <QStackedWidget>
#include "appcheckpage.h"

#include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginService/ksystemsetting_unit.h"



namespace KInstaller {
class appcheckframe : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit appcheckframe(QWidget *parent = nullptr);
    ~appcheckframe();
    void initUI();
    void addStyleSheet();
    void translateStr();
    quint16 loadItems();
    bool writeJson();
    void initFocusPolicy();//设置走焦策略
    QStackedWidget* stack;   //用于显示应用列表的控件
    qint8 mIndex;
    QLabel* m_mainTitle;
    QLabel* m_subTitle;
    QString SourcePath;
    QString Oem_Config="";

signals:
public slots:
    void clickNextButton();
    void clickPrevButton(QWidget *p);
    void getmessage(QWidget *p,QString str);
private:
    void keyPressEvent(QKeyEvent *event);
    void changeEvent(QEvent *event);

};
}
#endif // APPCHECKFRAME_H
