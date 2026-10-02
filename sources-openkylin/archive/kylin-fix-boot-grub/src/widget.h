#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QProcess>
#include <QTranslator>
#include <QPainterPath>
#include "centerwidget.h"
#include "centerstackwidget.h"
#include "fixfailedwidget.h"
#include "successwidget.h"
#include "repairingwidget.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void initTranslation();
    void initWidgetFullDesktop();
    void initUI();
    void initConnectSlots();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void switchStackWidget(bool status);
    void switchFixingWidget(QString diskName);
    void bashFixProcessFinishSlots(int exitCode, QProcess::ExitStatus exitStatus);

public:
    QVBoxLayout *m_pMainVboxLayout = nullptr;
    centerWidget *m_pCenterWidget = nullptr;
    centerStackWidget *m_pCeterStackWidget = nullptr;
    fixFailedWidget   *m_pFixFailedWidget = nullptr;
    successWidget     *m_pSuccessWidget = nullptr;
    repairingWidget   *m_pRepairingWidget = nullptr;
    QProcess          *m_pBashFixProcess = nullptr;
    QTranslator       *m_ptranslator = nullptr;
};
#endif // WIDGET_H
