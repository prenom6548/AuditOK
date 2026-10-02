#ifndef LICENSEFRAME_H
#define LICENSEFRAME_H

#include <QObject>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QGridLayout>
#include <QTextBrowser>
#include <QCheckBox>
#include <QLabel>
#include <QTimer>
#include "../PluginService/ui_unit/AutoResize.h"
#include "../PluginService/ui_unit/middleframemanager.h"

namespace KInstaller {


class LicenseFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit LicenseFrame(QWidget *parent = nullptr);

    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void loadLicense();
    void translateStr();
    QString readSettingIni();
    void setBoardTabOrder();
    void initFocusPolicy();//设置走焦策略
signals:
public slots:
    void clickNextButton();
    void nextBtnAvailable();
//    void slotTimer();
    void checkScrollEnd(int value);
private:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
    AutoResize *m_autoResizeHandler;
//    void resizeEvent(QResizeEvent * event);
public:
    bool lang_bl;
    QGridLayout *licneseLayout;
    QTextBrowser *license_text;
    QLabel* label_text;

    QLabel* m_mainTitle;
    QString m_fileStr;
    QString path;
    QString lang;
    QCheckBox* m_timetip;
    int scrollbarInt;
    double tmpDouble;


    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};
}
#endif // LICENSEFRAME_H
