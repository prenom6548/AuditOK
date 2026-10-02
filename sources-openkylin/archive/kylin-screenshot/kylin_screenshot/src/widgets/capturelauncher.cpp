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

#include "capturelauncher.h"
#include "src/core/controller.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/notificationwidget.h"
#include "src/utils/screengrabber.h"
#include "src/utils/screenshotsaver.h"
#include "src/widgets/imagelabel.h"
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QMimeData>
#include <QDrag>
#include <QFormLayout>
#include <QStandardPaths>
#include "xatomhelper.h"
#define ICON_THEME_NAME           "iconThemeName"
// https://github.com/KDE/spectacle/blob/941c1a517be82bed25d1254ebd735c29b0d2951c/src/Gui/KSWidget.cpp
// https://github.com/KDE/spectacle/blob/941c1a517be82bed25d1254ebd735c29b0d2951c/src/Gui/KSMainWindow.cpp
#define ORG_UKUI_STYLE            "org.ukui.style"

CaptureLauncher::CaptureLauncher(QWidget *parent) :
    QWidget(parent),
    m_id(0)
{
    setAccessibleName("kylin-screenshot_QWidget_captureLauncher");
    const QByteArray style_id(ORG_UKUI_STYLE);
    if(QGSettings::isSchemaInstalled(style_id)){
        m_styleSettings = new QGSettings(style_id);
    }
    setWindowIcon(QIcon::fromTheme("kylin-screenshot"));
    // setWindowTitle(tr("Screenshot"));
    setAttribute(Qt::WA_DeleteOnClose);
    connect(Controller::getInstance(), &Controller::captureTaken,
            this, &CaptureLauncher::captureTaken);
    connect(Controller::getInstance(), &Controller::captureFailed,
            this, &CaptureLauncher::captureFailed);
#ifndef UNUI
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(winId(), hints);
#endif

    m_titleIcon_label = new  QLabel(this);
    m_titleIcon_label->setPixmap(QPixmap(QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24,
                                                                                           24))));
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ"){
        m_titleIcon_label->move(576, 8);
    }else{
        m_titleIcon_label->move(8, 8);
    }
    m_titleIcon_label->setAccessibleName("kylin-screenshot_QLabel_titleIcon");
    m_titleName_label = new QLabel(this);
    m_titleName_label->setText(tr("Screenshot"));
    m_titleName_label->setAlignment(Qt::AlignLeft);
    m_titleName_label->setFixedSize(170, 25);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_titleName_label->move(406, 8);
    }else{
        m_titleName_label->move(40, 8);
    }
    m_titleName_label->setAccessibleName("kylin-screenshot_QLabel_titleName");
    m_exit_btn = new QPushButton(this);
    m_exit_btn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_exit_btn->setFixedSize(24, 24);
    m_exit_btn->setIconSize(QSize(16, 16));
    m_exit_btn->setProperty("isWindowButton", 0x2);
    m_exit_btn->setProperty("useIconHighlightEffect", 0x8);
    m_exit_btn->setToolTip(tr("Close"));
    m_exit_btn->setFlat(true);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_exit_btn->move(8, 8);
    }else{
        m_exit_btn->move(598, 8);
    }
    connect(m_exit_btn, &QPushButton::clicked,
            this, &CaptureLauncher::close);
    m_exit_btn->setAccessibleName("kylin-screenshot_QPushButton_exit");
    m_min_btn = new QPushButton(this);
    m_min_btn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_min_btn->setFixedSize(24, 24);
    m_min_btn->setIconSize(QSize(16, 16));
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_min_btn->move(40, 8);
    }else{
        m_min_btn->move(566, 8);
    }
    m_min_btn->setProperty("isWindowButton", 0x1);
    m_min_btn->setProperty("useIconHighlightEffect", 0x2);
    m_min_btn->setToolTip(tr("Min"));
    m_min_btn->setFlat(true);
    connect(m_min_btn, &QPushButton::clicked,
            // this, &CaptureLauncher::hide);
            this, &CaptureLauncher::showMinimized);
    m_min_btn->setAccessibleName("kylin-screenshot_QPushButton_min");
    m_imageLabel = new ImageLabel(this);
    bool ok;
    m_imageLabel->setScreenshot(ScreenGrabber().grabEntireDesktop(ok));
    if (!ok) {
    }
    m_imageLabel->setFixedSize(320, 180);
    m_min_btn->setIconSize(QSize(16, 16));
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_imageLabel->move(284, 56);
    }else{
        m_imageLabel->move(24, 56);
    }

    connect(m_imageLabel, &ImageLabel::dragInitiated,
            this, &CaptureLauncher::startDrag);
    m_imageLabel->setAccessibleName("kylin-screenshot_QLabel_imageLabel");
    m_CaptureModeLabel = new QLabel(tr("Capture Mode"), this);
    m_CaptureModeLabel->setFixedSize(105, 30);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_CaptureModeLabel->move(170, 56);
    }else{
        m_CaptureModeLabel->move(370, 56);
    }
    m_CaptureModeLabel->setAccessibleName("kylin-screenshot_QLabel_captureMode");
    m_captureType = new QComboBox(this);
    m_captureType->setFixedSize(174, 36);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_captureType->move(10, 96);
    }else{
        m_captureType->move(432, 96);
    }
    // TODO remember number
    m_captureType->insertItem(1, tr("Rectangular Region"), CaptureRequest::GRAPHICAL_MODE);
    m_captureType->insertItem(2, tr("Full Screen (All Monitors)"), CaptureRequest::FULLSCREEN_MODE);
    m_captureType->setAccessibleName("kylin-screenshot_QComboBox_captureType");
    m_delaySpinBox = new QSpinBox(this);
    m_delaySpinBox->setSingleStep(1.0);
    m_delaySpinBox->setFixedSize(174, 36);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_delaySpinBox->move(10, 140);
    }else{
        m_delaySpinBox->move(432, 140);
    }
    m_delaySpinBox->setMinimumWidth(160);
    connect(m_delaySpinBox,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, [this](int val)
    {
        QString sufix = val == 1 ? tr(" second") : tr(" seconds");
        this->m_delaySpinBox->setSuffix(sufix);
    });
    m_delaySpinBox->setAccessibleName("kylin-screenshot_QSpinBox_delaySpinBox");
    m_launchButton = new QPushButton(tr("Take shot"), this);
    m_launchButton->setFixedSize(96, 36);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_launchButton->move(10, 200);
    }else{
        m_launchButton->move(510, 200);
    }
    connect(m_launchButton, &QPushButton::pressed,
            this, &CaptureLauncher::startCapture);
    m_launchButton->setFocus();
    m_launchButton->setProperty("isImportant", true);
    m_launchButton->setAccessibleName("kylin-screenshot_QPushButton_launchButton");
    m_areaLabel = new  QLabel(this);
    m_areaLabel->setText(tr("Area:"));
    // m_areaLabel->setFixedSize(28, 24);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_areaLabel->move(200, 100);
    }else{
        m_areaLabel->move(370, 100);
    }
    m_areaLabel->setAccessibleName("kylin-screenshot_QLabel_areaLabel");
    m_delayLabel = new  QLabel(this);
    m_delayLabel->setText(tr("Delay:"));
    // m_delayLabel->setFixedSize(28, 24);
    if(systemLang == "ug_CN" || systemLang == "kg_KG" || systemLang == "kk_KZ" ){
        m_delayLabel->move(200, 146);
    }else{
        m_delayLabel->move(370, 146);
    }
    m_delayLabel->setAccessibleName("kylin-screenshot_QLabel_delayLabel");
    setFixedSize(630, 260);

    setFont();
    connect(m_styleSettings,&QGSettings::changed,this,[=](const QString &key) {
       if (key == "systemFontSize" || key == "systemFont") {
            setFont();
       }
       if(key==ICON_THEME_NAME){
           setWindowIcon(QIcon::fromTheme("kylin-screenshot"));
           m_titleIcon_label->setPixmap(QPixmap(QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24,24))));
       }
       if (key == "styleName" || key == "widgetThemeName") {
           updateTitleColor();
       }
    });
    installEventFilter(this);
}

// HACK: https://github.com/KDE/spectacle/blob/fa1e780b8bf3df3ac36c410b9ece4ace041f401b/src/Gui/KSMainWindow.cpp#L70
void CaptureLauncher::startCapture()
{
    hide();
    auto mode = static_cast<CaptureRequest::CaptureMode>(
        m_captureType->currentData().toInt());
    CaptureRequest req(mode, 1000 + m_delaySpinBox->value() * 1000);
    m_id = req.id();
    Controller::getInstance()->requestCapture(req);
}

void CaptureLauncher::startDrag()
{
    QDrag *dragHandler = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData(m_imageLabel->pixmap());
    dragHandler->setMimeData(mimeData);

    dragHandler->setPixmap(m_imageLabel->pixmap()
                           ->scaled(256, 256, Qt::KeepAspectRatioByExpanding,
                                    Qt::SmoothTransformation));
    dragHandler->exec();
}

void CaptureLauncher::captureTaken(uint id, QPixmap p, bool seedMsg)
{
    if (id == m_id) {
        m_id = 0;
        m_imageLabel->setScreenshot(p);
        auto mode = static_cast<CaptureRequest::CaptureMode>(
            m_captureType->currentData().toInt());
        QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        if (mode == CaptureRequest::FULLSCREEN_MODE) {
            ScreenshotSaver().saveToFilesystem(p, a.at(0), ".png", seedMsg);
        }
//        show();
    }
}

void CaptureLauncher::captureFailed(uint id)
{
    if (id == m_id) {
        m_id = 0;
//        show();
    }
}

void CaptureLauncher::setFont()
{
    QString fontFamily = m_styleSettings->get("systemFont").toString();
    double fontSize = m_styleSettings->get("systemFontSize").toDouble();
    if (fontSize >= 15)
    {
        m_launchButton->setFixedSize(106, 36);
    }
    else
    {
        m_launchButton->setFixedSize(96, 36);
    }

    QFont f(fontFamily);
    f.setPointSizeF(fontSize + 2);
    m_CaptureModeLabel->setFont(f);
    f.setPointSizeF(fontSize);
    m_titleName_label->setFont(f);
    m_areaLabel->setFont(f);
    m_delayLabel->setFont(f);
    m_launchButton->setFont(f);
    m_delaySpinBox->setFont(f);
    m_captureType->setFont(f);
    // ========= 根据图标中心线和字体高度调整标题文字的纵坐标 =========
    // 先拿到当前这套字体一行文字的高度
    QFontMetrics fm(f);
    int textHeight = fm.height();
    // 图标在父窗口中的位置和高度（以实际显示的 pixmap 高度为准）
    int iconTop = m_titleIcon_label->y();
    int iconHeight = m_titleIcon_label->pixmap()->height();
    int iconCenterY = iconTop + iconHeight / 2; // 图标的竖直中心线
    // 让“截图”这行字的中心 = 图标中心
    int labelTopY = iconCenterY - textHeight / 2;
    labelTopY = qMax(0, labelTopY); // 防止出现负数

    // 只改 y，不改 x（x 仍然按语言在构造函数里控制）
    m_titleName_label->move(m_titleName_label->x(), labelTopY);
}

void CaptureLauncher::changeEvent(QEvent *event){
    if( event->type() == QEvent::ActivationChange){
        updateTitleColor();
    }
    QWidget::changeEvent(event);
}

void CaptureLauncher::updateTitleColor()
{
    QString themeStyle = m_styleSettings->get("styleName").toString();
    bool isDark = (themeStyle == "ukui-dark" || themeStyle == "ukui-black");
    bool active = this->isWindow() ? isActiveWindow() : window()->isActiveWindow();
    // 根据窗口激活状态和主题，通过调色板设置颜色
    // 同时利用 enabled/disabled 状态，让未激活窗口自动使用灰色（Disabled 颜色）
    m_titleName_label->setEnabled(active);

    QPalette pal = m_titleName_label->palette();
    if (active) {
        // 激活窗口：深色主题用白色，浅色主题用黑色
        pal.setColor(QPalette::WindowText, isDark ? Qt::white : Qt::black);
        m_titleName_label->setPalette(pal);
    }
}

