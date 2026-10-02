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

#ifndef CENTERPOPUPWIDGET_H
#define CENTERPOPUPWIDGET_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QGSettings>
#include <QPainterPath>
#include "platformInfo.h"
#include "notifyreceiveinfo.h"
#include "adaptscreeninfo.h"
#include "centerpopupbackgroundwidget.h"

#define ORG_UKUI_STYLE "org.ukui.style"
#define ICON_THEME_NAME "iconThemeName"
#define STYLE_NAME "styleName"
#define SYSTEM_FONT "systemFont"
#define SYSTEM_FONT_SIZE "systemFontSize"
#define UKUI_TRANSPARENCY_SETTING_KEY "menuTransparency"


class CenterPopupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CenterPopupWidget(QWidget *parent = nullptr, NotifyReceiveInfo *entryInfo = 0);
    ~CenterPopupWidget();

    enum ClosedReason
    {
        TIMEOUT = 1,
        CLOSEDBYUSER = 2,
        CLOSEDBYAPP = 3,
        UNDEFINED = 4,
        HANDLEDBYUSER = 20
    };

    uint getCloseReason();
    QString getCloseActionKey();

private:
    void systemThemeChanges();
    void setWidgetAttribute();
    void initIconWidgetlayout();
    void initCloseButtonWidget();
    void initSummaryBodyWidget();
    void initButtonWidget();
    void initRightWidgetLayout();
    void initMainUiLayout();
    void updataWidgetLocation();

    void convertToImage(QString iconPath);
    QString setButtonStringBody(QString text, QToolButton *button);
    void processActionsBotton();
    QString convertMultilineText(QLabel *label, QString text);//单行文本转成多行文本

    void updataSummaryTextFormat(QString summary, QLabel *label);
    void updataBodyTextFormat(QString body, QLabel *label);
    void updataSummaryBodyTextFormat(QString summary, QLabel *summaryLabel, QString body, QLabel *bodyLabel);




Q_SIGNALS:
    void clickedCloseBtn(int id, uint closedReason);          //鼠标点击右上角退出按钮
    void clickedActionButton(int id, QString actionKey);      //点击动作按钮,发出该信号
private slots:
    void closeButtonSlots();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    QWidget *m_pIconWidget = nullptr;
    QVBoxLayout *m_pIconWidgetLayout = nullptr;
    QLabel *m_pIconLabel = nullptr;

    QWidget *m_pCloseButtonWidget = nullptr;
    QHBoxLayout *m_pCloseWidgetLayout = nullptr;
    QPushButton *m_pCloseButton = nullptr;

    QWidget *m_pSummaryBodyWidget = nullptr;
    QHBoxLayout *m_pSummaryBodyWidgetHLayout = nullptr;
    QVBoxLayout *m_pSummaryBodyWidgetVLayout = nullptr;
    QVBoxLayout *m_pSummaryBodyWidgetLayout = nullptr;
    QLabel *m_pSummaryLabel = nullptr;
    QLabel *m_pBodyLabel = nullptr;

    QWidget *m_pButtonWidget = nullptr;
    QHBoxLayout *m_pButtonWidgetLayout = nullptr;

    QWidget *m_pRightWidget = nullptr;
    QVBoxLayout *m_pRightWidgetLayout = nullptr;

    QVBoxLayout *m_pMainWidgetLayout = nullptr;

    adaptScreenInfo *m_pSreenInfo = nullptr;
    uint m_closeReason;
    QString m_closeActionKey;
    NotifyReceiveInfo *m_currentNotifyInfo = nullptr; //当前正在显示的通知

    QGSettings *m_pThemeGsetting = nullptr;
    float m_fTransparencyValue;
    CenterPopupBackgroundWidget *m_centerBackground = nullptr;
};

#endif // CENTERPOPUPWIDGET_H
