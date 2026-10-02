/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "homepagewidget.h"
#include "ui_homepagewidget.h"

#include <QLabel>
#include <QSignalMapper>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDBusInterface>
#include <QDebug>
#include <QScrollBar>
#include <QStringList>

#include <QSvgRenderer>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QPushButton>

#include <kysdk/applications/accessinfohelper.h>

#include "mainwindow.h"
#include "utils/functionselect.h"
#include "interface/ukcccommon.h"
using namespace ukcc;
#include "flowlayout.h"
#include "utils/modulefactory.h"

#define STYLE_FONT_SCHEMA  "org.ukui.style"
// 卡片保持 300，保证常见窗口下一行仍能排 3 个大项
#define MODULE_CARD_MIN_WIDTH 300
// 插件名行可用宽：卡片宽 - 图标48 - 边距/间距约96
#define MODULE_LABEL_MAXWIDTH (MODULE_CARD_MIN_WIDTH - 96)

const QStringList KexcludeModule{"update","security","application","search_f"};

static void suppressDecorativeLabel(QLabel *label)
{
    if (!label) {
        return;
    }
    label->setAccessibleName(QStringLiteral(" "));
}

// 按文字实际宽度设宽，并略留边（+2），避免 KLabel 末字被 elide 成「…」
static int pluginLabelFixedWidth(KLabel *label)
{
    if (!label) {
        return 0;
    }
    const QFontMetrics fm(label->font());
    const int textWidth = fm.horizontalAdvance(label->text());
    return qMin(textWidth + 2, MODULE_LABEL_MAXWIDTH);
}

HomePageWidget::HomePageWidget(QWidget *parent, QMap<QString, QGSettings *> map) :
    QWidget(parent),
    vecGsettins(map),
    ui(new Ui::HomePageWidget)
{
    qApp->installEventFilter(this);
    ui->setupUi(this);
    // 获取主窗口
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();

    // 初始化首页
    initUI();
    ui->scrollArea->setBackgroundRole(QPalette::Base);
    // 首页 QScrollArea 默认可 Tab 聚焦但无有效无障碍名称，会多停一次且朗读为空
    ui->scrollArea->setFocusPolicy(Qt::NoFocus);
    ui->scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    ui->scrollArea->verticalScrollBar()->setVisible(false);
    ui->scrollArea->horizontalScrollBar()->setVisible(false);

}

HomePageWidget::~HomePageWidget()
{
    delete ui;
    ui = nullptr;
}

void HomePageWidget::clearPluginLabelPressedStyle()
{
    // KLabel setTristate 点击后会 setFontColor 高亮；跳转离开时未必 leaveEvent，
    // 返回首页时恢复 PlaceholderText，避免多个插件名同时像「选中」
    for (const QList<KLabel *> &labels : mLabels) {
        for (KLabel *label : labels) {
            if (label) {
                label->setFontColorRole(QPalette::PlaceholderText);
            }
        }
    }
}

void HomePageWidget::showEvent(QShowEvent *event)
{
    clearPluginLabelPressedStyle();
    QWidget::showEvent(event);
}

void HomePageWidget::initUI() {
    FlowLayout * flowLayout = new FlowLayout(ui->widget_2, true, 0);
    flowLayout->setContentsMargins(70, 0, 70, 0);
    mModuleMap = UkccCommon::getModuleHideStatus();

    QSignalMapper * moduleSignalMapper = new QSignalMapper(this);
    int totalModule = ModulesFactory::size();
#ifdef WITHKYSEC
        bool isOpen3adm = false;
        if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())){
            isOpen3adm = true;
        }
#endif
    for(int index = 0; index < totalModule; index++) {
        ModuleInfo *curModuleInfo = ModulesFactory::getModuleInfoByIndex(index);
        if (curModuleInfo == nullptr) {
            continue;
        }
        //获取当前模块名
        const QString locale = QLocale::system().name();
        QString modulenamei18nString = curModuleInfo->getModuleNameLocale(locale);
        QString modulenameString = curModuleInfo->moduleName;

        if ((mModuleMap.keys().contains(modulenameString) && !mModuleMap[modulenameString].toBool())
                || (UkccCommon::isTablet() && KexcludeModule.contains(modulenameString))) {
            continue;
        }

        // pangw无安全模块-不显示 社区无更新和安全-不显示
        if ((modulenameString == "security" || modulenameString == "update") && UkccCommon::isCommunity()) {
            continue;
        }

        //三权分立下非root用户，不可设置网络、时间语言、更新
#ifdef WITHKYSEC
        if (isOpen3adm){
            if (modulenameString == "network" || modulenameString == "datetime" || modulenameString == "update") {
                continue;
            }
        }
#endif

        //构建首页10个模块
        //基础Widget
        QWidget * baseWidget = new QWidget();
        baseWidget->setAttribute(Qt::WA_DeleteOnClose);
        baseWidget->setObjectName("itemBaseWidget");
        //baseWidget 的顶级布局
        QVBoxLayout * baseVerLayout = new QVBoxLayout(baseWidget);
        baseVerLayout->setMargin(0);
        baseVerLayout->setSpacing(0);
        //包含显示内容的布局
        QBoxLayout * majorHorLayout = new QHBoxLayout();
        majorHorLayout->setMargin(0);
        majorHorLayout->setSpacing(0);

        //内容Widget的构建
        QPushButton * widget = new QPushButton();
        QString picModuleName = modulenameString;
        widget->setMinimumWidth(MODULE_CARD_MIN_WIDTH);
        widget->setMinimumHeight(97);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->setProperty("useButtonPalette", true);

        if (picModuleName == "search_f") {
            picModuleName = "search";
        }
        widget->setObjectName("itemWidget");
        widget->setFlat(true);
        QHBoxLayout * mainHorLayout = new QHBoxLayout(widget);
        mainHorLayout->setMargin(16);
        mainHorLayout->setSpacing(16);

        QLabel * logoLabel = new QLabel(widget);
        logoLabel->setFixedSize(48, 48);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setScaledContents(true);
        QString themeIconName = curModuleInfo->themeIconName;
        QString localIconName = curModuleInfo->localIconName;
        logoLabel->setPixmap(QIcon::fromTheme(themeIconName,QIcon(localIconName))
                             .pixmap(logoLabel->size()));
        suppressDecorativeLabel(logoLabel);
        const QByteArray settingId(STYLE_FONT_SCHEMA);
        if (QGSettings::isSchemaInstalled(settingId)) {
            QGSettings *mQtSettings = new QGSettings(settingId, QByteArray(), this);
            connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
                if (key == "iconThemeName") {
                    logoLabel->setPixmap(QIcon::fromTheme(themeIconName,QIcon(localIconName))
                                         .pixmap(logoLabel->size()));
                }
            });
        }
        QVBoxLayout * rightVerLayout = new QVBoxLayout();
        rightVerLayout->setContentsMargins(0, 2, 0, 0);
        rightVerLayout->setSpacing(8);

        QLabel * titleLabel = new QLabel(widget);
        titleLabel->setObjectName("mptitleLabel");
        titleLabel->setText(modulenamei18nString);

        QHBoxLayout * funcHorLayout = new QHBoxLayout();
        funcHorLayout->setSpacing(8);

        uint AllWidth = 0;
        QList<KLabel *> Labels;
        QList<PluginInfo> tmpList = curModuleInfo->pluginInfoList;
        int showModuleCount = 0;
        for (int funcIndex = 0; funcIndex < tmpList.size(); funcIndex++) {
            PluginInfo single = tmpList.at(funcIndex);
            qDebug() << showModuleCount << modulenameString << single.type << single.nameString << single.isEnable << single.namei18nString << single.mainShow;
            // 跳过不在首页显示的功能
            if (!single.mainShow) {
                continue;
            }
            if (mModuleMap.keys().contains(single.nameString.toLower())) {
                if (!mModuleMap[single.nameString.toLower()].toBool()) {
                    continue;
                }
            }
            QObject* pluginObj = ModulesFactory::getPluginObjectByName(single.namei18nString);
            if (pluginObj == nullptr) {
                continue;
            }

            QString textName = single.namei18nString;
            KLabel *label = new KLabel(widget);
            label->installEventFilter(this);
            label->setText(textName);
            label->setTristate(true);
            // 可 Tab 聚焦，需暴露插件名；勿用空格名（会覆盖 text，读屏无声）
            label->setAccessibleName(textName);

            // 除了设计要求不在首页显示的label，其他label均需监听gsetting变化，动态调整首页label显示
            if (vecGsettins.contains(single.oriNameString)) {
                connect(vecGsettins[single.oriNameString], &QGSettings::changed,[=](QString key){
                    if (key == SHOW_KEY) {
                        bool isShow = vecGsettins[single.oriNameString]->get(SHOW_KEY).toBool();
                        qDebug() << "isShow = "<< isShow;
                        // 动态调整在首页显示的插件池
                        if (isShow) {
                            if (!mLabels[index].contains(label)) {
                                mLabels[index].insert(funcIndex, label);
                            }
                            int MWidth = 0;
                            int shownCount = 0;
                            for (KLabel * label : mLabels[index]) {
                                // 设置label宽度为字符串宽度，最大为能显示的label的最大宽度
                                label->setFixedWidth(pluginLabelFixedWidth(label));
                                // 总宽度大于最大宽度
                                if (MWidth + label->width() + funcHorLayout->spacing() > MODULE_LABEL_MAXWIDTH) {
                                    // 已显示的label数量为0
                                    if (shownCount == 0) {
                                        label->setVisible(true);
                                        shownCount++;
                                        MWidth += label->width() + funcHorLayout->spacing();
                                    } else {
                                        label->setVisible(false);
                                    }
                                // 总宽度小于最大宽度
                                } else {
                                    shownCount++;
                                    label->setVisible(true);
                                    MWidth += label->width() + funcHorLayout->spacing();
                                }
                            }
                            // 显隐变化后重建组内唯一 Tab 落点
                            setupPluginLabelTabGroup(mLabels[index]);
                        } else {
                            if (mLabels[index].contains(label)) {
                                mLabels[index].removeOne(label);
                            }
                            label->setVisible(false);
                            setupPluginLabelTabGroup(mLabels[index]);
                        }
                    }
                });
            }

            connect(label, &KLabel::clicked, moduleSignalMapper, [=](){
                moduleSignalMapper->map();
            });
            moduleSignalMapper->setMapping(label, pluginObj);
            funcHorLayout->addWidget(label);

            // 插件未启用直接隐藏该label
            if (vecGsettins.contains(single.oriNameString)) {
               if (!vecGsettins[single.oriNameString]->get(SHOW_KEY).toBool()) {
                   label->setVisible(false);
                   continue;
               } else {
                   Labels.append(label);
               }
            } else {
                // gsetting不存在，判断isenable
                if (!single.isEnable) {
                    label->setVisible(false);
                    continue;
                } else {
                    Labels.append(label);
                }
            }

            qDebug() << AllWidth << label->width() << showModuleCount;
            // 设置label宽度为字符串宽度，最大为能显示的label的最大宽度
            label->setFixedWidth(pluginLabelFixedWidth(label));
            // 总宽度大于最大宽度
            if (AllWidth + label->width() + funcHorLayout->spacing() > MODULE_LABEL_MAXWIDTH) {
                // 已显示的label数量为0
                if (showModuleCount == 0) {
                    label->setVisible(true);
                    showModuleCount++;
                    AllWidth += label->width() + funcHorLayout->spacing();
                } else {
                    label->setVisible(false);
                }
            // 总宽度小于最大宽度
            } else {
                showModuleCount++;
                label->setVisible(true);
                AllWidth += label->width() + funcHorLayout->spacing();
            }
        }

        mLabels.insert(index, Labels);
        // 组内仅第一个可见插件可 Tab 进入，其余用方向键在二级项间切换
        setupPluginLabelTabGroup(Labels);
        funcHorLayout->addStretch();

        // 下属无插件，不显示
        if (curModuleInfo->pluginInfoList.size() == 0 || showModuleCount == 0) {
            widget->setVisible(false);
            continue;
        }

        const QString moduleAccessibleName = titleLabel->text();
        widget->setAccessibleName(moduleAccessibleName);
        widget->setAccessibleDescription(QString());
        titleLabel->setAccessibleName(moduleAccessibleName);
        titleLabel->setBuddy(widget);

        const QByteArray styleID(STYLE_FONT_SCHEMA);
        QGSettings *stylesettings = new QGSettings(styleID, QByteArray(), this);
        connect(stylesettings,&QGSettings::changed, [=](QString key) {
            if ("systemFont" == key || "systemFontSize" == key) {
                 int MWidth = 0;
                 int showModuleCount = 0;
                 for (KLabel * label : Labels) {
                     // 设置label宽度为字符串宽度，最大为能显示的label的最大宽度
                     label->setFixedWidth(pluginLabelFixedWidth(label));
                     if (MWidth + label->width() + funcHorLayout->spacing() > MODULE_LABEL_MAXWIDTH) {
                         if (showModuleCount == 0) {
                             label->setVisible(true);
                             showModuleCount++;
                             MWidth += label->width() + funcHorLayout->spacing();
                         } else {
                             label->setVisible(false);
                         }
                     } else {
                         showModuleCount++;
                         label->setVisible(true);
                         MWidth += label->width() + funcHorLayout->spacing();
                     }
                 }
                 setupPluginLabelTabGroup(mLabels[index]);
            }
        });
        connect(widget, &QPushButton::clicked, [=]() {
            //获取模块的第一项跳转
            QString firstFunc;
            QList<PluginInfo> tmpList = curModuleInfo->pluginInfoList;
            for (PluginInfo tmpStruct : tmpList) {
                bool isIntel = UkccCommon::isTablet();

                if ((isIntel && tmpStruct.oriNameString == "User Info")
                      || (!isIntel && tmpStruct.oriNameString == "User Info Intel")) {
                    continue;
                }

                QObject* pluginObj = ModulesFactory::getPluginObjectByName(tmpStruct.namei18nString);
                if (pluginObj == nullptr) {
                    continue;
                }

                // 若该插件不启用，跳转为下一项
                if (vecGsettins.contains(tmpStruct.oriNameString)) {
                    QGSettings *msettings = vecGsettins[tmpStruct.oriNameString];
                    if (msettings) {
                        if (!msettings->get(SHOW_KEY).toBool()) {
                            qDebug() << Q_FUNC_INFO << "plugin name" << tmpStruct.oriNameString << "show is false";
                            continue;
                        } else {
                            qDebug() << Q_FUNC_INFO << "plugin name" << tmpStruct.oriNameString << "show is true";
                        }
                    }
                }
                if (mModuleMap.isEmpty() || !mModuleMap.contains(tmpStruct.nameString.toLower()) || mModuleMap[tmpStruct.nameString.toLower()].toBool()) {
                    firstFunc = tmpStruct.namei18nString;
                    UkccCommon::buriedSettings(tmpStruct.oriNameString, nullptr, "home clicked");
                    //跳转
                    pmainWindow->functionBtnClicked(pluginObj);
                    break;
                }
            }
        });

        rightVerLayout->addStretch();
        rightVerLayout->addWidget(titleLabel);
        rightVerLayout->addLayout(funcHorLayout);
        rightVerLayout->addStretch();

        mainHorLayout->addWidget(logoLabel);
        mainHorLayout->addLayout(rightVerLayout);
        mainHorLayout->addStretch();

        widget->setLayout(mainHorLayout);

        majorHorLayout->addWidget(widget);
        majorHorLayout->addStretch();

        baseVerLayout->addLayout(majorHorLayout);
        baseVerLayout->addStretch();

        baseWidget->setLayout(baseVerLayout);

        flowLayout->addWidget(baseWidget);
    }
    connect(moduleSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginBtnClicked(QObject*)));
}

QGSettings *HomePageWidget::setGsettingsPath(QList<char *> list ,  QString name)
{
    // 为每个插件创建动态QGSettings对象，用于监听插件是否隐藏
    QByteArray ba;
    char *path;
    ba = (QString("%1%2").arg(name).arg("/")).toUtf8();
    path = ba.data();
    const QByteArray id(PLUGINS_SCHEMA);
    if (!QGSettings::isSchemaInstalled(id)) {
        return nullptr;
    }
    QGSettings *settings = nullptr;
    QString plugin = QString("%1%2%3").arg(PLUGINS_PATH).arg(name).arg("/");
    settings = new QGSettings(id, plugin.toUtf8().data(), this);

    //判断是否已存在该路径,不存在则赋初值
    for (int j = 0; j < list.count(); j++) {
        if (!qstrcmp(path, list.at(j))){
            return settings;
        }
    }
    settings->set(PLUGIN_NAME , name);
    settings->set(SHOW_KEY , true);
    return settings;
}

const QPixmap HomePageWidget::loadSvg(const QString &fileName, COLOR color)
{
    int size = 48;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size = 96;
    } else if (3 == ratio) {
        size = 144;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

QPixmap HomePageWidget::drawSymbolicColoredPixmap(const QPixmap &source, COLOR cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ( WHITE == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if( BLACK == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if (GRAY == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if(HIGHLIGHT == cgColor){
                    QColor highLightColor = palette().color(QPalette::Highlight);
                    color.setRed(highLightColor.red());
                    color.setGreen(highLightColor.green());
                    color.setBlue(highLightColor.blue());
                    img.setPixelColor(x, y, color);
                }
                else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}


bool HomePageWidget::isHomePluginLabel(QObject *obj) const
{
    KLabel *label = qobject_cast<KLabel *>(obj);
    if (!label) {
        return false;
    }
    for (const QList<KLabel *> &labels : mLabels) {
        if (labels.contains(label)) {
            return true;
        }
    }
    return false;
}

QList<KLabel *> HomePageWidget::pluginLabelGroup(KLabel *label) const
{
    for (const QList<KLabel *> &labels : mLabels) {
        if (labels.contains(label)) {
            return labels;
        }
    }
    return {};
}

void HomePageWidget::setupPluginLabelTabGroup(const QList<KLabel *> &labels)
{
    KLabel *tabStop = nullptr;
    for (KLabel *label : labels) {
        if (!label || !label->isVisible()) {
            continue;
        }
        // 组内默认 ClickFocus，不进 Tab 链；仅保留一个 StrongFocus（Tab 一次进入插件）
        label->setFocusPolicy(Qt::ClickFocus);
        if (!tabStop) {
            tabStop = label;
        }
    }
    if (tabStop) {
        tabStop->setFocusPolicy(Qt::StrongFocus);
    }
}

void HomePageWidget::takePluginLabelTabStop(KLabel *current)
{
    if (!current) {
        return;
    }
    const QList<KLabel *> group = pluginLabelGroup(current);
    for (KLabel *label : group) {
        if (!label || !label->isVisible()) {
            continue;
        }
        label->setFocusPolicy(label == current ? Qt::StrongFocus : Qt::ClickFocus);
    }
}

KLabel *HomePageWidget::findPluginLabelNeighbor(KLabel *current, int key) const
{
    if (!current) {
        return nullptr;
    }

    // 保持首页布局顺序（从左到右），避免几何计算在未完成布局时失效
    QList<KLabel *> units;
    for (KLabel *label : pluginLabelGroup(current)) {
        if (label && label->isVisible()) {
            units.append(label);
        }
    }
    if (units.size() < 2) {
        return nullptr;
    }

    const int idx = units.indexOf(current);
    if (idx < 0) {
        return nullptr;
    }

    switch (key) {
    case Qt::Key_Right:
    case Qt::Key_Down:
        if (idx + 1 < units.size()) {
            return units.at(idx + 1);
        }
        break;
    case Qt::Key_Left:
    case Qt::Key_Up:
        if (idx > 0) {
            return units.at(idx - 1);
        }
        break;
    default:
        break;
    }
    return nullptr;
}

bool HomePageWidget::handlePluginLabelKeyEvent(KLabel *label, QKeyEvent *keyEvent)
{
    if (!label || !keyEvent) {
        return false;
    }
    const int key = keyEvent->key();
    if (key == Qt::Key_Left || key == Qt::Key_Right
            || key == Qt::Key_Up || key == Qt::Key_Down) {
        if (KLabel *neighbor = findPluginLabelNeighbor(label, key)) {
            takePluginLabelTabStop(neighbor);
            // 新版可点 KLabel（setTristate）会画焦点框，但需键盘焦点原因；
            // OtherFocusReason 不会触发焦点框，方向键切换用 TabFocusReason。
            neighbor->setFocus(Qt::TabFocusReason);
        }
        return true;
    }
    if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter) {
        // KLabel 在 mouseRelease 里发 clicked；键盘激活发一次等效释放事件
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                                 label->rect().center(),
                                 Qt::LeftButton,
                                 Qt::LeftButton,
                                 Qt::NoModifier);
        QApplication::sendEvent(label, &releaseEvent);
        return true;
    }
    return false;
}

bool HomePageWidget::eventFilter(QObject *watched, QEvent *event)
{
    auto isHomeModuleCardWidget = [](QObject *obj) -> bool {
        QPushButton *btn = qobject_cast<QPushButton *>(obj);
        return btn && btn->objectName() == QLatin1String("itemWidget");
    };

    // 主窗口/侧栏等可能占用方向键快捷键；先 accept ShortcutOverride，否则 label 收不到 KeyPress
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();
        const bool arrowOrActivate = (key == Qt::Key_Left || key == Qt::Key_Right
                || key == Qt::Key_Up || key == Qt::Key_Down
                || key == Qt::Key_Space || key == Qt::Key_Return
                || key == Qt::Key_Enter);
        if (arrowOrActivate) {
            QWidget *fw = QApplication::focusWidget();
            if (isHomePluginLabel(watched) || isHomePluginLabel(fw)
                    || isHomeModuleCardWidget(watched) || isHomeModuleCardWidget(fw)) {
                keyEvent->accept();
                return true;
            }
        }
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();
        const bool isArrow = (key == Qt::Key_Left || key == Qt::Key_Right
                || key == Qt::Key_Up || key == Qt::Key_Down);

        KLabel *label = qobject_cast<KLabel *>(watched);
        if (!label || !isHomePluginLabel(label)) {
            label = qobject_cast<KLabel *>(QApplication::focusWidget());
        }
        if (label && isHomePluginLabel(label)) {
            if (handlePluginLabelKeyEvent(label, keyEvent)) {
                return true;
            }
        }

        // 模块卡片上方向键不切换（模块间只走 Tab）
        QObject *cardObj = isHomeModuleCardWidget(watched) ? watched : QApplication::focusWidget();
        if (isArrow && isHomeModuleCardWidget(cardObj)) {
            return true;
        }
    }

    if (isHomePluginLabel(watched)) {
        KLabel *label = static_cast<KLabel *>(watched);
        if (event->type() == QEvent::MouseButtonPress) {
            return true;
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            // 鼠标点选后把唯一 Tab 落点移到当前项
            takePluginLabelTabStop(label);
            label->setFocus(Qt::MouseFocusReason);
        } else if (event->type() == QEvent::FocusIn) {
            takePluginLabelTabStop(label);
        }
    }
    if (event->type() == QEvent::MouseMove) {
        if (ui->scrollArea->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
            ui->scrollArea->verticalScrollBar()->setVisible(true);
        } else {
            ui->scrollArea->verticalScrollBar()->setVisible(false);
        }
    }
    return QObject::eventFilter(watched, event);
}

