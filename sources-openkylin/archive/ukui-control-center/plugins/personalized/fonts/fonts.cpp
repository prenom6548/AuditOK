/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include <QLabel>
#include <QStringList>
#include <QDebug>
#include <QHBoxLayout>
#include <QListView>
#include <QObject>
#include <QMessageBox>
#include <QFileInfo>
#include <QEvent>
#include <QAccessible>
#include <QLocale>
#include <QtMath>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_MULTIPLE_MASTERS_H

#include "fonts.h"
#include "ui_fonts.h"
#include "mthread.h"
#include "kiconbar.h"
#include "fontunit.h"
#include "../../../shell/pinyin.h"
#include "../../../shell/customstyle.h"
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/klineframe.h>

#define STYLE_FONT_SCHEMA  "org.ukui.style"
#define SYSTEM_FONT_SIZE_KEY    "systemFontSize"
#define SYSTEM_FONT_KEY "systemFont"

#define UKCC_SCHEMA        "org.ukui.control-center"
#define FONT_SELECT_FILE   "fontSelectFile"
#define FONT_DEFAULT_PATH  "fontDefaultPath"

#define INTERFACE_SCHEMA   "org.mate.interface"
#define MONOSPACE_FONT_KEY "monospace-font-name"

#define GNOME_SCHEMA       "org.gnome.desktop.interface"

#define FONT_PATH "/ukui-control-center/"

namespace {
// 拼音字典以声调数字标记读音，搜索索引使用用户更常输入的无声调形式。
QString removePinyinTone(const QString &pinyin)
{
    QString result;
    for (const QChar &character : pinyin) {
        if (!character.isDigit()) {
            result.append(character);
        }
    }
    return result;
}
}

Fonts::Fonts() : mFirstLoad(true)
{
    qRegisterMetaType<ViewData>("ViewData");
    qDBusRegisterMetaType<ViewData>();
    qRegisterMetaType<ViewDataList>("ViewDataList");
    qDBusRegisterMetaType<ViewDataList>();
    m_fontInterface = new QDBusInterface("org.ukui.ukcc.session",
                                         "/Font",
                                         "org.ukui.ukcc.session.Font",
                                         QDBusConnection::sessionBus(), this);
    if (!m_fontInterface->isValid()) {
        qCritical() << "org.ukui.ukcc.session.Font DBus error:" << m_fontInterface->lastError();
    }
    m_installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + "/";
    pluginName = tr("Fonts");
    pluginType = PERSONALIZED;
}

Fonts::~Fonts()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

QString Fonts::plugini18nName() {
    return pluginName;
}

int Fonts::pluginTypes() {
    return pluginType;
}

QWidget *Fonts::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Fonts;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        ui->titleLabel->setContentsMargins(16, 0, 0, 0);
        setupComponent();
        initSearchText();
        setupConnect();
        initFontStatus();
    }
    QTimer::singleShot(500, [=]() {
        clickFontFile();
    });
    return pluginWidget;
}

bool Fonts::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == pluginWidget && event && event->type() == QEvent::EnabledChange) {
        qDebug() << Q_FUNC_INFO << "before updateDragWidgetState";
        updateDragWidgetState();
    }
    return QObject::eventFilter(watched, event);
}

const QString Fonts::name() const {
    return QStringLiteral("Fonts");
}

bool Fonts::isShowOnHomePage() const
{
    return true;
}

QIcon Fonts::icon() const
{
    return QIcon::fromTheme("ukui-font-symbolic");
}

bool Fonts::isEnable() const
{
    return true;
}

void Fonts::initSearchText() {
    //~ contents_path /Fonts/Fonts
    ui->titleLabel->setText(tr("Fonts"));

    //~ contents_path /Fonts/Available fonts
    m_fontTitleLabel->setText(tr("Available fonts"));

    //~ contents_path /Fonts/Default
    m_fontDefaultButton->setText(tr("Default"));
}


void Fonts::insertWidgetAt(QWidget* widget, int index) {
    QList<QWidget*> items;
    FontUnit* curWidget = qobject_cast<FontUnit*>(widget);
    if (!curWidget) {
        return;
    }
    while (QLayoutItem* item = m_flowLayout->takeAt(0)) {
        FontUnit* widget = qobject_cast<FontUnit*>(item->widget());
        if (widget && widget->getViewData()->path != curWidget->getViewData()->path) {
            items.append(item->widget());
        } else {
            delete item;  // 如果不是部件，则删除布局项
            item = nullptr;
        }
    }

    items.insert(index, widget);

    for (QWidget* item : items) {
        m_flowLayout->addWidget(item);
    }
}

void Fonts::setupComponent(){
    ui->titleLabel->setContentsMargins(16, 0, 0, 0);
    ui->titleLabel->setMinimumWidth(100);
    KDK_EXTEND_ALL_INFO_FORMAT(ui->titleLabel, name(), "", "");

    // 字体大小
    QFrame* fontSizeFrame = new QFrame(pluginWidget);

    KLabel* fontSizeLabel = new KLabel(fontSizeFrame);
    fontSizeLabel->setMinimumWidth(150);
    KDK_EXTEND_ALL_INFO_FORMAT(fontSizeLabel, name(), "", "");

    //~ contents_path /Fonts/Font size
    fontSizeLabel->setText(tr("Font size"));


    m_fontSizeSlider = new KSlider(fontSizeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontSizeSlider, name(), "", "");
    m_fontSizeSlider->setRange(1, 4);
    m_fontSizeSlider->setTickInterval(1);

    m_fontSizeSlider->setPageStep(1);
    KLabel* smallLabel = new KLabel(fontSizeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(smallLabel, name(), "", "");
    smallLabel->setText(tr("Small"));
    KLabel* largeLabel = new KLabel(fontSizeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(largeLabel, name(), "", "");
    largeLabel->setText(tr("Large"));

    QHBoxLayout *fontSizeHboxLayout = new QHBoxLayout(fontSizeFrame);
    fontSizeHboxLayout->setContentsMargins(16, 16, 16, 16);

    fontSizeHboxLayout->addWidget(fontSizeLabel);
    fontSizeHboxLayout->addStretch();
    fontSizeHboxLayout->addWidget(smallLabel);
    fontSizeHboxLayout->addWidget(m_fontSizeSlider);
    fontSizeHboxLayout->addWidget(largeLabel);
    ui->frame_3->layout()->addWidget(fontSizeFrame);

    // 等宽字体
    QFrame* monoFrame = new QFrame(pluginWidget);
    monoFrame->setMinimumSize(QSize(550, 0));
    monoFrame->setMaximumSize(QSize(16777215, 16777215));
    monoFrame->setFrameShape(QFrame::Box);

    KLabel* monoTitle1 = new KLabel(monoFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(monoTitle1, name(), "", "");

    //~ contents_path /Fonts/Mono Font
    monoTitle1->setText(tr("Mono Font"));
    m_monoTitleDesc = new KLabel(monoFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_monoTitleDesc, name(), "", "");
    m_monoTitleDesc->setText(tr("Take effect in applications such as programming that can configure Mono font"));
    m_monoTitleDesc->setFontColor(ThemeController::getCustomColorFromDT("kfont-secondary"));

    m_monoCombox = new QComboBox(monoFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_monoCombox, name(), "", "");

    QFontDatabase fontdb;
    QStringList fontfamiles = fontdb.families();
    m_monoModel = dynamic_cast<QStandardItemModel *>(m_monoCombox->model());
    for (QString fontValue : fontfamiles) {
        if (fontValue.contains("Mono") && !fontValue.contains("Ubuntu", Qt::CaseInsensitive)) {
            QStandardItem* monoItem = new QStandardItem(fontValue);
            monoItem->setFont(QFont(fontValue));
            m_monoModel->appendRow(monoItem);
        }
    }

    QVBoxLayout *monoVerticalLayout = new QVBoxLayout;
    monoVerticalLayout->setSpacing(0);
    monoVerticalLayout->setContentsMargins(0, 0, 0, 0);
    monoVerticalLayout->addWidget(monoTitle1);
    monoVerticalLayout->addWidget(m_monoTitleDesc);

    QHBoxLayout *monoHboxLayout = new QHBoxLayout(monoFrame);
    monoHboxLayout->setContentsMargins(16, 11, 16, 11);
    monoHboxLayout->addLayout(monoVerticalLayout, 1);
    monoHboxLayout->addWidget(m_monoCombox, 1);
    monoHboxLayout->setStretchFactor(m_monoCombox, 1);

    KHLineFrame* rotationFrame = new KHLineFrame(monoFrame);
    ui->frame_3->layout()->addWidget(rotationFrame);
    ui->frame_3->layout()->addWidget(monoFrame);

    // 恢复默认
    QFrame* defaultFrame = new QFrame(pluginWidget);

    KLabel* defaultLabel = new KLabel(defaultFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(defaultLabel, name(), "", "");
    //~ contents_path /Fonts/Restore font-related settings to the system's initial state
    defaultLabel->setText(tr("Restore font-related settings to the system's initial state"));
    defaultLabel->setMinimumWidth(300);
    m_fontDefaultButton = new KPushButton(defaultFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontDefaultButton, name(), "", "");

    QHBoxLayout *defaultHboxLayout = new QHBoxLayout(defaultFrame);
    defaultHboxLayout->setContentsMargins(16, -1, 16, -1);
    defaultHboxLayout->addWidget(defaultLabel);
    defaultHboxLayout->addStretch();
    defaultHboxLayout->addWidget(m_fontDefaultButton);

    KHLineFrame* defaultLine = new KHLineFrame(defaultFrame);
    ui->frame_3->layout()->addWidget(defaultLine);
    ui->frame_3->layout()->addWidget(defaultFrame);

    // 可用字体
    m_fontTitleLabel = new QLabel(ui->fontTitleFrame);
    m_fontTitleLabel->setContentsMargins(16,0,0,0);
    KDK_EXTEND_ALL_INFO_FORMAT(m_fontTitleLabel, name(), "", "");

    m_searchArea = new KSearchLineEdit(ui->fontTitleFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_searchArea, name(), "", "");
    m_searchArea->setClearButtonEnabled(false);
    m_searchArea->installEventFilter(this);
    m_searchArea->setPlaceholderText(tr("Enter the font to search"));

    m_fontTitleLayout = new QHBoxLayout(ui->fontTitleFrame);
    m_fontTitleLayout->setContentsMargins(0, 40, 0, 0);
    m_fontTitleLayout->addWidget(m_fontTitleLabel, 1);
    m_fontTitleLayout->addStretch();
    m_fontTitleLayout->addWidget(m_searchArea, 1);

    // 安装字体
    m_kDragWidget = new KDragWidget(ui->fontContentFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_kDragWidget, name(), "", "");
    m_kDragWidget->iconButton()->setIcon(QIcon::fromTheme("list-add.symbolic"));
    m_kDragWidget->textLabel()->setText(tr("Drag the font here or click the \"+\" to add it"));
    m_kDragWidget->setFixedHeight(120);
    QStringList list = {tr("font(*.ttf *.otf *.ttc)")};
    m_kDragWidget->setNameFilters(list);

    m_stackedWid = new QStackedWidget(ui->fontContentFrame);
    m_stackedWid->setMinimumHeight(196);
    m_searchBlankPage = new BlankPage(ui->fontContentFrame);
    m_blankPage = new BlankPage(ui->fontContentFrame);

    m_fontUnitFrame = new QFrame(ui->fontContentFrame);
    m_fontUnitFrame->setContentsMargins(0, 8, 0, 0);
    m_flowLayout = new FlowLayout(ui->fontContentFrame, 0, 8, 8);
    m_installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + "/";

    loadFontdata();

    m_fontUnitFrame->setLayout(m_flowLayout);
    m_stackedWid->addWidget(m_blankPage);
    m_stackedWid->addWidget(m_fontUnitFrame);
    m_stackedWid->addWidget(m_searchBlankPage);
    m_stackedWid->setCurrentIndex(1);

    m_fontContentLayout = new QVBoxLayout(ui->fontContentFrame);
    m_fontContentLayout->setContentsMargins(16, 16, 16, 16);
    m_fontContentLayout->addWidget(m_kDragWidget);
    m_fontContentLayout->addWidget(m_stackedWid, 1);

    m_pNormalTip = new KBallonTip(pluginWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pNormalTip, name(), "", "");
    m_pNormalTip->setWindowFlag(Qt::Tool);
    m_pNormalTip->setWindowFlag(Qt::FramelessWindowHint);
    m_pNormalTip->setAttribute(Qt::WA_TranslucentBackground);
    m_pNormalTip->setTipType(TipType::Normal);
    m_pNormalTip->setWindowModality(Qt::WindowModal);
    m_pNormalTip->setWindowModality(Qt::ApplicationModal);
    m_pNormalTip->setAutoFillBackground(true);
    m_pNormalTip->setBackgroundRole(QPalette::Base);
    m_pNormalTip->setContentsMargins(16, 16, 16, 16);  //设置内容边距

    m_progressDialog = new KProgressDialog(tr("Installing Font..."), "", 0, 100, pluginWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_progressDialog, name(), "", "");
    m_progressDialog->setFixedSize(376, 93);
    m_progressDialog->setBarType(BarType::ScrollBar);

    const QByteArray styleID(STYLE_FONT_SCHEMA);
    m_styleSettings = new QGSettings(styleID, QByteArray(), this);
    const QByteArray ukccID(UKCC_SCHEMA);
    m_ukccSettings = new QGSettings(ukccID, QByteArray(), this);
    if (m_ukccSettings->keys().contains(FONT_DEFAULT_PATH)) {
        QString fontDialogPath = m_ukccSettings->get(FONT_DEFAULT_PATH).toString();
        QFileInfo fileInfo(fontDialogPath);
        if (fileInfo.isFile()) {
            fontDialogPath = fileInfo.absolutePath();
        }
        qDebug() << "path directory" << fontDialogPath;
        m_kDragWidget->fileDialog()->setDirectory(fontDialogPath);
    }

    const QByteArray id(INTERFACE_SCHEMA);
    m_ifsettings = new QGSettings(id, QByteArray(), this);

    const QByteArray gnomeID(GNOME_SCHEMA);
    m_gnomeSettings = new QGSettings(gnomeID, QByteArray(), this);

    pluginWidget->installEventFilter(this);

    fontSizeLabel->setAccessibleName(tr("Font size"));
    fontSizeLabel->setAccessibleDescription(QString());
    fontSizeLabel->setBuddy(m_fontSizeSlider);
    // 覆盖 KDK 英文物件名/描述；字号数值见 updateFontSizeSliderAccessible
    updateFontSizeSliderAccessible();
    monoTitle1->setAccessibleName(tr("Mono Font"));
    monoTitle1->setAccessibleDescription(QString());
    monoTitle1->setBuddy(m_monoCombox);
    m_monoCombox->setAccessibleName(tr("Mono Font"));
    m_monoCombox->setAccessibleDescription(QString());
    // 左侧说明 setBuddy 到按钮；须覆盖 KDK 英文 accessibleName，否则读屏念物件路径
    defaultLabel->setAccessibleName(
        tr("Restore font-related settings to the system's initial state"));
    defaultLabel->setAccessibleDescription(QString());
    defaultLabel->setBuddy(m_fontDefaultButton);
    // 覆盖 KDK 英文 accessibleName/Description
    m_fontDefaultButton->setAccessibleName(tr("Default"));
    m_fontDefaultButton->setAccessibleDescription(QString());
    m_searchArea->setAccessibleName(m_searchArea->placeholderText());
    m_searchArea->setAccessibleDescription(QString());
    // KSearchLineEdit 默认为 ClickFocus，需 StrongFocus 才能 Tab 进入
    m_searchArea->setFocusPolicy(Qt::StrongFocus);
    if (KPushButton *addBtn = m_kDragWidget->iconButton()) {
        addBtn->setFocusPolicy(Qt::StrongFocus);
        // 覆盖 KDK 物件名，读屏报添加说明而非内部类名
        addBtn->setAccessibleName(tr("Drag the font here or click the \"+\" to add it"));
        addBtn->setAccessibleDescription(QString());
    }
}

void Fonts::updateDragWidgetState() {
    if (!m_kDragWidget) {
        return;
    }

    const bool enabled = pluginWidget && pluginWidget->isEnabled();
    qDebug() << Q_FUNC_INFO << "pluginWidget enable" << enabled;
    m_kDragWidget->setEnabled(enabled);
    m_kDragWidget->setAttribute(Qt::WA_TransparentForMouseEvents, !enabled);
    if (m_kDragWidget->iconButton()) {
        m_kDragWidget->iconButton()->setEnabled(enabled);
        m_kDragWidget->iconButton()->setAttribute(Qt::WA_TransparentForMouseEvents, !enabled);
    }
}

void Fonts::comboxTextChangedSlot(const QString& text)
{
    QVariant monoName = QVariant(QString("%1 %2").arg(text).arg(m_monospacefontStrList.at(1)));
    m_ifsettings->set(MONOSPACE_FONT_KEY, monoName);
    m_gnomeSettings->set(MONOSPACE_FONT_KEY, monoName);
    UkccCommon::buriedSettings("Font", text, QString("select"), monoName.toString());
}

void Fonts::monoSettingChangeSlot(const QString &key)
{
    if ("monospaceFontName" == key) {
        QString str = m_ifsettings->get(MONOSPACE_FONT_KEY).toString();
        m_monoCombox->setCurrentText(str.mid(0 , str.size() - 3));
    }
}

void Fonts::setupConnect(){
    connectToServer();

    connect(m_ifsettings , &QGSettings::changed, this, [=](const QString &key) {
        monoSettingChangeSlot(key);
    });

    connect(m_fontSizeSlider, &KSlider::valueChanged, this, [=](int) {
        updateFontSizeSliderAccessible();
        // 键盘调节无 pressed/released，松键前 isSliderDown 为 false，需在此落盘
        if (!m_fontSizeSlider->isSliderDown()) {
            fontValueChangeSlot();
        }
    });

    connect(m_fontSizeSlider, &KSlider::sliderReleased, [=]() {
        fontValueChangeSlot();
    });

    connect(m_monoCombox, &QComboBox::currentTextChanged, [=](QString text) {
        comboxTextChangedSlot(text);
    });

    connect(m_searchArea, &KSearchLineEdit::textChanged, [=](QString search) {
        searchTextChangeSlot(search);
    });

    connect(m_kDragWidget, &KDragWidget::getPath, [=](QString path){
        dragWidgetGetPathSlot(path);
    });

    connect(m_kDragWidget, &KDragWidget::getPathList, [=](QStringList pathList){
        dragWidgetGetPathListSlot(pathList);
    });

    // 监听系统字体的变化
    connect(m_styleSettings , &QGSettings::changed, this, [=](const QString &key) {
        styleSettingChangedSlot(key);
    });

    // 重置按钮
    connect(m_fontDefaultButton, &QPushButton::clicked, [=](bool checked){
        resetDefaultSlot();
    });
}

void Fonts::addItemSlot(FontUnit* unit)
{
    connect(unit, &FontUnit::remoteFontSig, this, [=](QStringList pathList, QString name) {
        updateFontAfterRemoveSlot(pathList, name);
    });
    m_fontUnitList.append(unit);
}

void Fonts::styleSettingChangedSlot(const QString &key) {
    if(SYSTEM_FONT_KEY == key || SYSTEM_FONT_SIZE_KEY == key) {
        int value = fontConvertToSlider(m_styleSettings->get(SYSTEM_FONT_SIZE_KEY).toFloat());
        m_fontSizeSlider->blockSignals(true);
        m_fontSizeSlider->setValue(value);
        m_fontSizeSlider->blockSignals(false);
        updateFontSizeSliderAccessible();
        qDebug() << Q_FUNC_INFO << "font size change" << key << value;
    }
    if (key == "styleName") {
        m_monoTitleDesc->setFontColor(ThemeController::getCustomColorFromDT("kfont-secondary"));
    }
}

void Fonts::dragWidgetGetPathListSlot(QStringList pathList) {
    qDebug()<< "KDragWidget select file path.size" << pathList.size();
    checkFontFile(pathList);
    UkccCommon::buriedSettings(name(), "KDragWidget", QString("select file"));
    if (m_ukccSettings->keys().contains(FONT_DEFAULT_PATH)) {
        int size = m_kDragWidget->fileDialog()->selectedFiles().size();
        if (size <= 0) {
            return;
        }
        QString curPath = m_kDragWidget->fileDialog()->selectedFiles().first();
        if (curPath != "") {
            m_ukccSettings->set(FONT_DEFAULT_PATH, curPath);
        }
    }
}

void Fonts::dragWidgetGetPathSlot(QString path) {
    qDebug()<< "KDragWidget select file path" << path;
    QStringList pathList = {path};
    checkFontFile(pathList);
    UkccCommon::buriedSettings(name(), "KDragWidget", QString("select file"));
    if (m_ukccSettings->keys().contains(FONT_DEFAULT_PATH)) {
        int size = m_kDragWidget->fileDialog()->selectedFiles().size();
        if (size <= 0) {
            return;
        }
        QString curPath = m_kDragWidget->fileDialog()->selectedFiles().first();
        if (curPath != "") {
            m_ukccSettings->set(FONT_DEFAULT_PATH, curPath);
        }
    }
}

void Fonts::searchTextChangeSlot(QString search) {
    qDebug() << Q_FUNC_INFO << "searchText" << search;

    int showNum = 0;
    for (auto& unit : m_fontUnitList) {
        if (!unit) {
            continue;
        }
        ViewData *viewData = unit->getViewData();
        QString fontInfo = viewData->name + " " + viewData->style;
        // 中文环境额外支持按 Fontconfig 本地化家族名及其拼音搜索。
        if (QLocale::system().language() == QLocale::Chinese) {
            fontInfo += " " + viewData->family + " "
                    + removePinyinTone(Chinese2Pinyin(viewData->family));
        }
        if (fontInfo.contains(QRegExp(search, Qt::CaseInsensitive, QRegExp::FixedString))) {
            unit->setHidden(false);
            ++showNum;
            m_flowLayout->addWidget(unit);
            m_flowLayout->update();
        } else {
            unit->setHidden(true);
            m_flowLayout->removeWidget(unit);
        }
    }
    qDebug() << Q_FUNC_INFO << "search" << showNum << "showNum" << showNum;
    if (showNum <= 0) {
        if (search.isEmpty()) {
            showBlankPageSlot();
        } else {
            showSearchBlankPageSlot();
        }
    } else {
        showFontPageSlot();
    }
}

void Fonts::updateFontSizeSliderAccessible()
{
    if (!m_fontSizeSlider) {
        return;
    }
    const float size = sliderConvertToSize(m_fontSizeSlider->value());
    // 滑块 AT-SPI value 仍是档位 1–4；把实际 pt 写入 description 供读屏播报
    const QString sizeText = qFuzzyCompare(size, float(qRound(size)))
            ? QString::number(int(qRound(size)))
            : QString::number(double(size), 'f', 1);
    m_fontSizeSlider->setAccessibleName(tr("Font size"));
    m_fontSizeSlider->setAccessibleDescription(sizeText);
    QAccessibleValueChangeEvent ev(m_fontSizeSlider, QVariant(size));
    QAccessible::updateAccessibility(&ev);
}

void Fonts::fontValueChangeSlot() {
    int value = m_fontSizeSlider->value();
    qDebug() << Q_FUNC_INFO << "sliderReleased value" << value;
    float size = sliderConvertToSize(value);
    updateFontSizeSliderAccessible();
    m_fontInterface->call("setFontSize", double(size));
    UkccCommon::buriedSettings(name(), "font size change", QString("settings"), QString::number(size, 'f', 1));
}

void Fonts::resetDefaultSlot() {
    m_fontInterface->call("resetDefault");
    initFontStatus();
    UkccCommon::buriedSettings(name(), "reset default font", QString("clicked"));
}

void Fonts::deleteAllWidgets(QLayout* layout)
{
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->layout()) {
            deleteAllWidgets(item->layout());
        } else if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void Fonts::initFontUnitFrame()
{
    m_allFontData.clear();
    QDBusReply<ViewDataList> viewDataReply = m_fontInterface->call("getViewData");
    if (viewDataReply.isValid()) {
        m_allFontData = viewDataReply.value();
        qDebug() << Q_FUNC_INFO << "get viewData size" << m_allFontData.size();
    }
}

QStringList Fonts::splitFontNameSize(QString value) {
    QStringList valueStringList;
    QString str = value.right(1);
    QString ch0 = static_cast<QString>('0');
    QString ch9 = static_cast<QString>('9');
    if (str >= ch0 && str <= ch9) {
        QStringList tmpStringList = value.split(' ');
        QString::SectionFlag flag = QString::SectionSkipEmpty;
        valueStringList << value.section(' ', 0, tmpStringList.length() - 2, flag);
        valueStringList << value.section(' ', tmpStringList.length() - 1, tmpStringList.length() - 1, flag);
    } else {
        valueStringList << value << "10";
    }
    return valueStringList;
}

void Fonts::initFontStatus() {
    // 字体大小
    QDBusReply<double> res = m_fontInterface->call("getFontSize");
    if (res.isValid()) {
        double fontSize = res.value();
        int size = fontConvertToSlider(float(fontSize));
        m_fontSizeSlider->blockSignals(true);
        m_fontSizeSlider->setValue(size);
        m_fontSizeSlider->blockSignals(false);
        updateFontSizeSliderAccessible();
        qDebug() << Q_FUNC_INFO << "setFontSize" << size << fontSize;
    }


    m_monospacefontStrList = splitFontNameSize(m_ifsettings->get(MONOSPACE_FONT_KEY).toString());
    qDebug() << "initFontStatus list" << m_monospacefontStrList << m_monospacefontStrList[0];
    QString currentmonofont = m_monospacefontStrList[0];
    if ("DejaVu sans Mono" == currentmonofont) {
        currentmonofont = "DejaVu Sans Mono";
    }
    m_monoCombox->blockSignals(true);
    m_monoCombox->setCurrentText(currentmonofont);
    m_monoCombox->blockSignals(false);

    m_flowLayout->update();
}

int Fonts::fontConvertToSlider(float size) const {
    if (size == 10.0) {
        return 1;
    } else if (size == 11.0) {
        return 1;
    } else if (size == 12.0) {
        return 2;
    } else if (size == 13.0) {
        return 2;
    } else if (size == 13.5) {
        return 3;
    } else if (size == 14.0) {
        return 3;
    } else if (size >= 15.0) {
        return 4;
    } else {
        return 1;
    }
}

float Fonts::sliderConvertToSize(int value) const
{
    switch (value) {
    case 1:
        return 10;
        break;
    case 2:
        return 12;
        break;
    case 3:
        return 13.5;
        break;
    case 4:
        return 15;
        break;
    default:
        return 10;
        break;
    }
}

void Fonts::connectToServer(){
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&Fonts::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

void Fonts::keyChangedSlot(const QString &key) {
    if(key == "font") {
        initFontStatus();
    }
}

bool Fonts::getFontFile(QString fileName)
{
    if (fileName.isEmpty()) {
        return false;
    }
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        if (fileInfo.isFile()) {
            return true;
        }
    }
    return false;
}

void Fonts::clickFontFile() {
    if (!m_ukccSettings || !m_ukccSettings->keys().contains(FONT_SELECT_FILE)) {
        qDebug() << Q_FUNC_INFO << "m_ukccSettings get fontSelectFile failed";
        return;
    }
    QString paths = m_ukccSettings->get(FONT_SELECT_FILE).toString();
    if (paths.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "fontSelectFile is empty";
        return;
    }
    qDebug() << Q_FUNC_INFO << "select file" << paths;
    QStringList installFontList;
    installFontList = paths.split("/n/n");
    QStringList fontList;
    for (int i = 0; i < installFontList.size(); i++) {
        if (getFontFile(installFontList[i])) {
            fontList << installFontList[i];
        }
    }
    m_ukccSettings->reset(FONT_SELECT_FILE);
    if (!fontList.isEmpty()) {
        checkFontFile(installFontList);
        qDebug() << Q_FUNC_INFO << "install select file" << installFontList;
    }
}

void Fonts::showBlankPageSlot()
{
    qDebug() << Q_FUNC_INFO << "show blank page";
    m_stackedWid->setCurrentIndex(0);
    m_blankPage->changeBlankText();
    return;
}

void Fonts::showFontPageSlot()
{
    qDebug() << Q_FUNC_INFO << "show font page";
    m_stackedWid->setCurrentIndex(1);
    return;
}

void Fonts::showSearchBlankPageSlot() {
    qDebug() << Q_FUNC_INFO << "show search blank page";
    m_stackedWid->setCurrentIndex(2);
    m_searchBlankPage->changeSearchText();
}

void Fonts::checkFontFile(QStringList list)
{
    qDebug() << Q_FUNC_INFO << "checkFontFile size" << list;
    addFont(list);
}

bool Fonts::judgeBadFontFile(QString path)
{
    QString ret;
    std::string str = path.toStdString();
    FcChar8* file = (FcChar8*)(str.c_str());
    FcPattern* pat = nullptr;
    int count;
    FcBlanks* blanks = FcConfigGetBlanks(NULL);
    pat = FcFreeTypeQuery((FcChar8 *)file, 0, blanks, &count);

    // 判断是否从字体文件中获取到字体数据
    if (pat == nullptr) {
        qDebug() << "判断字体文件，字体文件损坏：" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        ret = path;
    }

    FcPatternDestroy(pat);

    return ret == path;
}

bool Fonts::chooseFontFile(QString path)
{
    QStringList list = path.split(".");
    QString fontTypeTTF = "ttf";
    QString fontTypeOTF = "otf";
    QString fontTypeTTC = "ttc";
    QString fontType = list.at(list.size()-1);

    if (fontType.compare(fontTypeTTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeOTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeTTC, Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool Fonts::judgeRepeatFontFile(QString path) {
    for (int i = 0; i < m_allFontData.size(); i++) {
        if (m_allFontData[i].path.contains(path)) {
            return true;
        }
    }
    return false;
}

void Fonts::addFont(QStringList fontList)
{
    QString repeatedFontNameStr;
    QString badFontNameStr;
    QVariantList needInstallList;
    QVariantList repeatedFontNameList;

    QDBusReply<ViewDataList> installViewDataReply = m_fontInterface->call("getInstallViewData");
    if (installViewDataReply.isValid()) {
        m_installViewData = installViewDataReply.value();
        qDebug() << Q_FUNC_INFO << "get installViewData size" << m_installViewData.size();
    } else {
        qDebug() << Q_FUNC_INFO << "get installViewData failed";
    }
    // 获取可以安装的字体文件
    for (int i = 0; i < fontList.size(); i++) {
        QString path = fontList[i];
        QFileInfo fileinfo;
        fileinfo = QFileInfo(path);
        QString fileName = fileinfo.fileName();

        if (!chooseFontFile(path)) {
            qDebug() << Q_FUNC_INFO << path << "is not font file type";
            continue;
        }
        if (judgeBadFontFile(path)) {
            badFontNameStr.append(fileName + "\n");
            qDebug() << Q_FUNC_INFO << path << "is bad file";
            continue;
        }
        if (judgeRepeatFontFile(fileName)) {
            repeatedFontNameStr.append(fileName + "\n");
            repeatedFontNameList.append(QVariant(path));
            qDebug() << Q_FUNC_INFO << fileName << "repeated file";
        } else {
            needInstallList.append(QVariant(path));
        }
    }
    qDebug() << Q_FUNC_INFO << badFontNameStr << repeatedFontNameStr << needInstallList.size();
    // 重复字体名称
    if (!repeatedFontNameStr.isEmpty()) {
        QMessageBox msg(pluginWidget);
        msg.setIcon(QMessageBox::Warning);
        msg.addButton(tr("Skip"), QMessageBox::NoRole);
        msg.addButton(tr("Replace"), QMessageBox::YesRole);
        msg.setText(tr("The font with this name is already installed"));
        msg.setInformativeText(QString("%1").arg(repeatedFontNameStr));
        int ret = msg.exec();
        if (ret) {
            needInstallList << repeatedFontNameList;
            qDebug() << Q_FUNC_INFO << repeatedFontNameStr << "need new install";
        }
    }
    // 无法安装字体
    if (!badFontNameStr.isEmpty()) {
        QMessageBox msg(pluginWidget);
        msg.setIcon(QMessageBox::Critical);
        msg.addButton(tr("Confirm"), QMessageBox::ApplyRole);
        msg.setText(tr("The font file is damaged and cannot be installed."));
        msg.setInformativeText(QString("%1").arg(badFontNameStr));
        int ret = msg.exec();
        if (ret) {
            qDebug() << Q_FUNC_INFO << badFontNameStr << "no need install";
        }
    }

    if (needInstallList.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "need install font file is empty";
        return;
    }
    QString installFontPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + FONT_PATH;
    // 请求dbus安装新字体列表
    QDBusReply<bool> res = m_fontInterface->call("installFontList", needInstallList);
    m_installSucSize = 0;
    if (res.isValid() && res.value()) {
        for (int i = 0; i < needInstallList.size(); i++) {
            QString path = needInstallList.at(i).toString();
            QFileInfo fileinfo;
            fileinfo = QFileInfo(path);
            QString fileName = fileinfo.fileName();
            QString filePath = installFontPath + fileName;
            QFontDatabase::addApplicationFont(filePath);
            ++m_installSucSize;
        }
    } else {
        qDebug() << Q_FUNC_INFO << "installFontList dbus failed" << res.isValid() << res.value();
    }
    if (m_installSucSize <= 0) {
        qDebug() << Q_FUNC_INFO << "no install suc font file";
        return;
    }
    // 安装进度弹窗
    QTimer::singleShot(1000, this, [=]() {
        m_progressDialog->close();
        m_pNormalTip->setText(QString(tr("already installed %1 fonts!")).arg(m_installSucSize));
        m_pNormalTip->setTipTime(2000);
        m_pNormalTip->showInfo();
    });
    m_progressDialog->exec();
    updateFontList();
    //更新UI
    QList<QString> installFontFamilyList;
    for (int i = 0; i < needInstallList.size(); i++) {
        QString path = needInstallList.at(i).toString();
        QFileInfo fileinfo;
        fileinfo = QFileInfo(path);
        QString fileName = fileinfo.fileName();
        QString filePath = installFontPath + fileName;
        for (int j = 0; j < m_installViewData.size(); ++j) {
            if (filePath == m_installViewData[j].path || m_installViewData[j].pathList.contains(filePath)) {
                QString fontFamily = m_installViewData[j].family;
                if (installFontFamilyList.contains(fontFamily)) {
                    qDebug() << Q_FUNC_INFO << "already install font" << filePath;
                    continue;
                }
                installFontFamilyList.append(fontFamily);
                bool hitFontUnit = false;
                FontUnit* oldUnit = nullptr;
                for (auto& unit : m_fontUnitList) {
                    QString unitFontFamily = unit->getViewData()->family;
                    if (fontFamily == unitFontFamily) {
                        hitFontUnit = true;
                        oldUnit = unit;
                        m_fontUnitList.removeAll(oldUnit);
                        qDebug() << Q_FUNC_INFO << "install font already create fontUnit" << filePath << fontFamily;
                        break;
                    }
                }
                if (hitFontUnit && oldUnit) {  // 安装后是否需要显示当前样式
                    if (filePath != m_installViewData[j].path) {
                        if (m_installViewData[j].pathList.contains(filePath)) {
                            int index = m_installViewData[j].pathList.indexOf(filePath);
                            m_installViewData[j].path = filePath;
                            m_installViewData[j].style = m_installViewData[j].styleList.at(index);
                        }
                    }
                    oldUnit->updateViewData(&m_installViewData[j]);
                }
                FontUnit* unit = oldUnit;
                if (!hitFontUnit) {
                    unit = new FontUnit(&m_installViewData[j], m_installPath, m_fontUnitFrame);
                    if (!unit) {
                        qWarning() << Q_FUNC_INFO << "new FontUnit failed";
                        continue;
                    }
                    connect(unit, &FontUnit::remoteFontSig, this, [=](QStringList pathList, QString name) {
                        updateFontAfterRemoveSlot(pathList, name);
                    });
                }
                unit->setInstallFlag(true);
                m_fontUnitList.insert(0, unit);
                insertWidgetAt(unit, 0);
                m_flowLayout->update();
                qDebug() << Q_FUNC_INFO << "update flow layout suc" << m_installViewData[j].path;
                break;
            }
        }
    }
}

void Fonts::updateFontList()
{
    QDBusReply<ViewDataList> viewDataReply = m_fontInterface->call("getViewData");
    if (viewDataReply.isValid()) {
        m_allFontData.clear();
        m_allFontData = viewDataReply.value();
        qDebug() << Q_FUNC_INFO << "get viewData size" << m_allFontData.size();
    }
    QDBusReply<ViewDataList> installViewDataReply = m_fontInterface->call("getInstallViewData");
    if (installViewDataReply.isValid()) {
        m_installViewData.clear();
        m_installViewData = installViewDataReply.value();
        qDebug() << Q_FUNC_INFO << "get installViewData size" << m_installViewData.size();
    } else {
        qDebug() << Q_FUNC_INFO << "get installViewData failed";
    }
}

void Fonts::updateFontAfterRemoveSlot(QStringList pathList, QString name)
{
    updateFontList();
    for (int idx = 0; idx < m_fontUnitList.size(); ++idx) {
        if (!m_fontUnitList[idx]) {
            qDebug() << Q_FUNC_INFO << "remove fontunit" << idx;
            m_fontUnitList.removeAt(idx);
            break;
        } else {
            ViewData* viewData = m_fontUnitList[idx]->getViewData();
            if (viewData->pathList.contains(pathList.at(0)) && (viewData->pathList.size() == pathList.size())) {
                m_fontUnitList.removeAt(idx);
                qDebug() << Q_FUNC_INFO << "remove fontunit" << idx;
                break;
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "updateFontAfterRemoveSlot" << pathList << name;
}

void Fonts::loadFontdata()
{
    QDBusReply<ViewDataList> viewDataReply = m_fontInterface->call("getViewData");
    if (viewDataReply.isValid()) {
        m_allFontData = viewDataReply.value();
        qDebug() << Q_FUNC_INFO << "get viewData size" << m_allFontData.size();
    }
    m_fontUnitList.clear();

    int initSize = 30;
    for (int idx = 0; idx < m_allFontData.size() &&(idx < initSize); ++idx) {
        QMetaObject::invokeMethod(this, [=]() {
            qDebug() << Q_FUNC_INFO << "font path list.size" << m_allFontData.at(idx).pathList.size();
            FontUnit* unit = new FontUnit(&m_allFontData.at(idx), m_installPath);
            connect(unit, &FontUnit::remoteFontSig, this, [=](QStringList pathList, QString name) {
                updateFontAfterRemoveSlot(pathList, name);
            });
            m_fontUnitList.append(unit);
            m_flowLayout->addWidget(unit);
        });
    }

    QTimer::singleShot(10, this, [this, initSize]() {
        loadBatch(initSize);
    });
}

void Fonts::loadBatch(int initSize) {
    for (int idx = initSize; idx < m_allFontData.size(); ++idx) {
        FontUnit* unit = new FontUnit(&m_allFontData.at(idx), m_installPath);
        connect(unit, &FontUnit::remoteFontSig, this, [=](QStringList pathList, QString name) {
            updateFontAfterRemoveSlot(pathList, name);
        });
        m_fontUnitList.append(unit);
        m_flowLayout->addWidget(unit);
    }
}
