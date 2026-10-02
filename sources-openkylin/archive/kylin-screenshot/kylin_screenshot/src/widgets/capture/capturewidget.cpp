/* Copyright (C) 2020, KylinSoft Co., Ltd.
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

// Based on Lightscreen areadialog.cpp, Copyright 2017  Christian Kaiser <info@ckaiser.com.ar>
// released under the GNU GPL2  <https://www.gnu.org/licenses/gpl-2.0.txt>

// Based on KDE's KSnapshot regiongrabber.cpp, revision 796531, Copyright 2007 Luca Gugelmann <lucag@student.ethz.ch>
// released under the GNU LGPL  <http://www.gnu.org/licenses/old-licenses/library.txt>
#include "src/utils/systemnotification.h"
#include "src/utils/filenamehandler.h"
#include "capturewidget.h"
#include "src/widgets/capture/hovereventfilter.h"
#include "src/widgets/panel/sidepanelwidget.h"
#include "src/utils/colorutils.h"
#include "src/utils/globalvalues.h"
#include "src/widgets/capture/notifierbox.h"
#include "src/widgets/capture/colorpicker.h"
#include "src/utils/screengrabber.h"
#include "src/utils/systemnotification.h"
#include "src/utils/screenshotsaver.h"
#include "src/core/controller.h"
#include "src/widgets/capture/modificationcommand.h"
#include <QUndoView>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QShortcut>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QMouseEvent>
#include <QBuffer>
#include <QDesktopWidget>
#include <QDebug>
#include <QRegion>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QBitmap>
#include <KF5/KWindowSystem/KWindowSystem>
#include "ukuiwindowhelper/ukuiwindowhelper.h"
//#include <X11/Xlib.h>
//#include <X11/extensions/shapeconst.h>
//#include <X11/extensions/shape.h>
#include <QtX11Extras/QX11Info>
#include <src/config/movelabel.h>
#include <qdbusconnection.h>
#include <qdbusinterface.h>
#ifndef UNUI
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#endif
#ifdef ENABLE_RECORD
#include "ssrtools.h"
#include "mypopup.h"
#endif
#include "../ukui-window-switch-wlcom/multitaskviewmodelbywlcomhandler.h"
// CaptureWidget is the main component used to capture the screen. It scontains an
// are of selection with its respective buttons.

// enableSaveWIndow
CaptureWidget::CaptureWidget(const uint id, const QString &savePath, bool fullScreen,
                             QWidget *parent) :
    QWidget(parent),
    m_mouseIsClicked(false),
    m_rightClick(false),
    m_newSelection(false),
    m_grabbing(false),
    m_captureDone(false),
    m_previewEnabled(true),
    m_adjustmentButtonPressed(false),
    m_activeButton(nullptr),
    m_activeTool(nullptr),
    m_toolWidget(nullptr),
    m_mouseOverHandle(SelectionWidget::NO_SIDE),
    m_id(id)
  #ifdef ENABLE_RECORD
    ,
    m_isolatedButtons{}
  #endif
{
    isrunning = new QGSettings("org.ukui.screenshot");
    isrunning->set("isrunning", true);
    // Base config of the widget
    m_eventFilter = new HoverEventFilter(this);
    connect(m_eventFilter, &HoverEventFilter::hoverIn,
            this, &CaptureWidget::childEnter);
    connect(m_eventFilter, &HoverEventFilter::hoverOut,
            this, &CaptureWidget::childLeave);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    m_showInitialMsg = m_config.showHelpValue();
    m_opacity = m_config.contrastOpacityValue();
    setMouseTracking(true);
    initContext(savePath, fullScreen);
    initShortcuts();

#ifdef Q_OS_WIN
    // Top left of the whole set of screens
    QPoint topLeft(0, 0);
#endif
    if (fullScreen) {
        // Grab Screenshot
        bool ok = true;
        m_context.screenshot = ScreenGrabber().grabEntireDesktop(ok);
        if (!ok) {
#ifndef UNUI
            SystemNotification().sendMessage(tr("Unable to capture screen"));
            hide_window();
            this->close();
#endif
        }
        m_context.origScreenshot = m_context.screenshot;
#ifdef Q_OS_WIN
        setWindowFlags(Qt::WindowStaysOnTopHint
                       | Qt::FramelessWindowHint
                       | Qt::Popup);

        for (QScreen * const screen : QGuiApplication::screens()) {
            QPoint topLeftScreen = screen->geometry().topLeft();
            if (topLeft.x() > topLeftScreen.x()
                || topLeft.y() > topLeftScreen.y()) {
                topLeft = topLeftScreen;
            }
        }
        move(topLeft);
#else
        setWindowFlags(Qt::BypassWindowManagerHint
//                       | Qt::WindowStaysOnTopHint
                       | Qt::FramelessWindowHint
                       | Qt::Tool);
#ifndef UNUI
//         KWindowSystem::setType(winId(), NET::OnScreenDisplay);
         UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
         windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::SystemWindow);
#endif
#endif
        resize(pixmap().size().width()/m_context.screenshot.devicePixelRatio(), pixmap().size().height()/m_context.screenshot.devicePixelRatio());
    }
    auto devicePixelRatio = m_context.screenshot.devicePixelRatio();
#ifndef UNUI
    if (m_info.waylandDectected())
    {
        MultitaskViewModelByWlcomHandler *wlcomHandler = new MultitaskViewModelByWlcomHandler();
        for (int i = 0; i < wlcomHandler->logicalScreenCount(); ++i)
        {
            QList<WlcomWindow> wlcomWinList = wlcomHandler->createWindowList(wlcomHandler->getCurrentDesktop(), i);
            QVector <QRect > tmpRects;
            for (int j = 0; j < wlcomWinList.count(); ++j)
            {
                if (wlcomWinList[j].minimized())
                {
                    continue;
                }

                QRect rect = QRect(wlcomWinList[j].x(),
                             wlcomWinList[j].y(),
                             wlcomWinList[j].width(),
                             wlcomWinList[j].height());
                if (tmpRects.count() == 0)
                {
                    tmpRects.append(rect);
                }
                else
                {
                    int insertIndex = 0;
                    if ((rect.x() == 0) && (rect.y() == 0))
                    {
                        tmpRects.insert(insertIndex, rect);
                    }
                    else
                    {
                        tmpRects.append(rect);
                    }
                }
            }

            rects.append(tmpRects);
        }
    }
    else
    {
        // 当前窗口活跃状态存储窗口ID
        QList<WId> windows = KWindowSystem::stackingOrder();
        for (long long unsigned int const id: windows) {
            // 获取窗口状态信息
            KWindowInfo info(id, NET::WMState);
            // 判断窗口是否显示
            if (false == info.hasState(NET::Hidden)) {
                // 获取窗口位置 长宽信息
                KWindowInfo info2(id, NET::WMFrameExtents);
                rects.append(QRect(info2.frameGeometry().x()/devicePixelRatio,
                                   info2.frameGeometry().y()/devicePixelRatio,
                                   info2.frameGeometry().width()/devicePixelRatio,
                                   info2.frameGeometry().height()/devicePixelRatio
                                   )
                             );
            }
        }
    }
#endif
    isDrag = false;
    isfirstMove = false;
    isfirstPress = false;
    isfirstRelease = false;
    isPressButton = false;
    isReleaseButton = false;
    m_grabbing = false;
    isMove = false;
    // Create buttons
    m_lastHoverRect = QRect();
    taskbar_label = new  MoveLabel(this);
    if (Controller::getInstance()->isManagerStart)
        taskbar_label->setFixedSize(632, 44);
    else
        taskbar_label->setFixedSize(712, 44);
    // 设置阴影边框
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 2);
    shadowEffect->setColor(QColor(0, 0, 0, 70));
    shadowEffect->setBlurRadius(20);
    taskbar_label->setGraphicsEffect(shadowEffect);

    line_1 = new QFrame(taskbar_label);
    line_1->setFrameShape(QFrame::VLine);
    line_1->setFixedSize(1, 16);
    line_1->move(378, 15);

    line_2 = new QFrame(taskbar_label);
    line_2->setFrameShape(QFrame::VLine);
    line_2->setFixedSize(1, 16);
    line_2->move(438, 15);

    m_buttonHandler = new ButtonHandler(taskbar_label);
    taskbar_label->m_buttonHandler = m_buttonHandler;
    if (Controller::getInstance()->isManagerStart)
        updateButtonsInManager();
    else
        updateButtons();
    if (m_context.fullscreen) {
        for (QScreen * const screen : QGuiApplication::screens()) {
            QRect r = QRect(screen->geometry());
#ifdef Q_OS_WIN
            r.moveTo(r.topLeft() - topLeft);
#endif
            areas.append(r);
        }
    } else {
        areas.append(rect());
    }
    m_buttonHandler->updateScreenRegions(areas);
    m_buttonHandler->hide();
    taskbar_label->hide();
    initSelection();
    updateCursor();
    // Init color picker
    m_colorPicker = new ColorPicker(this);
    font_color = new FontSize_Color_Chose(this);
    font_color2 = new FontSize_Color_Chose2(this);
    save_location = new  Save_Location(this);
    save_location2 = new Save_Location2(this);
    font_options = new  Font_Options(this);
    font_options2 = new Font_Options2(this);
#ifdef ENABLE_RECORD
    recorder = new Recorder(this);
#endif

    connect(m_colorPicker, &ColorPicker::colorSelected,
            this, &CaptureWidget::setDrawColor);
    connect(font_color, &FontSize_Color_Chose::font_size_change,
            this, &CaptureWidget::setDrawThickness);
    connect(font_color, &FontSize_Color_Chose::colorSelected,
            this, &CaptureWidget::setDrawColor);
    connect(font_color2, &FontSize_Color_Chose2::font_size_change2,
            this, &CaptureWidget::setDrawThickness);
    connect(font_color2, &FontSize_Color_Chose2::colorSelected2,
            this, &CaptureWidget::setDrawColor);
    connect(font_options, &Font_Options::colorSelected,
            this, &CaptureWidget::setDrawColor);
    connect(font_options, &Font_Options::font_size_Selete,
            this, &CaptureWidget::setTextDrawThickness);
    connect(font_options, &Font_Options::font_type_Selete,
            this, &CaptureWidget::font_type_changed);
    connect(font_options, SIGNAL(font_bold_change(bool)),
            this, SLOT(font_bold_clicked(bool)));
    connect(font_options, SIGNAL(font_italic_change(bool)),
            this, SLOT(font_italic_clicked(bool)));
    connect(font_options, SIGNAL(font_underline_change(bool)),
            this, SLOT(font_underline_clicked(bool)));
    connect(font_options, SIGNAL(font_delete_change(bool)),
            this, SLOT(font_delete_clicked(bool)));
    connect(font_options2, &Font_Options2::font_size_Selete,
            this, &CaptureWidget::textThicknessChanged);
    connect(font_options2, &Font_Options2::font_type_Selete,
            this, &CaptureWidget::font_type_changed);
    connect(font_options2, SIGNAL(font_bold_change(bool)),
            this, SLOT(font_bold_clicked(bool)));
    connect(font_options2, SIGNAL(font_italic_change(bool)),
            this, SLOT(font_italic_clicked(bool)));
    connect(font_options2, SIGNAL(font_underline_change(bool)),
            this, SLOT(font_underline_clicked(bool)));
    connect(font_options2, SIGNAL(font_delete_change(bool)),
            this, SLOT(font_delete_clicked(bool)));
    connect(font_options2, &Font_Options2::colorSelected,
            this, &CaptureWidget::setDrawColor);
    connect(save_location, &Save_Location::save_type_clicked,
            this, &CaptureWidget::ClickedSaveType);
    connect(save_location2, &Save_Location2::save_type_clicked,
            this, &CaptureWidget::ClickedSaveType2);

    m_colorPicker->hide();
    font_color->setStartPos(95);
    font_color->setTriangleInfo(20, 10);
    font_color->setFixedSize(QSize(192, 67));

    font_color2->setStartPos(95);
    font_color2->setTriangleInfo(20, 10);
    font_color2->setFixedSize(QSize(187, 62));

    save_location->setStartPos(72);
    save_location->setTriangleInfo(20, 10);
    save_location->setFixedSize(QSize(151, 143));
    save_location->setCenterWidget();

    save_location2->setStartPos(72);
    save_location2->setTriangleInfo(20, 10);
    save_location2->setFixedSize(QSize(151, 135));
    save_location2->setCenterWidget();

    font_options->setStartPos(160);
    font_options->setTriangleInfo(16, 8);
    font_options->setFixedSize(QSize(334, 89));
    font_options->setCenterWidget();

    font_options2->setStartPos(160);
    font_options2->setTriangleInfo(16, 8);
    font_options2->setFixedSize(QSize(334, 89));
    font_options2->setCenterWidget();

    size_label = new QLabel(this);
    size_label->setFixedSize(82, 24);
    size_label->hide();
    size_label->setAlignment(Qt::AlignCenter);

#ifndef SUPPORT_NEWUI
    m_context.saveType = ".png";
    QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    m_context.savePath = a.at(0);
#endif
    // Init notification widget
    m_notifierBox = new NotifierBox(this);
    m_notifierBox->hide();
#ifdef ENABLE_RECORD
    recorder->m_pushbutton_cancel->setFixedSize(QSize(50, 50));
    recorder->m_pushbutton_cancel->move(200, 100);
    recorder->m_pushbutton_cancel->setText(QString::fromUtf8("cancel"));
    recorder->m_pushbutton_save->setFixedSize(QSize(50, 50));
    recorder->m_pushbutton_save->setText(QString::fromUtf8("save"));
    recorder->m_pushbutton_save->move(300, 100);
    recorder->m_pushbutton_cancel->hide();
    recorder->m_pushbutton_save->hide();
#endif
    font_color_point = new QPoint();
    connect(&m_undoStack, &QUndoStack::indexChanged,
            this, [this](int){
        this->update();
    });
    initPanel();
    QFont ft;
    ft.setPointSize(10);
    SaveAs_btn = new  QPushButton(this);
    SaveAs_btn->setFixedSize(90, 30);
    SaveAs_btn->setFont(ft);
    SaveAs_btn->setStyleSheet("QPushButton{background-color:rgb(0,98,240)}");
    SaveAs_btn->setText(tr("save as"));

    hide_ChildWindow();
    connect(SaveAs_btn, SIGNAL(pressed()), this, SLOT(ClickedSaveAsFile()));

    m_scrollPreview = new QLabel(this);
    m_scrollPreview->installEventFilter(this);
    initTipDock();
}

CaptureWidget::~CaptureWidget()
{
#ifndef UNUI
    if (m_captureDone) {
        emit captureTaken(m_id, this->pixmap(), true);
    } else {
        emit captureFailed(m_id);
    }
    m_config.setdrawThickness(m_context.thickness);

    if(line_1) {
        delete line_1;
    }

    if(line_2) {
        delete line_2;
    }

#endif
}

// redefineButtons retrieves the buttons configured to be shown with the
// selection in the capture
void CaptureWidget::updateButtons(
        #ifdef ENABLE_RECORD
    bool isRecord
        #endif
    )
{
#ifdef ENABLE_RECORD
    vectorButtons.clear();
#endif
    m_uiColor = m_config.uiMainColorValue();
    m_contrastUiColor = m_config.uiContrastColorValue();
    auto buttons = m_config.getButtons(
            #ifdef ENABLE_RECORD
        isRecord
            #endif
        );
    for (const CaptureButton::ButtonType &t: buttons) {
        CaptureButton *b = new CaptureButton(t, taskbar_label);
        b->setAccessibleName(b->tool()->objectName());
#ifndef SUPPORT_NEWUI
        if (t == CaptureButton::TYPE_OPTION) {
            m_sizeIndButton = b;
            m_sizeIndButton->setColor(Qt::black);
        } else
#endif
        if (t == CaptureButton::TYPE_SAVE) {
            b->setColor(Qt::black);
        } else {
            b->setColor(m_uiColor);
        }
        makeChild(b);
#ifdef ENABLE_RECORD
        if (b->tool()->isIsolated()) {
            m_isolatedButtons.insert(b->m_buttonType, b->tool());
        }

#endif
        connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->tool(), &CaptureTool::requestAction,
                this, &CaptureWidget::handleButtonSignal);
        connect(b->tool(), &CaptureTool::captureExit,
                this, &CaptureWidget::captureExit);

        if (t == CaptureButton::TYPE_SCROLL) {
            connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::onScrollButtonPressed);
        }

        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
}

QPixmap CaptureWidget::pixmap()
{
    QPixmap p;
    if (m_toolWidget && m_activeTool) {
#ifndef UNUI
        p = m_context.selectedScreenshotArea().copy();
        QPainter painter(&p);
        m_activeTool->process(painter, p);
#endif
    } else {
        p = m_context.selectedScreenshotArea();
    }
    return m_context.selectedScreenshotArea();
}

void CaptureWidget::deleteToolwidgetOrClose()
{
#ifndef UNUI
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
        m_toolWidget = nullptr;
    } else {
        hide_window();
        emit captureExit(m_id);
        close();
    }
#endif
}

void CaptureWidget::size_label_option()
{
#ifndef UNUI
    if (isScrolling()) {
        size_label->hide();
        return;
    }

    QString str;
    QString borderRadius =  "classical" == m_context.style_settings->get("widgetThemeName").toString() ? "0px" : "4px";
    if ((m_context.style_name.compare("ukui-dark") == 0)
        || (m_context.style_name.compare("ukui-black") == 0)) {
        taskbar_label->setStyleSheet(QString("QLabel {"
                                     "border-width: 0px; "
                                     "border-radius: %1; "
                                     "background-color:rgb(25,25,25,209);}").arg(borderRadius));
        str = QString("QLabel {"
              "border-width: 0px; "
              "border-radius: %1; "
              "background-color: rgb(25,25,25,180);"
              "font-size: 15px;"
              "color: white}").arg(borderRadius);
        line_1->setStyleSheet("QFrame{border: 1px solid #727272;background-color: #727272;}");
        line_2->setStyleSheet("QFrame{border: 1px solid #727272;background-color: #727272;}");
    } else {
        taskbar_label->setStyleSheet(QString("QLabel {"
                                     "border-width: 0px; "
                                     "border-radius: %1; "
                                     "background-color:rgb(225,225,225,209);}").arg(borderRadius));

        str = QString("QLabel {"
              "border-width: 0px; "
              "border-radius: %1; "
              "background-color:rgb(225,225,225,180);"
              "font-size: 15px;"
              "color: black}").arg(borderRadius);
    }
    size_label->setStyleSheet(str);
    size_label->setText(tr("%1 * %2")
                        .arg(qRound(m_selection->geometry().intersected(rect()).width()*this->windowHandle()->devicePixelRatio()))
                        .arg(qRound(m_selection->geometry().intersected(rect()).height()*this->windowHandle()->devicePixelRatio())));
    if (size_label->x() + size_label->width() + 10 >= vectorButtons.first()->pos().x()
        && size_label->y() > vectorButtons.first()->pos().y() && m_buttonHandler->isVisible())
        size_label->move(m_selection->geometry().intersected(rect()).x(),
                         m_selection->geometry().intersected(rect()).y()+2);
    else if (m_selection->geometry().intersected(rect()).y()-24 < 0)
        size_label->move(m_selection->geometry().intersected(rect()).x(),
                         m_selection->geometry().intersected(rect()).y()+2);
    else
        size_label->move(m_selection->geometry().intersected(rect()).x(),
                         m_selection->geometry().intersected(rect()).y()-37);

    size_label->show();
#endif
}

void CaptureWidget::paintEvent(QPaintEvent *)
{
#ifndef UNUI
    QPainter painter(this);
    m_context.style_name = m_context.style_settings->get("style-name").toString();

    if (needToUpdate) {
        return;
    }

    if (!isScrolling())
    {
        painter.drawPixmap(0, 0, m_context.screenshot);
    }

    // 添加工具栏模糊效果
    if (m_buttonHandler->isVisible()) {
        // 创建工具栏区域的模糊效果
        QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(8); // 设置模糊半径

        // 获取工具栏区域的截图
        QPixmap toolbarPixmap = m_context.screenshot.copy(taskbar_label->geometry());
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(toolbarPixmap);
        item->setGraphicsEffect(blur);

        // 创建场景并渲染
        QGraphicsScene scene;
        scene.addItem(item);
        scene.render(&painter, taskbar_label->geometry(), QRectF());

        // 增强模糊效果
        blur->setBlurRadius(15);
        scene.render(&painter, taskbar_label->geometry(), QRectF());
        scene.render(&painter, taskbar_label->geometry(), QRectF());

        delete blur;
        delete item;
    }
    if (m_activeTool && m_mouseIsClicked) {
        painter.save();
        m_activeTool->process(painter, m_context.screenshot);
        painter.restore();
    } /*else if (m_activeButton && m_activeButton->tool()->showMousePreview()
               && m_previewEnabled) {
        painter.save();
        m_activeButton->tool()->paintMousePreview(painter, m_context);
        painter.restore();
    }*/ else if (!m_buttonHandler->isVisible()) {
        if (m_context.mousePos.x() >= 0 && m_context.mousePos.x()-144 <= qApp->desktop()->width()
            && m_context.mousePos.y() >= 0
            && m_context.mousePos.y()-144 <= qApp->desktop()->height()) {
            painter.setOpacity(0.5);
            updateMagnifier(m_context);
            if (!crosspixmap) {
                qDebug() << "not follow mouse";
            } else if(!isfirstPress){
                painter.drawPixmap(magnifier_x, magnifier_y, crosspixmap);
                painter.drawText(magnifier_x+20, magnifier_y+120, tr("%1 , %2")
                                 .arg(m_context.mousePos.x()).arg(m_context.mousePos.y()));
            }
        }
    }

    QColor overlayColor(0, 0, 0, m_opacity);
    painter.setBrush(overlayColor);
    QRect r;
    if (m_selection->isVisible()) {
        r = m_selection->geometry().normalized().adjusted(0, 0, -1, -1);
    }
    QRegion grey(rect());
    grey = grey.subtracted(r);
    painter.setClipRegion(grey);
    painter.drawRect(-1, -1, rect().width() + 1, rect().height() + 1);
    painter.setClipRect(rect());

    QFont ft;
    ft.setPointSize(10);
    painter.setRenderHint(QPainter::Antialiasing);
    if (m_selection->isVisible()) {
        // QPixmap p = PixmapToRound(m_context.origScreenshot.copy(taskbar_label->geometry()), 6);
        // taskbar_label->setPixmap(p);
        painter.setOpacity(0.6);
        if (m_buttonHandler->isVisible()) {
            if ((m_context.style_name.compare("ukui-dark") == 0)
                || (m_context.style_name.compare("ukui-black") == 0)) {
                painter.setBrush(QColor(0, 0, 0));
                painter.setPen(QColor(0, 0, 0));
            } else {
                painter.setBrush(QColor(200, 200, 200));
                painter.setPen(QColor(200, 200, 200));
            }
            // painter.drawRoundedRect(rr, 6, 6, Qt::AbsoluteSize);
            painter.drawRoundedRect(vectorButtons.last()->pos().x(),
                                    vectorButtons.last()->pos().y(),
                                    GlobalValues::buttonBaseSize()-9,
                                    GlobalValues::buttonBaseSize()-9,
                                    (GlobalValues::buttonBaseSize()-9)/2,
                                    (GlobalValues::buttonBaseSize()-9)/2);
            // paint handlers
        }
        painter.setPen(m_uiColor);
        size_label->show();
        // auto pixelRatio = m_context.origScreenshot.devicePixelRatio();
        if ((vectorButtons.first()->pos().x() > 0 && m_buttonHandler->isVisible())) {
            painter.setPen(QColor(255, 255, 255));
            painter.setOpacity(1);
            painter.drawLine(r.topLeft(), r.bottomLeft());
            painter.drawLine(r.topLeft(), r.topRight());
            painter.drawLine(r.bottomRight(), r.topRight());
            painter.drawLine(r.bottomRight(), r.bottomLeft());
            if (!isScrolling())
            {
                for (auto r: m_selection->handlerAreas()) {
                    // painter.drawRoundRect(r, 80, 80)
                    painter.setBrush(QColor(160, 160, 160));
                    painter.setPen(QColor(255, 255, 255));
                    painter.drawRoundedRect(r, r.width()/2, r.height()/2, Qt::AbsoluteSize);
                    painter.drawPixmap(r, QPixmap(QStringLiteral(":/img/material/control_point.png")));
                }
            }
        }
    }
    updateChildWindow();
#endif
}

QPixmap CaptureWidget::applyEffectToImage(QPixmap src, QGraphicsEffect *effect, int extent)
{
#ifndef UNUI
    if (src.isNull()) return QPixmap();  // No need to do anything else!
    if (!effect) return src;            // No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(src);
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QPixmap res(src.size()+QSize(extent*2, extent*2));
    // QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(),
                 QRectF(-extent, -extent, src.width()+extent*2, src.height()+extent*2));
    return res;
#else
    return QPixmap();
#endif
}

QPixmap CaptureWidget::PixmapToRound(const QPixmap &img, int radius)
{
#ifndef UNUI
    if (img.isNull()) {
        return QPixmap();
    }

    QSize size(img.size());
    QBitmap mask(size);
    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if ((m_context.style_name.compare("ukui-dark") == 0)
        || (m_context.style_name.compare("ukui-black") == 0)) {
        painter.fillRect(mask.rect(), Qt::black);
    } else {
        painter.fillRect(mask.rect(), Qt::white);
    }
    painter.setBrush(QColor(0, 0, 0));
    painter.drawRoundedRect(mask.rect(), radius, radius, Qt::AbsoluteSize);

    QPixmap image = img;    // .scaled(size);
    image.setMask(mask);
    return image;
#else
    return QPixmap();
#endif
}

void CaptureWidget::updateChildWindow()
{
#ifndef UNUI
    if (font_color->isVisible()) {
        if (m_buttonHandler->FontSize_Color_Chose_Window_Y < vectorButtons.first()->pos().y()) {
            font_color->hide();
            font_color2->show();
        }
    }
    if (font_color2->isVisible()) {
        if (m_buttonHandler->FontSize_Color_Chose_Window_Y > vectorButtons.first()->pos().y()) {
            font_color2->hide();
            font_color->show();
        }
    }
    if (save_location->isVisible()) {
        if (m_buttonHandler->Save_Location_Window_Pos.y() < vectorButtons.first()->pos().y()) {
            save_location->hide();
            save_location2->show();
        }
    }
    if (save_location2->isVisible()) {
        if (m_buttonHandler->Save_Location_Window_Pos.y() > vectorButtons.first()->pos().y()) {
            save_location2->hide();
            save_location->show();
        }
    }
    if (font_options->isVisible()) {
        if (m_buttonHandler->Font_Options_Window_Pos.y() < vectorButtons.first()->pos().y()) {
            font_options->hide();
            font_options2->show();
        }
    }
    if (font_options2->isVisible()) {
        if (m_buttonHandler->Font_Options_Window_Pos.y() > vectorButtons.first()->pos().y()) {
            font_options2->hide();
            font_options->show();
        }
    }
    font_color->move(
        vectorButtons.first()->pos().x()+length, m_buttonHandler->FontSize_Color_Chose_Window_Y);
    font_color2->move(
        vectorButtons.first()->pos().x()+length, m_buttonHandler->FontSize_Color_Chose_Window_Y);
    save_location->move(m_buttonHandler->Font_Options_Window_Pos);
    save_location2->move(m_buttonHandler->Font_Options_Window_Pos);
    font_options->move(m_buttonHandler->Font_Options_Window_Pos);
    font_options2->move(m_buttonHandler->Font_Options_Window_Pos);
#endif
}

void CaptureWidget::updateMagnifier(CaptureContext m_context)
{
    int x = m_context.mousePos.x();
    int y = m_context.mousePos.y();
    for (int i = 0; i < areas.size(); i++) {
        if (areas.at(i).contains(m_context.mousePos)) {
            if (y+144 >= areas.at(i).y() +areas.at(i).height()) {
                if (x+144 <= areas.at(i).x() + areas.at(i).width()) {
                    magnifier_x = x+15;
                    magnifier_y = y-129;
                } else {
                    magnifier_x = x-144;
                    magnifier_y = y-144;
                }
            } else {
                if (x+144 >= areas.at(i).x() + areas.at(i).width()) {
                    magnifier_x = x-144;
                    magnifier_y = y+10;
                } else {
                    magnifier_x = x+15;
                    magnifier_y = y+15;
                }
            }
        }
    }
}

void CaptureWidget::mousePressEvent(QMouseEvent *e)
{
#ifndef UNUI
    update();
    if (e->button() == Qt::RightButton) {
        if (!isfirstPress)
            return;
    } else if (e->button() == Qt::LeftButton) {
        m_showInitialMsg = false;
        m_mouseIsClicked = true;
        isPressButton = true;
        m_dragStartPoint = e->pos();
        // 第一次按下 确定框选区域
        if (!isfirstPress) {
            isfirstPress = true;
        }
        // Click using a tool
        else if (m_activeButton) {
            if (m_activeTool) {
                if (m_activeTool->isValid() && m_toolWidget) {
                    pushToolToStack();
                } else {
                    if( m_activeButton->tool()->name() == tr("文本") /*&& !m_activeTool->isValid()*/)
                    {
                        if(!m_activeTool->isValid()) {
                            m_toolWidget->move(e->pos());
                        }
                    }
                    else
                    {
                        m_activeTool->deleteLater();
                        m_activeTool = nullptr;

                        //fixme: 18261
                        if (m_toolWidget) {
                               m_toolWidget->deleteLater();
                               return;
                        }
                    }
                }
            }
            m_activeTool = m_activeButton->tool()->copy(this);
            connect(this, &CaptureWidget::colorChanged,
                    m_activeTool, &CaptureTool::colorChanged);
            connect(this, &CaptureWidget::thicknessChanged,
                    m_activeTool, &CaptureTool::thicknessChanged);
            connect(this, &CaptureWidget::textThicknessChanged,
                    m_activeTool, &CaptureTool::textthicknessChanged);
            connect(m_activeTool, &CaptureTool::requestAction,
                    this, &CaptureWidget::handleButtonSignal);
            connect(this, &CaptureWidget::textchanged,
                    m_activeTool, &CaptureTool::textChanged);
            qDebug() << "m_activeTool is  pressed";
            m_activeTool->drawStart(m_context);

            m_buttonHandler->m_isTooldrawMode = true;
            m_buttonHandler->setScrolltoolState();

            return;
        } else if (m_mouseOverHandle == SelectionWidget::NO_SIDE
                   && m_selection->geometry().contains(e->pos())) {
            isMove = true;
        }
        m_selection->saveGeometry();
        m_selection->setVisible(true);
        isReleaseButton = false;
    }
    updateCursor();
#endif
}

void CaptureWidget::mouseMoveEvent(QMouseEvent *e)
{
#ifndef UNUI
    m_context.mousePos = e->globalPos();
    if (!isfirstMove) {
        isfirstMove = true;
    }
    if (isfirstPress && isPressButton) {
        if (!isDrag) {
            isDrag = true;
        }
    }
    // QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
    if (isfirstPress) {
        if (!isfirstRelease) {
            m_selection->setVisible(true);
            m_selection->setGeometry(
                QRect(m_dragStartPoint, m_context.mousePos).normalized());
            m_selection->setVisible(true);
        } else if (isPressButton && !m_activeButton) {
            // updateCursor();
            if (m_mouseOverHandle == SelectionWidget::NO_SIDE
                && isMove
                ) {
                // Moving the whole selection
                QRect initialRect = m_selection->savedGeometry().normalized();
                QPoint newTopLeft = initialRect.topLeft() + (e->pos() - m_dragStartPoint);
                QRect finalRect(newTopLeft, initialRect.size());
                if (finalRect.left() < rect().left()) {
                    finalRect.setLeft(rect().left());
                } else if (finalRect.right() > rect().right()) {
                    finalRect.setRight(rect().right());
                }
                if (finalRect.top() < rect().top()) {
                    finalRect.setTop(rect().top());
                } else if (finalRect.bottom() > rect().bottom()) {
                    finalRect.setBottom(rect().bottom());
                }
                m_selection->setGeometry(finalRect.normalized().intersected(rect()));
                m_context.selection = m_selection->geometry();
                m_buttonHandler->updatePosition(m_selection->geometry());
                taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                                    vectorButtons.first()->pos().y() - 3);
                m_buttonHandler->hide();
                taskbar_label->hide();
                label1++;
                if(label1 == 1){
                    update();
                }
            } else {
                if (isScrolling()) {
                    // 长截图模式下不允许改变选择区域
                    return;
                }

                // Dragging a handle
                QRect r = m_selection->savedGeometry();
                QPoint offset = e->pos() - m_dragStartPoint;
                bool symmetryMod = qApp->keyboardModifiers() & Qt::ShiftModifier;

                using sw = SelectionWidget;
                if (m_mouseOverHandle == sw::TOPLEFT_SIDE
                    || m_mouseOverHandle == sw::TOP_SIDE
                    || m_mouseOverHandle == sw::TOPRIGHT_SIDE) {             // dragging one of the top handles
                    r.setTop(r.top() + offset.y());
                    if (symmetryMod) {
                        r.setBottom(r.bottom() - offset.y());
                    }
                }
                if (m_mouseOverHandle == sw::TOPLEFT_SIDE
                    || m_mouseOverHandle == sw::LEFT_SIDE
                    || m_mouseOverHandle == sw::BOTTONLEFT_SIDE) {             // dragging one of the left handles
                    r.setLeft(r.left() + offset.x());
                    if (symmetryMod) {
                        r.setRight(r.right() - offset.x());
                    }
                }
                if (m_mouseOverHandle == sw::BOTTONLEFT_SIDE
                    || m_mouseOverHandle == sw::BOTTON_SIDE
                    || m_mouseOverHandle == sw::BOTTONRIGHT_SIDE) {             // dragging one of the bottom handles
                    r.setBottom(r.bottom() + offset.y());
                    if (symmetryMod) {
                        r.setTop(r.top() - offset.y());
                    }
                }
                if (m_mouseOverHandle == sw::TOPRIGHT_SIDE
                    || m_mouseOverHandle == sw::RIGHT_SIDE
                    || m_mouseOverHandle == sw::BOTTONRIGHT_SIDE) {             // dragging one of the right handles
                    r.setRight(r.right() + offset.x());
                    if (symmetryMod) {
                        r.setLeft(r.left() - offset.x());
                    }
                }
                m_selection->setGeometry(r.intersected(rect()).normalized());
                m_context.selection = m_selection->geometry();
                m_buttonHandler->hide();
                taskbar_label->hide();
                label2++;
                if(label2 == 1){
                    update();
                }
            }
        } else if (isPressButton && m_activeTool) {
            // drawing with a tool
            if (m_adjustmentButtonPressed) {
                m_activeTool->drawMoveWithAdjustment(e->pos());
            } else {
                m_activeTool->drawMove(e->pos());
            }
            update();
            // Hides the buttons under the mouse. If the mouse leaves, it shows them.
            if (m_buttonHandler->buttonsAreInside()) {
                const bool containsMouse = m_buttonHandler->contains(m_context.mousePos);
                // m_buttonHandler->show();
// if (containsMouse) {
// m_buttonHandler->hide();
// } else {
// m_buttonHandler->show();
// }
                m_buttonHandler->show();
                taskbar_label->show();
            }
        } /*else if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
            qDebug()<<"进入这里";
            update();
        }*/ else {
            if (!m_selection->isVisible()) {
                // isSure = false;
                m_buttonHandler->hide();
                taskbar_label->hide();
                return;
            }
            m_mouseOverHandle = m_selection->getMouseSide(m_context.mousePos);
            updateCursor();
        }
    } else {
        if (!isfirstPress) {
            updateCrosspixmap(e->globalPos());
            QRect currentHoverRect;
            for(QRect const rect : rects){
                if(rect.contains(e->pos())){
                    currentHoverRect = rect;
                }
            }
            if(currentHoverRect != m_lastHoverRect){
                qDebug()<<"进入此处"<<currentHoverRect<<m_lastHoverRect;
                m_lastHoverRect = currentHoverRect;
                if(!currentHoverRect.isEmpty()){
                    m_selection->setGeometry(currentHoverRect);
                    m_selection->setVisible(true);
                }
            }
        }
    }
#endif
}

void CaptureWidget::updateCrosspixmap(QPoint e)
{
#ifndef UNUI
    if (isScrolling())
        return;

    w = 26 * this->windowHandle()->devicePixelRatio();
    h = 26 * this->windowHandle()->devicePixelRatio();
    // mypixmap = mypixmap.grabWidget(this,e->pos().x()-w/2-1,e->pos().y()-h/2-1,w,h);
    mypixmap = mypixmap.grabWidget(this, e.x()-12, e.y()-12, 26, 26);
    QImage crosstmp = mypixmap.toImage();
    QRgb value = qRgb(0, 0, 255);

    QImage image = crosstmp.scaled(w*5, h*5, Qt::KeepAspectRatio);
    w=w*5;
    h=h*5;

    for (int i = 0; i < w; i++) {
        for (int j = h/2-1; j < h/2; j++) {
            if (i % 6 == 0)
            {
                continue;
            }

            image.setPixel(i, j, value);
        }
    }
    for (int i = w/2-1; i < w/2; i++) {
        for (int j = 0; j < h; j++) {
            if (j % 6 == 0)
            {
                continue;
            }

            image.setPixel(i, j, value);
        }
    }
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < h; j++) {
            image.setPixel(i, j, value);
            image.setPixel(j, i, value);
        }
    }
    for (int i = w-5; i < w; i++) {
        for (int j = 0; j < h; j++) {
            image.setPixel(i, j, value);
            image.setPixel(j, i, value);
        }
    }

    crosspixmap = QPixmap::fromImage(image);
    update();
#endif
}

void CaptureWidget::mouseReleaseEvent(QMouseEvent *e)
{
#ifndef UNUI
    if (e->button() == Qt::RightButton) {
        m_colorPicker->hide();
        m_rightClick = false;
        // when we end the drawing we have to register the last  point and
        // add the temp modification to the list of modifications
    } else if (Qt::LeftButton == e->button()) {
        if (!isfirstRelease) {
            isfirstRelease = true;
            if (!isDrag) {
                for (QRect const rect : rects) {
                    if (rect.contains(e->pos())) {
                        m_selection->setGeometry(rect);
                    }
                }
            }
            m_buttonHandler->show();
            taskbar_label->show();
        } else {
            if (m_mouseIsClicked && m_activeTool) {
                // font_color->hide();
                m_activeTool->drawEnd(m_context.mousePos);
                if (m_activeTool->isValid()) {
                    pushToolToStack();
                } else if (!m_toolWidget) {
                    m_activeTool->deleteLater();
                    m_activeTool = nullptr;
                }
            } else if (!m_buttonHandler->isVisible() && m_selection->isVisible()) {
                // Don't go outside
                QRect newGeometry = m_selection->geometry().intersected(rect());
                // normalize
                if (newGeometry.width() <= 0) {
                    int left = newGeometry.left();
                    newGeometry.setLeft(newGeometry.right());
                    newGeometry.setRight(left);
                }
                if (newGeometry.height() <= 0) {
                    int top = newGeometry.top();
                    newGeometry.setTop(newGeometry.bottom());
                    newGeometry.setBottom(top);
                }
                m_selection->setGeometry(newGeometry);
        #ifdef ENABLE_RECORD
                recorder->ssr->OnUpdateVideoAreaFields_(newGeometry);// bybobbi
        #endif
                m_context.selection = extendedRect(&newGeometry);
                updateSizeIndicator();
                m_buttonHandler->updatePosition(newGeometry);
                taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                                    vectorButtons.first()->pos().y() - 3);
                taskbar_label->show();
                m_buttonHandler->show();
            }
        }
        m_context.selection = m_selection->geometry();
        updateSizeIndicator();
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        m_mouseIsClicked = false;
        m_newSelection = false;
        m_grabbing = false;
        isPressButton = false;
        isReleaseButton = true;
        isMove = false;
    }
    updateCursor();
    update();
    size_label_option();
    label1 = 0;
    label2 = 0;
#endif
}

void CaptureWidget::keyPressEvent(QKeyEvent *e)
{
#ifndef UNUI
    if (!m_selection->isVisible()) {
        return;
    } else if (e->key() == Qt::Key_Up
               && m_selection->geometry().top() > rect().top()) {
        m_selection->move(QPoint(m_selection->x(), m_selection->y() -1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        update();
    } else if (e->key() == Qt::Key_Down
               && m_selection->geometry().bottom() < rect().bottom()) {
        m_selection->move(QPoint(m_selection->x(), m_selection->y() +1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        update();
    } else if (e->key() == Qt::Key_Left
               && m_selection->geometry().left() > rect().left()) {
        m_selection->move(QPoint(m_selection->x() -1, m_selection->y()));
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        update();
    } else if (e->key() == Qt::Key_Right
               && m_selection->geometry().right() < rect().right()) {
        m_selection->move(QPoint(m_selection->x() +1, m_selection->y()));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        update();
    } else if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = true;
    }
#endif
}

void CaptureWidget::keyReleaseEvent(QKeyEvent *e)
{
#ifndef UNUI
    if (e->key() == Qt::Key_Control) {
        m_adjustmentButtonPressed = false;
    }
#endif
}

void CaptureWidget::wheelEvent(QWheelEvent *e)
{
#ifndef UNUI
    QPoint topLeft = qApp->desktop()->screenGeometry(
        qApp->desktop()->screenNumber(QCursor::pos())).topLeft();
    if (font_color->isVisible() || font_color2->isVisible()) {
        m_context.thickness += e->delta() / 120;
        m_context.thickness = qBound(1, m_context.thickness, 20);
        m_notifierBox->showMessage(QString::number(m_context.thickness));
        emit thicknessChanged(m_context.thickness);
        fontsize_color_chose_default();
        fontsize_color_chose2_default();
    } else if (font_options->isVisible() || font_options2->isVisible()) {
        m_context.text_thickness += e->delta() / 120;
        m_context.text_thickness = qBound(6, m_context.text_thickness, 99);
        m_notifierBox->showMessage(QString::number(m_context.text_thickness));
        emit thicknessChanged(m_context.text_thickness);
        emit textThicknessChanged(m_context.text_thickness);
        font_options_defult();
        font_options2_defult();
    }
    int offset = m_notifierBox->width() / 4;
    if (m_activeButton && m_activeButton->tool()->showMousePreview()) {
        update();
    }
    m_notifierBox->move(mapFromGlobal(topLeft) + QPoint(offset, offset));
#endif
}

void CaptureWidget::resizeEvent(QResizeEvent *e)
{
#ifndef UNUI
    QWidget::resizeEvent(e);
    m_context.widgetDimensions = rect();
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
    m_panel->setFixedHeight(height());
    if (!m_context.fullscreen) {
        m_buttonHandler->updateScreenRegions(rect());
    }
#endif
}

void CaptureWidget::moveEvent(QMoveEvent *e)
{
#ifndef UNUI
    QWidget::moveEvent(e);
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
#endif
}

void CaptureWidget::initContext(const QString &savePath, bool fullscreen)
{
    m_context.widgetDimensions = rect();
    m_context.color = m_config.drawColorValue();
    m_context.savePath = savePath;
    m_context.widgetOffset = mapToGlobal(QPoint(0, 0));
    m_context.mousePos = mapFromGlobal(QCursor::pos());
    m_context.thickness = m_config.drawThicknessValue();
    m_context.text_thickness = m_config.drawTextThicknessValue();
    m_context.fullscreen = fullscreen;
    m_context.font_type = QFont("方正黑体");
    m_context.bold = false;
    m_context.italic = false;
    m_context.underline = false;
    m_context.deleteline = false;
    m_context.style_settings = new QGSettings("org.ukui.style");
    m_context.style_name = m_context.style_settings->get("style-name").toString();
    m_context.isScrollEnabled = false;
    m_context.isScrollSuccess = true;
}

void CaptureWidget::initPanel()
{
    m_panel = new UtilityPanel(this);
    makeChild(m_panel);
    QRect panelRect = rect();
    if (m_context.fullscreen) {
        panelRect = QGuiApplication::primaryScreen()->geometry();
    }
    panelRect.moveTo(mapFromGlobal(panelRect.topLeft()));
    panelRect.setWidth(m_colorPicker->width() * 3);
    m_panel->setGeometry(panelRect);

    SidePanelWidget *sidePanel
        = new SidePanelWidget(&m_context.screenshot);
    connect(sidePanel, &SidePanelWidget::colorChanged,
            this, &CaptureWidget::setDrawColor);
    connect(sidePanel, &SidePanelWidget::thicknessChanged,
            this, &CaptureWidget::setDrawThickness);
    connect(this, &CaptureWidget::colorChanged,
            sidePanel, &SidePanelWidget::updateColor);
    connect(this, &CaptureWidget::thicknessChanged,
            sidePanel, &SidePanelWidget::updateThickness);
    connect(sidePanel, &SidePanelWidget::togglePanel,
            m_panel, &UtilityPanel::toggle);

    sidePanel->colorChanged(m_context.color);
    sidePanel->thicknessChanged(m_context.thickness);
    m_panel->pushWidget(sidePanel);
    m_panel->pushWidget(new QUndoView(&m_undoStack, this));
}

void CaptureWidget::initSelection()
{
    // m_selection = new SelectionWidget(m_uiColor, this);
    // Color of the outer border of the capture
    m_selection = new SelectionWidget(QColor(Qt::white), this);
#ifndef UNUI
    connect(m_selection, &SelectionWidget::animationEnded, this, [this](){
        this->m_buttonHandler->updatePosition(this->m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
    });
#endif
    m_selection->setVisible(false);
    m_selection->setGeometry(QRect());
}

void CaptureWidget::setState(CaptureButton *b)
{
#ifndef UNUI
    if (!b) {
        return;
    }
#ifdef ENABLE_RECORD
    b->tool()->pressCalled();
#endif
    m_context.style_name = m_context.style_settings->get("style-name").toString();
    if (m_toolWidget) {
        m_toolWidget->deleteLater();
        if (m_activeTool->isValid()) {
            pushToolToStack();
        }
    }
    if (m_activeButton != b) {
        processTool(b->tool());
    }
    // Only close activated from button
    if (b->tool()->closeOnButtonPressed()) {
        b->setIcon(b->tool()->icon(m_contrastUiColor, false, m_context));
        hide_ChildWindow();
        hide_window();
        close();
    }
    if (b->tool()->isSelectable()) {
        if (m_activeButton != b) {
            QWidget *confW = b->tool()->configurationWidget();
            m_panel->addToolWidget(confW);
            if (m_activeButton) {
                m_activeButton->setColor(m_uiColor);
#ifdef ENABLE_RECORD
                if (!m_isolatedButtons[m_activeButton->m_buttonType]) {
#endif
                m_activeButton->setIcon(m_activeButton->tool()->icon(m_uiColor, false, m_context));
#ifdef ENABLE_RECORD
                m_activeButton->tool()->setIsPressed(false);
            }
#endif
            }
            show_childwindow(b);
            m_activeButton = b;
#ifdef ENABLE_RECORD
            b->setIcon(b->tool()->icon(m_contrastUiColor, b->tool()->getIsPressed(), m_context));
#else
            b->setIcon(b->tool()->icon(m_contrastUiColor, true, m_context));
#endif
            m_activeButton->setColor(m_contrastUiColor);
        } else if (m_activeButton) {
            m_panel->clearToolWidget();
            m_activeButton->setColor(m_uiColor);
#ifdef ENABLE_RECORD
            b->setIcon(b->tool()->icon(m_contrastUiColor, b->tool()->getIsPressed(), m_context));
#else
            b->setIcon(b->tool()->icon(m_uiColor, false, m_context));
#endif
            hide_ChildWindow();
            m_activeButton = nullptr;
        }
#ifdef ENABLE_RECORD
        record_do_sth(b);
#endif
        update(); // clear mouse preview
    }
#endif
}

void CaptureWidget::processTool(CaptureTool *t)
{
#ifndef UNUI
    auto backup = m_activeTool;
    // The tool is active during the pressed().
    m_activeTool = t;
    t->pressed(m_context);
    m_activeTool = backup;
#endif
}

void CaptureWidget::handleButtonSignal(CaptureTool::Request r)
{
#ifndef UNUI
    switch (r) {
    case CaptureTool::REQ_CLEAR_MODIFICATIONS:
        m_undoStack.setIndex(0);
        update();
        break;
    case CaptureTool::REQ_CLOSE_GUI:
        hide_window();
        emit captureExit(m_id);
        close();
        break;
    case CaptureTool::REQ_HIDE_GUI:
        hide();
        break;
    case CaptureTool::REQ_HIDE_SELECTION:
        m_newSelection = true;
        m_selection->setVisible(false);
        updateCursor();
        break;
    case CaptureTool::REQ_SELECT_ALL:
        m_selection->setGeometryAnimated(rect());
        break;
    case CaptureTool::REQ_UNDO_MODIFICATION:
        m_undoStack.undo();
        break;
    case CaptureTool::REQ_REDO_MODIFICATION:
        m_undoStack.redo();
        break;
    case CaptureTool::REQ_REDRAW:
        update();
        break;
    case CaptureTool::REQ_TOGGLE_SIDEBAR:
        m_panel->toggle();
        break;
    case CaptureTool::REQ_SHOW_COLOR_PICKER:
        // TODO
        break;
    case CaptureTool::REQ_MOVE_MODE:
        setState(m_activeButton);      // Disable the actual button
        break;
    case CaptureTool::REQ_COPY:
        remergeScreenshot();
        if (!m_context.isScrollEnabled || m_context.isScrollSuccess)
        {
            emit captureCopy(m_id);
            m_captureDone = true;
        }
        break;
    case CaptureTool::REQ_CAPTURE_DONE_OK:
        m_captureDone = true;
        break;
    case CaptureTool::REQ_SAVE:
        remergeScreenshot();
        break;
    case CaptureTool::REQ_CUT:
        // m_captureDone =false;
        // update();
#ifdef ENABLE_RECORD
        m_buttonHandler->hide();               // bybobbi
        m_buttonHandler->clearButtons();
        updateButtons(false);
        {
            QRect newGeometry = m_selection->geometry().intersected(rect());
            qDebug() << "record newGeometry is " << newGeometry.x() << ", " << newGeometry.y();
            // normalize
            if (newGeometry.width() <= 0) {
                int left = newGeometry.left();
                newGeometry.setLeft(newGeometry.right());
                newGeometry.setRight(left);
            }
            if (newGeometry.height() <= 0) {
                int top = newGeometry.top();
                newGeometry.setTop(newGeometry.bottom());
                newGeometry.setBottom(top);
            }
            m_selection->setGeometry(newGeometry);
            m_context.selection = extendedRect(&newGeometry);
            updateSizeIndicator();
            m_buttonHandler->updatePosition(newGeometry);
        }
        m_buttonHandler->show();
#endif
        break;
    case CaptureTool::REQ_LUPING:
        // m_captureDone = true;
        // hide_window();
#ifdef ENABLE_RECORD
        m_buttonHandler->hide();               // bybobbi
        m_buttonHandler->clearButtons();
        updateButtons(true);
        {
            QRect newGeometry = m_selection->geometry().intersected(rect());
            qDebug() << "bybobbi: record newGeometry in LUPING is " << newGeometry.x() << ", " <<
                newGeometry.y();
            // normalize
            if (newGeometry.width() <= 0) {
                int left = newGeometry.left();
                newGeometry.setLeft(newGeometry.right());
                newGeometry.setRight(left);
            }
            if (newGeometry.height() <= 0) {
                int top = newGeometry.top();
                newGeometry.setTop(newGeometry.bottom());
                newGeometry.setBottom(top);
            }
            m_selection->setGeometry(newGeometry);
            m_context.selection = extendedRect(&newGeometry);
            updateSizeIndicator();
            m_buttonHandler->updatePosition(newGeometry);
        }
        m_buttonHandler->show();
#endif
        break;
    case CaptureTool::REQ_OPTIONS:
        // update();
        break;
    case CaptureTool::REQ_ADD_CHILD_WIDGET:
        if (!m_activeTool) {
            break;
        }
        if (m_toolWidget) {
            m_toolWidget->deleteLater();
        }
        // 标位于框选区域内  子窗口才有效
        m_toolWidget = m_activeTool->widget();
        if (m_toolWidget) {
            if (m_context.selection.contains(QPoint(m_context.mousePos.x()+m_context.text_thickness
                                                    *3,
                                                    m_context.mousePos.y()
                                                    + m_context.text_thickness*3))) {
                makeChild(m_toolWidget);
                m_toolWidget->move(m_context.mousePos);
                m_toolWidget->show();
                m_toolWidget->setFocus();
            } else
                m_toolWidget->hide();
        }
        break;
    case CaptureTool::REQ_ADD_CHILD_WINDOW:
        if (!m_activeTool) {
            break;
        } else {
            QWidget *w = m_activeTool->widget();
            connect(this, &CaptureWidget::destroyed, this, &CaptureWidget::hide_window);
            connect(this, &CaptureWidget::destroyed, w, &QWidget::deleteLater);
            w->show();
        }
        break;
    case CaptureTool::REQ_ADD_EXTERNAL_WIDGETS:
        if (!m_activeTool) {
            break;
        } else {
            QWidget *w = m_activeTool->widget();
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();
        }
        break;
#ifdef ENABLE_RECORD
    case CaptureTool::REQ_CURSOR_RECORD:
        break;
    case CaptureTool::REQ_AUDIO_RECORD:
        break;
    case CaptureTool::REQ_FOLLOW_MOUSE_RECORD:
        break;
    case CaptureTool::REQ_OPTION_RECORD:
        recorder->OnRecordOptionClicked();
        break;
    case CaptureTool::REQ_START_RECORD:
        recorder->OnRecordStartClicked();
        break;
#endif
    default:
        break;
    }
#endif
}

void CaptureWidget::setDrawColor(const QColor &c)
{
#ifndef UNUI
    m_context.color = c;
    ConfigHandler().setDrawColor(m_context.color);
    emit colorChanged(c);
#endif
}

void CaptureWidget::setDrawThickness(const int &t)
{
#ifndef UNUI
    m_context.thickness = qBound(1, t, 99);
    ConfigHandler().setdrawThickness(m_context.thickness);
    emit thicknessChanged(m_context.thickness);
#endif
}

void CaptureWidget::setTextDrawThickness(const int &t)
{
#ifndef UNUI
    m_context.text_thickness = qBound(6, t, 99);
    ConfigHandler().setdrawThickness(m_context.text_thickness);
    emit textThicknessChanged(m_context.text_thickness);
#endif
}

void CaptureWidget::leftResize()
{
#ifndef UNUI
    if (m_selection->isVisible()
        && m_selection->geometry().right() > m_selection->geometry().left()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, -1, 0));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        updateSizeIndicator();
        update();
    }
#endif
}

void CaptureWidget::rightResize()
{
#ifndef UNUI
    if (m_selection->isVisible() && m_selection->geometry().right() < rect().right()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, 1, 0));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        updateSizeIndicator();
        update();
    }
#endif
}

void CaptureWidget::upResize()
{
#ifndef UNUI
    if (m_selection->isVisible()
        && m_selection->geometry().bottom() > m_selection->geometry().top()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, 0, -1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        updateSizeIndicator();
        update();
    }
#endif
}

void CaptureWidget::downResize()
{
#ifndef UNUI
    if (m_selection->isVisible() && m_selection->geometry().bottom() < rect().bottom()) {
        m_selection->setGeometry(m_selection->geometry() + QMargins(0, 0, 0, 1));
        QRect newGeometry = m_selection->geometry().intersected(rect());
        m_context.selection = extendedRect(&newGeometry);
        m_buttonHandler->updatePosition(m_selection->geometry());
        taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                            vectorButtons.first()->pos().y() - 3);
        updateSizeIndicator();
        update();
    }
#endif
}

void CaptureWidget::initShortcuts()
{
#ifndef UNUI
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(hide_window()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(saveScreenshot()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this, SLOT(copyScreenshot()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), this, SLOT(undo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z), this, SLOT(redo()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this, SLOT(rightResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this, SLOT(leftResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up), this, SLOT(upResize()));
    new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down), this, SLOT(downResize()));
    new QShortcut(Qt::Key_Escape, this, SLOT(deleteToolwidgetOrClose()));
    new QShortcut(Qt::Key_Return, this, SLOT(copyScreenshot()));
    new QShortcut(Qt::Key_Enter, this, SLOT(copyScreenshot()));
#endif
}

void CaptureWidget::updateSizeIndicator()
{
#ifndef UNUI
    if (m_sizeIndButton) {
        const QRect &selection = extendedSelection();
    }
#endif
}

void CaptureWidget::updateCursor()
{
#ifndef UNUI
    if (!isfirstPress) {
        setCursor(Qt::CrossCursor);
    } else if (!m_activeButton) {
        using sw = SelectionWidget;
        if (m_mouseOverHandle != sw::NO_SIDE) {
            // cursor on the handlers
            switch (m_mouseOverHandle) {
            case sw::TOPLEFT_SIDE:
            case sw::BOTTONRIGHT_SIDE:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case sw::TOPRIGHT_SIDE:
            case sw::BOTTONLEFT_SIDE:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case sw::LEFT_SIDE:
            case sw::RIGHT_SIDE:
                setCursor(Qt::SizeHorCursor);
                break;
            case sw::TOP_SIDE:
            case sw::BOTTON_SIDE:
                setCursor(Qt::SizeVerCursor);
                break;
            default:
                break;
            }
        } else if (m_selection->isVisible()
                   && m_selection->geometry().contains(m_context.mousePos) && isPressButton) {
            setCursor(Qt::ClosedHandCursor);
            //鼠标首次按压进行选框
            if(isfirstPress && !isfirstRelease){
                setCursor(Qt::SizeFDiagCursor);
            }
        } else if (m_selection->isVisible() && isReleaseButton && m_selection->geometry().contains(m_context.mousePos)) {
            setCursor(Qt::OpenHandCursor);
            //鼠标首次释放选框完成
            m_mouseOverHandle = m_selection->getMouseSide(m_context.mousePos);
            if( m_mouseOverHandle == sw::BOTTONRIGHT_SIDE){
                setCursor(Qt::SizeFDiagCursor);
            }
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        if (m_selection->isVisible() && m_selection->geometry().contains(m_context.mousePos)) {
            if (m_activeButton->m_buttonType == CaptureButton::TYPE_TEXT)
            {
                setCursor(Qt::IBeamCursor);
            }
            else
            {
                setCursor(Qt::CrossCursor);
            }
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
#endif
}

void CaptureWidget::pushToolToStack()
{
#ifndef UNUI
    auto mod = new ModificationCommand(
        &m_context.screenshot, m_activeTool);
    disconnect(this, &CaptureWidget::colorChanged,
               m_activeTool, &CaptureTool::colorChanged);
    disconnect(this, &CaptureWidget::thicknessChanged,
               m_activeTool, &CaptureTool::thicknessChanged);
    if (m_panel->toolWidget()) {
        disconnect(m_panel->toolWidget(), nullptr, m_activeTool, nullptr);
    }
    m_undoStack.push(mod);
    m_activeTool = nullptr;
#endif
}

void CaptureWidget::makeChild(QWidget *w)
{
    w->setParent(this);
    w->installEventFilter(m_eventFilter);
}

void CaptureWidget::togglePanel()
{
    m_panel->toggle();
}

void CaptureWidget::childEnter()
{
#ifndef UNUI
    m_previewEnabled = false;
    update();
#endif
}

void CaptureWidget::childLeave()
{
#ifndef UNUI
    m_previewEnabled = true;
    update();
#endif
}

void CaptureWidget::copyScreenshot()
{
#ifndef UNUI
    m_captureDone = true;
    ScreenshotSaver().saveToClipboard(pixmap());
    hide_window();
    close();
#endif
}

void CaptureWidget::saveScreenshot()
{
#ifndef UNUI
    m_captureDone = true;
    hide();
    if (m_context.savePath.isEmpty()) {
        ScreenshotSaver().saveToFilesystemGUI(pixmap());
    } else {
        ScreenshotSaver().saveToFilesystem(pixmap(), m_context.savePath);
    }
    hide_window();
    close();
#endif
}

void CaptureWidget::undo()
{
#ifndef UNUI
    m_undoStack.undo();
#endif
}

void CaptureWidget::redo()
{
#ifndef UNUI
    m_undoStack.redo();
#endif
}

QRect CaptureWidget::extendedSelection() const
{
    if (!m_selection->isVisible())
        return QRect();
    QRect r = m_selection->geometry();
    return extendedRect(&r);
}

QRect CaptureWidget::extendedRect(QRect *r) const
{
    auto devicePixelRatio = m_context.screenshot.devicePixelRatio();
    return QRect(r->left(),
                 r->top(),
                 r->width(),
                 r->height());
}

void CaptureWidget::ClickedSaveAsFile()
{
#ifndef UNUI
    hide_window();
    bool ok = false;
    while (!ok) {
        QFileDialog fileDialog(this, Qt::CustomizeWindowHint);
        QString savePath = QFileDialog::getSaveFileName(
            nullptr,
            QString(),
            FileNameHandler().absoluteSavePath() + ".png",
            QLatin1String(
                "Portable Network Graphic file (PNG) (*.png);;BMP file (*.bmp);;JPEG file (*.jpg)"));

        if (savePath.isNull()) {
            break;
        }

        if (!savePath.endsWith(QLatin1String(".png"), Qt::CaseInsensitive)
            && !savePath.endsWith(QLatin1String(".bmp"), Qt::CaseInsensitive)
            && !savePath.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive)) {
            savePath += QLatin1String(".png");
        }

        if (m_context.isScrollEnabled) {
            ok = m_context.scrollPixmap.save(savePath);
        } else {
            ok = pixmap().save(savePath);
        }

        QUrl fullPath = QUrl::fromLocalFile(savePath);

        if (ok) {
            QString pathNoFile = savePath.left(savePath.lastIndexOf(QLatin1String("/")));
            ConfigHandler().setSavePath(pathNoFile);
            QString message = QObject::tr("Capture saved as ") + savePath;
            QString msg = "<a href=\"" + fullPath.toString() + "\">" + message + "</a>";

            SystemNotification().sendMessage(msg, savePath);
        } else {
            QString msg = QObject::tr("Error trying to save as ") + savePath;
            QMessageBox saveErrBox(
                QMessageBox::Warning,
                QObject::tr("Save Error"),
                msg);
            saveErrBox.setWindowIcon(QIcon(
                                         "/usr/share/icons/ukui-icon-theme-default/128x128/apps/kylin-screenshot.png"));
            saveErrBox.exec();
        }
    }
    hide_window();
    close();
#endif
}

void CaptureWidget::ClickedSavedir()
{
#ifndef UNUI
    hide_window();
    QFileDialog fileDialog(nullptr, Qt::CustomizeWindowHint);
    fileDialog.setDirectory(m_context.savePath);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Drives | QDir::NoSymLinks);
    if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
    {
        fileDialog.move(((qApp->desktop()->geometry().width()
                          -fileDialog.width())/2),
                        ((qApp->desktop()->geometry().height()
                          -fileDialog.height())/2));
        kdk::WindowManager::setGeometry(fileDialog.windowHandle(),fileDialog.geometry());
    }
    else
    {
        QScreen *primaryScreen = QGuiApplication::primaryScreen();
        QRect screenGeometry = qApp->primaryScreen()->geometry();
        int dialogWidth = fileDialog.width();
        int dialogHeight = fileDialog.height();
        int x = (screenGeometry.width() - dialogWidth) / 2;
        int y = (screenGeometry.height() - dialogHeight) / 2;
        fileDialog.setGeometry(x,y,dialogWidth,dialogHeight);
    }
    if (fileDialog.exec() == QDialog::Accepted) {
        fileDialog.hide();
        file = fileDialog.selectedFiles()[0];
        fileDialog.hide();
        qDebug()<<"select dir:"<<file;
        save_location->SaveDir->setText(file);
        save_location->SaveDir->setToolTip(file);
        m_context.savePath = file;
        save_location->update();
    }
    fileDialog.hide();
    show_window();
#endif
}

void CaptureWidget::ClickedSavedir2()
{
#ifndef UNUI
    hide_window();
    QFileDialog fileDialog(nullptr, Qt::CustomizeWindowHint);
    fileDialog.setDirectory(m_context.savePath);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Drives | QDir::NoSymLinks);
    if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
    {
        fileDialog.move(((qApp->desktop()->geometry().width()
                          -fileDialog.width())/2),
                        ((qApp->desktop()->geometry().height()
                          -fileDialog.height())/2));
        kdk::WindowManager::setGeometry(fileDialog.windowHandle(),fileDialog.geometry());
    }
    else
    {
        QScreen *primaryScreen = QGuiApplication::primaryScreen();
        QRect screenGeometry = qApp->primaryScreen()->geometry();
        int dialogWidth = fileDialog.width();
        int dialogHeight = fileDialog.height();
        int x = (screenGeometry.width() - dialogWidth) / 2;
        int y = (screenGeometry.height() - dialogHeight) / 2;
        fileDialog.setGeometry(x,y,dialogWidth,dialogHeight);
    }
    if (fileDialog.exec() == QDialog::Accepted) {
        fileDialog.hide();
        file = fileDialog.selectedFiles()[0];
        fileDialog.hide();
        qDebug()<<"select dir:"<<file;
        save_location2->SaveDir->setText(file);
        save_location2->SaveDir->setToolTip(file);
        m_context.savePath = file;
        save_location2->update();
    }
    fileDialog.hide();
    show_window();
#endif
}

void CaptureWidget::font_type_changed(QFont f)
{
#ifndef UNUI
    m_context.font_type = f;
    emit textchanged(m_context);
#endif
}

void CaptureWidget::font_bold_clicked(bool b)
{
#ifndef UNUI
    m_context.bold = b;
    emit textchanged(m_context);
#endif
}

void CaptureWidget::font_delete_clicked(bool b)
{
#ifndef UNUI
    m_context.deleteline = b;
    emit textchanged(m_context);
#endif
}

void CaptureWidget::font_underline_clicked(bool b)
{
#ifndef UNUI
    m_context.underline = b;
    emit textchanged(m_context);
#endif
}

void CaptureWidget::font_italic_clicked(bool b)
{
#ifndef UNUI
    m_context.italic = b;
    emit textchanged(m_context);
#endif
}

void CaptureWidget::ClickedSaveType(int i)
{
    switch (i) {
    case 0:
        m_context.saveType = ".jpg";
        break;
    case  1:
        m_context.saveType = ".png";
        break;
    case 2:
        m_context.saveType = ".bmp";
        break;
    default:
        m_context.saveType = ".png";
        break;
    }
}

void CaptureWidget::ClickedSaveType2(int i)
{
    switch (i) {
    case 0:
        m_context.saveType = ".jpg";
        break;
    case  1:
        m_context.saveType = ".png";
        break;
    case 2:
        m_context.saveType = ".bmp";
        break;
    default:
        m_context.saveType = ".png";
        break;
    }
}

void CaptureWidget::font_options_defult()
{
#ifndef UNUI
    font_options->move(m_buttonHandler->Font_Options_Window_Pos);
    font_options->Font_size->setValue(m_context.text_thickness);
    font_options->color = m_context.color;
    font_options->Underline = m_context.underline;
    font_options->italic = m_context.italic;
    font_options->bold = m_context.bold;
    font_options->Delete = m_context.deleteline;
#endif
}

void CaptureWidget::font_options2_defult()
{
#ifndef UNUI
    font_options2->move(m_buttonHandler->Font_Options_Window_Pos);
    font_options2->Font_size->setValue(m_context.text_thickness);
    font_options2->color = m_context.color;
    font_options2->Underline = m_context.underline;
    font_options2->italic = m_context.italic;
    font_options2->bold = m_context.bold;
    font_options2->Delete = m_context.deleteline;
#endif
}

void CaptureWidget::fontsize_color_chose_default()
{
#ifndef UNUI
    font_color->color = m_context.color;
    int i = m_context.thickness;
    if (i >= 0 && i <= 2) {
        font_color->color_rect = font_color->m_colorAreaList.at(0);
    } else if (i >= 3 && i < 5) {
        font_color->color_rect = font_color->m_colorAreaList.at(1);
    } else if (i >= 5 && i < 8) {
        font_color->color_rect = font_color->m_colorAreaList.at(2);
    } else if (i >= 8) {
        font_color->color_rect = font_color->m_colorAreaList.at(3);
    } else {
        qDebug()<<"Error";
    }
#endif
}

void CaptureWidget::fontsize_color_chose2_default()
{
#ifndef UNUI
    font_color2->color = m_context.color;
    int i = m_context.thickness;
    if (i >= 0 && i <= 2) {
        font_color2->color_rect = font_color2->m_colorAreaList.at(0);
    } else if (i >= 3 && i < 5) {
        font_color2->color_rect = font_color2->m_colorAreaList.at(1);
    } else if (i >= 5 && i < 8) {
        font_color2->color_rect = font_color2->m_colorAreaList.at(2);
    } else if (i >= 8) {
        font_color2->color_rect = font_color2->m_colorAreaList.at(3);
    } else {
        qDebug()<<"Error";
    }
#endif
}

void CaptureWidget::savetype_chose_default()
{
#ifndef UNUI
    save_location->move(m_buttonHandler->Save_Location_Window_Pos);
    if (m_context.saveType == ".jpg")
        save_location->type_rect = save_location->m_TypeList.at(0);
    else if (m_context.saveType == ".bmp")
        save_location->type_rect = save_location->m_TypeList.at(2);
    else
        save_location->type_rect = save_location->m_TypeList.at(1);
    save_location->SaveDir->setText(m_context.savePath);
#endif
}

void CaptureWidget::savetype_chose2_default()
{
#ifndef UNUI
    save_location->move(m_buttonHandler->Save_Location_Window_Pos);
    if (m_context.saveType == ".jpg")
        save_location2->type_rect = save_location2->m_TypeList.at(0);
    else if (m_context.saveType == ".bmp")
        save_location2->type_rect = save_location2->m_TypeList.at(2);
    else
        save_location2->type_rect = save_location2->m_TypeList.at(1);
    save_location2->SaveDir->setText(m_context.savePath);
#endif
}

void CaptureWidget::hide_window()
{
#ifndef UNUI
    isrunning->set("isrunning", false);
    hide();
#endif
}

void CaptureWidget::show_window()
{
#ifndef UNUI
    if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
    {
        kdk::UkuiStyleHelper::self()->removeHeader(this);
    }
    else
    {
        UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
        windowhelper->removeTitleBar();
    }
    this->show();
    if (!isActiveWindow()) {       // 判断是否是活动窗口
        activateWindow();          // 设置成活动窗口
    }
    setWindowFlags(Qt::BypassWindowManagerHint
//                   | Qt::WindowStaysOnTopHint
                   | Qt::FramelessWindowHint
                   | Qt::Tool);
#endif
}

// format code
void CaptureWidget::hide_ChildWindow()
{
#ifndef UNUI
    font_color->hide();
    font_color2->hide();
    save_location->hide();
    save_location2->hide();
    font_options->hide();
    font_options2->hide();
    SaveAs_btn->hide();
#endif
}

#ifdef ENABLE_RECORD
void CaptureWidget::record_do_sth(CaptureButton *b)
{
    int i = b->m_buttonType;
    switch (i) {
    case CaptureButton::TYPE_RECORD_CURSOR:
        recorder->OnRecordCursorClicked(b->tool()->getIsPressed());
        break;
    case CaptureButton::TYPE_RECORD_FOLLOW_MOUSE:
        recorder->OnRecordFollowMouseClicked(b->tool()->getIsPressed());
        break;
    case CaptureButton::TYPE_RECORD_AUDIO:
        recorder->OnRecordAudioClicked(b->tool()->getIsPressed());
        break;
    }
}

#endif
void CaptureWidget::show_childwindow(CaptureButton *b)
{
#ifndef UNUI
    int i = b->m_buttonType;
    switch (i) {
    case CaptureButton::TYPE_TEXT:
        show_Font_Options_Window(b);
        break;
    case CaptureButton::TYPE_RECT:
    case CaptureButton::TYPE_CIRCLE:
    case CaptureButton::TYPE_LINE:
    case CaptureButton::TYPE_ARROW:
    case CaptureButton::TYPE_PEN:
    case CaptureButton::TYPE_MARKER:
        show_FontSize_Color_Chose_Window(b);
        break;
#ifndef SUPPORT_NEWUI
    case CaptureButton::TYPE_SAVEAS:
        deal_with_SaveAs(b);
        break;
    case CaptureButton::TYPE_OPTION:
        show_Save_Location_Window(b);
        break;
#endif
    default:
        hide_ChildWindow();
        break;
    }
#endif
}

// rect  circle  line arrow  pen marker
void CaptureWidget::show_FontSize_Color_Chose_Window(CaptureButton *b)
{
#ifndef UNUI
    hide_ChildWindow();
    fontsize_color_chose_default();
    fontsize_color_chose2_default();
    qDebug()<<"show FontSize_Color_Chose_Window";
    // 计算弹出窗口的x坐标，确保不会超出父窗口的左边界
    int calculatedX = b->x() -90;
    if(calculatedX < 0){
        calculatedX = 0;
    }
    if (b->y() > m_selection->y()) {
        if (b->y()+150 <= QGuiApplication::primaryScreen()->geometry().height()) {
            font_color_point->setX(calculatedX);
            font_color_point->setY(b->y()+50);
            font_color->move(font_color_point->x(), font_color_point->y());
            font_color->show();
            length = font_color->x() - vectorButtons.first()->pos().x();
        } else {
            font_color_point->setX(calculatedX);
            font_color_point->setY(b->y()-80);
            font_color2->move(font_color_point->x(), font_color_point->y());
            font_color2->show();
            length = font_color2->x() - vectorButtons.first()->pos().x();
        }
    } else {
        if (b->y()-80 >= 0) {
            font_color_point->setX(calculatedX);
            font_color_point->setY(b->y()-80);
            font_color2->move(font_color_point->x(), font_color_point->y());
            font_color2->show();
            length = font_color2->x() - vectorButtons.first()->pos().x();
        } else {
            font_color_point->setX(calculatedX);
            font_color_point->setY(b->y()+50);
            font_color->move(font_color_point->x(), font_color_point->y());
            font_color->show();
            length = font_color->x() - vectorButtons.first()->pos().x();
        }
    }
#endif
}

// options
void CaptureWidget::show_Save_Location_Window(CaptureButton *b)
{
#ifndef UNUI
    hide_ChildWindow();
    savetype_chose_default();
    savetype_chose2_default();
    qDebug()<<"show Save_Location_Window";
    connect(save_location->SaveDir, SIGNAL(pressed()), this, SLOT(ClickedSavedir()));
    connect(save_location2->SaveDir, SIGNAL(pressed()), this, SLOT(ClickedSavedir2()));
    if (b->y() > m_selection->y()) {
        if (b->y()+170 <= QGuiApplication::primaryScreen()->geometry().height())
            save_location->show();
        else
            save_location2->show();
    } else {
        if (b->y()-155 >= 0)
            save_location2->show();
        else
            save_location->show();
    }
#endif
}

// text
void CaptureWidget::show_Font_Options_Window(CaptureButton *b)
{
#ifndef UNUI
    hide_ChildWindow();
    font_options_defult();
    font_options2_defult();
    qDebug()<<"show Font_Options_Window";
    if (b->y() > m_selection->y()) {
        if (b->y()+125 <= QGuiApplication::primaryScreen()->geometry().height())
            font_options->show();
        else
            font_options2->show();
    } else {
        if (b->y()-95 >= 0)
            font_options2->show();
        else
            font_options->show();
    }
#endif
}

void CaptureWidget::deal_with_SaveAs(CaptureButton *b)
{
#ifndef UNUI
    hide_ChildWindow();
    setCursor(Qt::ArrowCursor);
    qDebug()<<"show SaveAs";
    if (b->y()+77 <= QGuiApplication::primaryScreen()->geometry().height()) {
        font_color_point->setX(b->x()-63);
        font_color_point->setY(b->y()+40);
    } else {
        font_color_point->setX(b->x()-63);
        font_color_point->setY(b->y()-37);
    }
    SaveAs_btn->move(font_color_point->x(), font_color_point->y());
    SaveAs_btn->show();
#endif
}

void CaptureWidget::updateButtonsInManager()
{
#ifndef UNUI
    vectorButtons.clear();
    m_uiColor = m_config.uiMainColorValue();
    m_contrastUiColor = m_config.uiContrastColorValue();
    auto buttons = m_config.getButtons();
    for (const CaptureButton::ButtonType &t: buttons) {
        //CaptureButton *b = new CaptureButton(t, taskbar_label);

        if (t == CaptureButton::TYPE_SAVE) {
            continue;
        }
        CaptureButton *b = new CaptureButton(t, taskbar_label);
        b->setColor(m_uiColor);
        b->setAccessibleName(b->tool()->objectName());

        makeChild(b);

        connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->tool(), &CaptureTool::requestAction,
                this, &CaptureWidget::handleButtonSignal);
        connect(b->tool(), &CaptureTool::captureExit,
                this, &CaptureWidget::captureExit);

        if (t == CaptureButton::TYPE_SCROLL) {
            connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::onScrollButtonPressed);
        }
        if (t == CaptureButton::TYPE_COPY){
            b->setToolTip(tr("finish"));
        }

        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
#endif
}

void CaptureWidget::updateButtonsInScrollMode()
{
#ifndef UNUI
    m_buttonHandler->setScrollMode(true);
    return;

    vectorButtons.clear();

    auto buttons = m_config.getButtons(
            #ifdef ENABLE_RECORD
        isRecord
            #endif
        );
    for (const CaptureButton::ButtonType &t: buttons) {
        if(Controller::getInstance()->isManagerStart && t == CaptureButton::TYPE_SAVE)
            continue;

        if (t != CaptureButton::TYPE_SCROLL
            && t != CaptureButton::TYPE_CLOSE
            && t != CaptureButton::TYPE_COPY
            && t != CaptureButton::TYPE_SAVE
#ifndef SUPPORT_NEWUI
            && t != CaptureButton::TYPE_SAVEAS
#endif
             ) {
            continue;
        }

        CaptureButton *b = new CaptureButton(t, taskbar_label);
        b->setAccessibleName(b->tool()->objectName());
#ifndef SUPPORT_NEWUI
        if (t == CaptureButton::TYPE_OPTION) {
            m_sizeIndButton = b;
            m_sizeIndButton->setColor(Qt::black);
        } else
#endif
        if (t == CaptureButton::TYPE_SAVE) {
            b->setColor(Qt::black);
        } else {
            b->setColor(m_uiColor);
        }
        makeChild(b);

        connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::setState);
        connect(b->tool(), &CaptureTool::requestAction,
                this, &CaptureWidget::handleButtonSignal);
        connect(b->tool(), &CaptureTool::captureExit,
                this, &CaptureWidget::captureExit);

        if (t == CaptureButton::TYPE_SCROLL) {
            connect(b, &CaptureButton::pressedButton, this, &CaptureWidget::onScrollButtonPressed);
        }

        setButtonToolTipInScrollMode(b);
        if(Controller::getInstance()->isManagerStart && t == CaptureButton::TYPE_COPY)
            b->setToolTip(tr("finish"));

        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
#endif
}

void CaptureWidget::setButtonToolTipInScrollMode(CaptureButton *b)
{
#ifndef UNUI
    CaptureButton::ButtonType t = b->m_buttonType;
    switch (t) {
        case CaptureButton::TYPE_SCROLL:
            b->setToolTip(tr("Click to exit scroll screenshot mode"));
            break;
        default:
            break;
    }

    return;
#endif
}

QSize CaptureWidget::getTaskBarLabelSize()
{
    // 在长截图模式下工具栏只有几个工具按钮，滚动截图工具按钮，取消工具按钮，保存到剪贴板工具按钮，另存为文件工具按钮。
    int sumWidth = 0;
#ifndef UNUI
    for (int i = 0; i < vectorButtons.size(); i++) {
        CaptureButton *button = vectorButtons.at(i);
        CaptureButton::ButtonType t = button->m_buttonType;
        if(Controller::getInstance()->isManagerStart && t == CaptureButton::TYPE_SAVE)
            continue;

        if (t != CaptureButton::TYPE_SCROLL
            && t != CaptureButton::TYPE_CLOSE
            && t != CaptureButton::TYPE_COPY
            && t != CaptureButton::TYPE_SAVE
#ifndef SUPPORT_NEWUI
            && t != CaptureButton::TYPE_SAVEAS
#endif
             ) {
            continue;
        }

        if (button->width() > 0) {
            sumWidth += button->width();
        } else {
            sumWidth += GlobalValues::buttonBaseSize();
        }
    }

    // 工具按钮之间还需一定的间隔
    sumWidth += (vectorButtons.size() - 2) * m_buttonHandler->getSeparatorWidth();
#endif
    // 工具按钮的宽是固定的。
    return QSize(sumWidth, 44);

}

void CaptureWidget::unsetTransparentForMouseEvents() {
#ifndef UNUI
    QRect selection =  m_context.selection;
    if (selection.isNull()) {
        return ;
    }

    QRegion window( 0,
                    0,
                    width(),
                    height(),
                    QRegion::Rectangle );

    this->setMask(window);

    QPixmap emptyPixmap;
    m_context.scrollPixmap = emptyPixmap;
    m_context.isScrollEnabled = false;
    scrollDirection = 0;

    m_buttonHandler->hide();
    taskbar_label->hide();
    needToUpdate = true;
    update();

    QTimer::singleShot(100, [this]() {
        bool success = true;
        m_context.screenshot = ScreenGrabber().grabEntireDesktop(success);
        vectorButtons.clear();
        m_buttonHandler->clearButtons();

        if (Controller::getInstance()->isManagerStart)
            updateButtonsInManager();
        else
            updateButtons();

        QRect selectionGeometry = m_selection->geometry();
        if (Controller::getInstance()->isManagerStart)
            taskbar_label->setFixedSize(632, 44);
        else
            taskbar_label->setFixedSize(712, 44);
        if(!taskbar_label->getFirstMove()){
            m_buttonHandler->updatePosition(selectionGeometry);
            taskbar_label->move(vectorButtons.first()->pos().x() - 15,
                                vectorButtons.first()->pos().y() - 3);
        }
        m_buttonHandler->show();
        taskbar_label->show();
        needToUpdate = false;
        update();
    });

    return ;
#endif
}

class CaptureWidget::TipDot : public QWidget
{
public:
    TipDot(QWidget *parent = nullptr) : QWidget(parent){
        setFixedSize(12, 12);
    }
    ~TipDot() {}

    void setColor(const QColor &c)
    {
        m_color = c;
    }

protected:
    QColor m_color;
    void paintEvent(QPaintEvent *event) override
    {
#ifndef UNUI
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(0, 0, 12, 12, 6, 6);
        painter.fillPath(path, m_color);
#endif
    }

};

void CaptureWidget::onScrollButtonPressed(CaptureButton *b) {
#ifndef UNUI
    QRect selectionGeometry = m_selection->geometry();
    m_canScorlling = true;
    m_splicingErrorCount = 0;

    if (isScrolling()) {
        // 退出长截图模式，显示竖线
        line_1->show();
        line_2->show();
        m_scrollPreview->hide();
        m_TipDock->hide();
        m_buttonHandler->setScrollMode(false);
        unsetTransparentForMouseEvents();
        return;
    }

    font_color->hide();
    font_options->hide();
    handleButtonSignal(CaptureTool::REQ_CLEAR_MODIFICATIONS);

    line_1->hide();
    line_2->hide();
    setScrollTip(SS_Init, tr("Scroll to start screenshot,until splicing success"));
    m_TipDock->show();
    updateButtonsInScrollMode();
    setButtonToolTipInScrollMode(b);
    m_buttonHandler->updatePosition(selectionGeometry);
    m_buttonHandler->show();

    QRect taskbarLabelGeometry = taskbar_label->geometry();
    if (selectionGeometry.contains(taskbarLabelGeometry) || selectionGeometry.intersects(taskbarLabelGeometry)) {
        setScrollTip(SS_Error, tr("The scrolling area cannot contain a toolbar"));
        m_canScorlling = false;
        m_context.isScrollSuccess = false;
    }

    setTransparentForMouseEvents();

    if (m_selection->height() < 120)
    {
        setScrollTip(SS_Error, tr("The height of the scrolling area is less than 120 pixels"));
        m_canScorlling = false;
        m_context.isScrollSuccess = false;
    }

    QTimer::singleShot(200, [this]() {
        if (m_context.scrollPixmap.isNull()) {
            updatePreview(getScrollhotArea());
        } else {
            updatePreview(m_context.scrollPixmap);
        }
    });
    return ;
#endif
}

bool CaptureWidget::isScrolling()
{
    return m_context.isScrollEnabled;
}

void CaptureWidget::setScrollTip(CaptureWidget::ScorllStatus status, const QString &tip)
{
#ifndef UNUI
    switch (status)
    {
    case SS_Ok:
        m_TipDot->setColor(QColor(0, 255, 0));
        break;
    case SS_Init:
        m_TipDot->setColor(QColor(144, 144, 144));
        break;
    case SS_Warning:
        m_TipDot->setColor(QColor(255, 153, 0));
        break;
    case SS_Error:
        m_TipDot->setColor(QColor(204, 51, 0));
        break;
    default:
        break;
    }

    m_TipText->setText(tip);
    m_TipDot->update();
#endif
}

void CaptureWidget::setTransparentForMouseEvents() {
#ifndef UNUI
    QRect selection =  m_context.selection;
    if (selection.isNull()) {
        return ;
    }

    if (m_canScorlling)
    {
        hide();  // 因为mask v11实现和v10有变化，目前采用隐藏，再show的方式刷新，否则选中区域不能刷新
        QRegion window( 0,
                        0,
                        width(),
                        height(),
                        QRegion::Rectangle );

        QRegion mouseHole(selection.x(), selection.y(), selection.width(), selection.height(), QRegion::Rectangle);

        QRegion r1 = window.QRegion::subtracted(mouseHole);
        this->setMask( r1 );
        show();
    }

    // call paintEvent
    m_context.isScrollEnabled = true;
    update();

    initEventMonitor();

    QTimer::singleShot(200, [this]() {
        bool success = true;
        m_context.screenshot = ScreenGrabber().grabEntireDesktop(success);
        if (!success) {
            return;
        }

        m_context.scrollPixmap = getScrollhotArea();
    });

    return ;
#endif
}

MoveLabel *CaptureWidget::getTaskbar_label()
{
    return taskbar_label;
}

void CaptureWidget::initEventMonitor()
{
#ifndef UNUI
    QDBusInterface *iface = new QDBusInterface("com.kylin.inputdevice.manager", "/", "com.kylin.inputdevice.manager",
        QDBusConnection::systemBus(), this);
    if (!iface->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        return;
    }

    connect(iface, SIGNAL(onMouseWheel(QPoint)), this, SLOT(onMouseScroll(QPoint)));
    return;
#endif
}

QPixmap CaptureWidget::getScrollhotArea()
{
    QRect seclectedRec = m_context.selection;
    return m_context.screenshot.copy(seclectedRec.x()*m_context.screenshot.devicePixelRatio(),
        seclectedRec.y()*m_context.screenshot.devicePixelRatio(),
        seclectedRec.width()*m_context.screenshot.devicePixelRatio(),
        seclectedRec.height()*m_context.screenshot.devicePixelRatio());
}

#define Preview_Max_Width 300
#define Preview_Min_Width 100

void CaptureWidget::updatePreview(const QPixmap &preview)
{
#ifndef UNUI
    if (!m_scrollPreview)
        return;

    QPoint preview_pos;
    int left_width = m_selection->x();
    int right_width = qApp->desktop()->screenGeometry().width() - m_selection->x() - m_selection->width();

    QPixmap pix = preview.scaledToWidth(Preview_Max_Width);
    if (pix.height() > m_selection->y() + m_selection->height() - 20)
    {
        pix = pix.scaledToHeight(m_selection->y() + m_selection->height() - 20);
    }

    int max_width = Preview_Max_Width;

    if (right_width >= Preview_Max_Width + 20)
    {
        // 右边够最宽用右边
        preview_pos.setX(m_selection->x() + m_selection->width() + 10);
    }
    else if (left_width >= Preview_Max_Width + 20)
    {
        // 左边够最宽用左边
        preview_pos.setX(m_selection->x() - pix.width() - 10);
    }
    else if (right_width >= Preview_Min_Width + 20)
    {
        // 右边够最窄用右边
        max_width = right_width - 20;
        preview_pos.setX(m_selection->x() + m_selection->width() + 10);
        pix = pix.scaledToWidth(max_width);
        if (pix.height() > m_selection->y() + m_selection->height() - 20)
        {
            pix = pix.scaledToHeight(m_selection->y() + m_selection->height() - 20);
        }
        preview_pos.setX(m_selection->x() + m_selection->width() + 10);
    }
    else if (left_width >= Preview_Min_Width + 20)
    {
        // 左边够最窄用右边
        max_width = left_width - 20;
        pix = pix.scaledToWidth(max_width);
        if (pix.height() > m_selection->y() + m_selection->height() - 20)
        {
            pix = pix.scaledToHeight(m_selection->y() + m_selection->height() - 20);
        }
        preview_pos.setX(m_selection->x() - pix.width() - 10);
    }
    else
    {
        // 显示不了预览图，就不显示了
        m_scrollPreview->hide();
        return;
    }

    preview_pos.setY(m_selection->y() + m_selection->height() - pix.height());

    m_scrollPreview->show();
    m_scrollPreview->setGeometry(QRect(preview_pos, pix.size()));
    m_scrollPreview->setPixmap(pix);
#endif
}

void CaptureWidget::onMouseScrollUp()
{
#ifndef UNUI
    bool success = true;
    m_context.screenshot = ScreenGrabber().grabEntireDesktop(success);
    if (!success || !m_canScorlling) {
        return;
    }

    if (m_context.screenshot.size().height() == 0) {
        return;
    }

    dealMouseScrollUp();
    return;
#endif
}

void CaptureWidget::dealMouseScroll()
{
#ifndef UNUI
    QPixmap nextPixmap = getScrollhotArea();
    if (m_context.scrollPixmap.isNull()) {
        return;
    }

    QImage scrollImage = m_context.scrollPixmap.toImage();
    cv::Mat scrollMat(scrollImage.height(), scrollImage.width(), CV_8UC4, const_cast<uchar *>(scrollImage.bits()),
                   static_cast<size_t>(scrollImage.bytesPerLine()));

    QImage nextImage = nextPixmap.toImage();
    cv::Mat nextMat(nextImage.height(), nextImage.width(), CV_8UC4, const_cast<uchar *>(nextImage.bits()),
                   static_cast<size_t>(nextImage.bytesPerLine()));

    // 转灰度图像
    cv::Mat scrollGray, nextGray;
    cvtColor(scrollMat, scrollGray, CV_BGR2GRAY);
    cvtColor(nextMat, nextGray, CV_BGR2GRAY);

    cv::Mat templGray = nextGray(cv::Range(0, 120), cv::Range::all());
    cv::Mat tailOfScrollGray = scrollGray(cv::Range(scrollGray.rows - nextGray.rows, scrollGray.rows), cv::Range::all());

    cv::Mat res(tailOfScrollGray.rows - templGray.rows + 1, tailOfScrollGray.cols - templGray.cols + 1, CV_32FC1);
    matchTemplate(tailOfScrollGray, templGray, res, CV_TM_CCOEFF_NORMED);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);

    if (!checkSplicingResult(maxVal, maxLoc.y))
        return;

    // 拼接
    cv::Mat result = cv::Mat::zeros(cvSize(scrollMat.cols, scrollGray.rows + maxLoc.y), scrollMat.type());
    cv::Mat partOfScrollMat = scrollMat(cv::Range(0, scrollGray.rows - nextGray.rows + maxLoc.y), cv::Range::all());

    partOfScrollMat.copyTo(cv::Mat(result, cv::Range(0, scrollGray.rows - nextGray.rows + maxLoc.y), cv::Range::all()));
    nextMat.copyTo(cv::Mat(result, cv::Range(scrollGray.rows - nextGray.rows + maxLoc.y, scrollGray.rows + maxLoc.y), cv::Range::all()));

    QImage mergerImage(result.data, result.cols, result.rows, static_cast<int>(result.step), QImage::Format_ARGB32);
    m_context.scrollPixmap = QPixmap::fromImage(mergerImage);

    // 拼接后的图像需要显示为预览图
    updatePreview(m_context.scrollPixmap);
    setScrollTip(SS_Ok, tr("Splicing success"));

    return;
#endif
}

void CaptureWidget::onMouseScroll(QPoint angleData)
{
#ifndef UNUI
    if(!isScrolling() || !m_canScorlling) {
        return;
    }
//qDebug() << "onMouseScroll----" << m_context.selection;
//qDebug() << "onMouseScroll----" << QCursor::pos();
    /*if(!m_context.selection.contains(QCursor::pos())) {
        return;
    }*/

    static int rollback_count = 0;
    int direction = SCROLL_UP;
    if (angleData.y() < 0) {
        direction = SCROLL_DOWN;
    }
    if ((scrollDirection != 0) && (scrollDirection != direction)) {
        setScrollTip(SS_Warning, tr("Rollback not supported"));
        rollback_count++;
        if (rollback_count > 4) {
            setScrollTip(SS_Error, tr("Too much rollback, unable to continue splicing"));
            // 错误之后就不要继续让截图了
            m_canScorlling = false;
            m_context.isScrollSuccess = false;
        }
        return;
    }
    rollback_count = 0;

    scrollDirection = direction;
    if (direction == SCROLL_UP) {
        onMouseScrollUp();
        return;
    }

    bool success = true;
    m_context.screenshot = ScreenGrabber().grabEntireDesktop(success);
    if (!success) {
        return;
    }

    if (m_context.screenshot.size().height() == 0) {
        return;
    }

    dealMouseScroll();
    return;
#endif
}

bool CaptureWidget::checkSplicingResult(double similarity, int matchedPosition)
{
#ifndef UNUI
    if (similarity < 0.2) {
        // 在正常每次拼接过程中，每次匹配结果相似度都应该很高的，一旦出现了相似度低的情景，则可以说明滚动过快，导致本次图形跟上次没有相似部分
        setScrollTip(SS_Error, tr("Scrolling too fast and splicing failed"));
        m_canScorlling = false;
        m_context.isScrollSuccess = false;
        return false;
    }

    if (similarity >= 0.9 && matchedPosition > 0) {
        m_splicingErrorCount = 0;
        return true;
    }

//    setScrollTip(SS_Warning, tr("Splicing failed, please continue scrolling to try splicing."));

    // 包含了不能滚动的区域，或者滚动区域空白太多，就会连续出现每次匹配结果都是相似很高的位置为上次滚动的图片偏移为0的位置。
    // 即认为本次滚动和上次滚动，滚动区内容没有发生改变。连续出现10次这样的情况给用户发送一个提示信息，无法继续拼接。
    m_splicingErrorCount++;
    if (m_splicingErrorCount > 10) {
        setScrollTip(SS_Error, tr("Too many blank space or region not scrollable"));
        // 错误之后就不要继续让截图了
        m_canScorlling = false;
        m_context.isScrollSuccess = false;
    }

    return false;
#endif
}

void CaptureWidget::dealMouseScrollUp()
{
#ifndef UNUI
    QPixmap nextPixmap = getScrollhotArea();
    if (m_context.scrollPixmap.isNull()) {
        return;
    }

    QImage scrollImage = m_context.scrollPixmap.toImage();
    cv::Mat scrollMat(scrollImage.height(), scrollImage.width(), CV_8UC4, const_cast<uchar *>(scrollImage.bits()),
                   static_cast<size_t>(scrollImage.bytesPerLine()));

    QImage nextImage = nextPixmap.toImage();
    cv::Mat nextMat(nextImage.height(), nextImage.width(), CV_8UC4, const_cast<uchar *>(nextImage.bits()),
                   static_cast<size_t>(nextImage.bytesPerLine()));

    // 转灰度图像
    cv::Mat scrollGray, nextGray;
    cvtColor(scrollMat, scrollGray, CV_BGR2GRAY);
    cvtColor(nextMat, nextGray, CV_BGR2GRAY);

    cv::Mat templGray = scrollGray(cv::Range(0, 120), cv::Range::all());

    cv::Mat res(nextMat.rows - templGray.rows + 1, nextMat.cols - templGray.cols + 1, CV_32FC1);

    matchTemplate(nextGray, templGray, res, CV_TM_CCOEFF_NORMED);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);
    if (!checkSplicingResult(maxVal, maxLoc.y))
        return;

    // 拼接，y到nextGray.rows距离是重叠部分，2个区域相加减去重叠部分，就是合并区的长度
    cv::Mat result = cv::Mat::zeros(cvSize(scrollMat.cols, scrollGray.rows + maxLoc.y), scrollMat.type());
    cv::Mat partOfNextMat = nextMat(cv::Range(0, maxLoc.y), cv::Range::all());

    partOfNextMat.copyTo(cv::Mat(result, cv::Range(0, maxLoc.y), cv::Range::all()));
    scrollMat.copyTo(cv::Mat(result, cv::Range(maxLoc.y, scrollGray.rows + maxLoc.y), cv::Range::all()));

    QImage mergerImage(result.data, result.cols, result.rows, static_cast<int>(result.step), QImage::Format_ARGB32);
    m_context.scrollPixmap = QPixmap::fromImage(mergerImage);

    // 拼接后的图像需要显示为预览图
    updatePreview(m_context.scrollPixmap);
    setScrollTip(SS_Ok, tr("Splicing success"));

    return;
#endif
}

void CaptureWidget::initTipDock()
{
#ifndef UNUI
    m_TipDock = new QWidget(taskbar_label);
    m_TipDock->setAttribute(Qt::WA_TranslucentBackground);
    m_TipDock->move(4, 4);
    m_TipDock->setFixedWidth(400);

    QHBoxLayout *lay = new QHBoxLayout(m_TipDock);
    m_TipDot = new TipDot;
    lay->addWidget(m_TipDot);

    m_TipText = new QLabel;
    m_TipText->setAttribute(Qt::WA_TranslucentBackground);
    lay->addWidget(m_TipText);

    m_TipDock->hide();
#endif
}

void CaptureWidget::remergeScreenshot()
{
#ifndef UNUI
    if(!isScrolling() || !m_canScorlling) {
        hide_window();
        close();
        return;
    }

    if (scrollDirection == SCROLL_UP) {
        onMouseScrollUp();
        return;
    }

    bool success = true;
    m_context.screenshot = ScreenGrabber().grabEntireDesktop(success);
    if (!success) {
        return;
    }

    QPixmap nextPixmap = getScrollhotArea();

    if (m_context.scrollPixmap.isNull()) {
        return;
    }

    QImage scrollImage = m_context.scrollPixmap.toImage();
    cv::Mat scrollMat(scrollImage.height(), scrollImage.width(), CV_8UC4, const_cast<uchar *>(scrollImage.bits()),
                   static_cast<size_t>(scrollImage.bytesPerLine()));

    QImage nextImage = nextPixmap.toImage();
    cv::Mat nextMat(nextImage.height(), nextImage.width(), CV_8UC4, const_cast<uchar *>(nextImage.bits()),
                   static_cast<size_t>(nextImage.bytesPerLine()));

    // 转灰度图像
    cv::Mat scrollGray, nextGray;
    cvtColor(scrollMat, scrollGray, CV_BGR2GRAY);
    cvtColor(nextMat, nextGray, CV_BGR2GRAY);

    cv::Mat templGray = nextGray(cv::Range(0, 120), cv::Range::all());
    cv::Mat tailOfScrollGray = scrollGray(cv::Range(scrollGray.rows - nextGray.rows, scrollGray.rows), cv::Range::all());

    cv::Mat res(tailOfScrollGray.rows - templGray.rows + 1, tailOfScrollGray.cols - templGray.cols + 1, CV_32FC1);
    matchTemplate(tailOfScrollGray, templGray, res, CV_TM_CCOEFF_NORMED);

    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);

    if (!checkSplicingResult(maxVal, maxLoc.y))
        return;

    // 拼接
    cv::Mat result = cv::Mat::zeros(cvSize(scrollMat.cols, scrollGray.rows + maxLoc.y), scrollMat.type());
    cv::Mat partOfScrollMat = scrollMat(cv::Range(0, scrollGray.rows - nextGray.rows + maxLoc.y), cv::Range::all());

    partOfScrollMat.copyTo(cv::Mat(result, cv::Range(0, scrollGray.rows - nextGray.rows + maxLoc.y), cv::Range::all()));
    nextMat.copyTo(cv::Mat(result, cv::Range(scrollGray.rows - nextGray.rows + maxLoc.y, scrollGray.rows + maxLoc.y), cv::Range::all()));

    QImage mergerImage(result.data, result.cols, result.rows, static_cast<int>(result.step), QImage::Format_ARGB32);
    m_context.scrollPixmap = QPixmap::fromImage(mergerImage);

    // 拼接后的图像需要显示为预览图
    updatePreview(m_context.scrollPixmap);
    setScrollTip(SS_Ok, tr("Splicing success"));

    return;
#endif
}

