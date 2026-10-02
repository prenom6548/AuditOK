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
#include "wallpaperui.h"
#include "ukcccommon.h"
#include "kbackgroundgroup.h"
#include <QScrollBar>
#include <QStackedWidget>
#include <QImageReader>
#include <QFile>
#include <QCoreApplication>
#include <QTimer>
#include <kysdk/applications/accessinfohelper.h>

using namespace ukcc;
using namespace kdk;

// 加载图片（支持 .dib 格式）
static QPixmap loadPixmapWithDibSupport(const QString &filename)
{
    QPixmap pixmap(filename);

    // 如果加载失败且是 .dib 格式，尝试用 BMP 格式读取
    if (pixmap.isNull() && filename.toLower().endsWith(".dib")) {
        QImageReader reader(filename, QByteArrayLiteral("bmp"));
        if (reader.canRead()) {
            QImage image = reader.read();
            if (!image.isNull()) {
                pixmap = QPixmap::fromImage(image);
            }
        }
    }

    // 如果仍然失败，尝试读取文件数据
    if (pixmap.isNull()) {
        QFile file(filename);
        if (file.size() <= 100000000 && file.open(QIODevice::ReadOnly)) {
            pixmap.loadFromData(file.readAll());
            file.close();
        }
    }

    return pixmap;
}

WallpaperUi::WallpaperUi()
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_uiLayout = new QVBoxLayout(this);
    m_uiLayout->setMargin(0);
    createObject();
    initUi();
    initColors();
    initSignals();

    m_stackedWidget->setCurrentIndex(1);
}

WallpaperUi::~WallpaperUi()
{

}

void WallpaperUi::createObject()
{
    m_wallpaperTitleLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_wallpaperTitleLabel, "Wallpaper", "", "");
    m_wallpaperTitleLabel->setContentsMargins(16, 0, 0, 0);
    m_previewFrame = new UkccFrame(this);
    m_previewFrame->setFixedHeight(193);
    m_previewLabel = new PreviewLabel(m_previewFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_previewLabel, "Wallpaper", "", "preview label of wallpaper");
    m_settingGroup = new SettingGroup(this);
    m_colorGroup = new SettingGroup(m_settingGroup);
    // 先建颜色容器；「添加」按钮在 initColors 末尾创建，保证 Tab：颜色块 → 添加
    m_colorFrame = new UkccFrame(m_colorGroup);
    m_colorLayout = new FlowLayout(m_colorFrame, 16, -1, -1);
    m_addbuttonFrame = new UkccFrame(m_colorGroup);
    m_addbuttonlayout = new QHBoxLayout(m_addbuttonFrame);
    m_addbuttonlayout->setMargin(0);
    m_picBtnFrame = new UkccFrame(m_settingGroup);
    m_picFrame = new UkccFrame(m_picBtnFrame);
    m_picFrame->setLineWidth(0);
    m_picFrame->setMaximumHeight(1074);
    m_pictureLayout = new FlowLayout(m_picFrame, 8, 8, 8);
    m_pictureLayout->setContentsMargins(16, 0, 8, 0);
    m_bottomFrame = new UkccFrame(m_picBtnFrame);
    m_bottomFrame->setFixedHeight(60);
    // m_stackedWidget 在 initUi 中于预览区之后创建，避免焦点链插在组合框之前
}

void WallpaperUi::initUi()
{
    // 1 桌面背景
    //~ contents_path /Wallpaper/Desktop Background
    m_wallpaperTitleLabel->setText(tr("Desktop Background"));

    // 2.1 背景
    m_modeWidget = new QWidget();
    QHBoxLayout* modeLayout = new QHBoxLayout(m_modeWidget);
    modeLayout->setContentsMargins(16, 0, 16, 0);
    modeLayout->setSpacing(0);
    //~ contents_path /Wallpaper/Background
    QLabel* modeLabel = new QLabel(tr("Background"), m_modeWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(modeLabel, "Wallpaper", "", "");
    m_modeComboBox = new QComboBox(m_modeWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_modeComboBox, "Wallpaper", "", "");
    m_modeComboBox->setFixedWidth(240);
    modeLayout->addWidget(modeLabel);
    modeLayout->addStretch();
    modeLayout->addWidget(m_modeComboBox);

    // 2.2 显示方式
    m_typeWidget = new QWidget();
    QHBoxLayout* typeLayout = new QHBoxLayout(m_typeWidget);
    typeLayout->setContentsMargins(16, 0, 16, 0);
    typeLayout->setSpacing(0);
    //~ contents_path /Wallpaper/Type
    QLabel* typeLabel = new QLabel(tr("Type"), m_typeWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(typeLabel, "Wallpaper", "", "");
    m_typeComboBox = new QComboBox(m_typeWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_typeComboBox, "Wallpaper", "", "");
    m_typeComboBox->setFixedWidth(240);
    typeLayout->addWidget(typeLabel);
    typeLayout->addStretch();
    typeLayout->addWidget(m_typeComboBox);

    KBackgroundGroup* content = new KBackgroundGroup(m_previewFrame);
    content->setBorderVisible(true);
    content->addWidget(m_modeWidget);
    content->addWidget(m_typeWidget);

    QVBoxLayout *previewVLayout = new QVBoxLayout();
    previewVLayout->setContentsMargins(8, 0, 0, 0);
    previewVLayout->setSpacing(0);
    previewVLayout->addWidget(content);
    previewVLayout->addStretch();

    QHBoxLayout* previewHLayout = new QHBoxLayout(m_previewFrame);
    previewHLayout->setContentsMargins(16, 12, 16, 16);
    previewHLayout->addWidget(m_previewLabel);
    previewHLayout->addSpacing(0);
    previewHLayout->addLayout(previewVLayout);

    // 预览区域滚动条（仅承载布局，不参与 Tab；须带父对象且早于 stacked 创建，保证焦点链在「本地图片」之前）
    QScrollArea *previewScrollArea = new QScrollArea(this);
    previewScrollArea->setFocusPolicy(Qt::NoFocus);
    previewScrollArea->viewport()->setFocusPolicy(Qt::NoFocus);
    previewScrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    previewScrollArea->verticalScrollBar()->setEnabled(false);
    previewScrollArea->verticalScrollBar()->setVisible(false);
    previewScrollArea->horizontalScrollBar()->setEnabled(false);
    previewScrollArea->setWidgetResizable(true);
    previewScrollArea->setWidget(m_previewFrame);
    previewScrollArea->setFixedHeight(192);

    // 须晚于预览区创建：Qt 按创建/重父级插入焦点链，过早创建会导致「本地图片」先于「背景/显示方式」
    m_stackedWidget = new QStackedWidget(this);

    // 3.1 选择图片
    //~ contents_path /Wallpaper/Choose Picture
    QLabel* chooseLabel = new QLabel(tr("Choose Picture"), m_bottomFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(chooseLabel, "Wallpaper", "", "");

    // 3.2 本地图片
    //~ contents_path /Wallpaper/Local Pictures
    m_localBtn = new KPushButton(m_bottomFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_localBtn, "Wallpaper", "", "");
    m_localBtn->setText(tr("Local Pictures"));

    // 3.3 在线图片
    //~ contents_path /Wallpaper/Online Pictures
    m_onlineBtn = new KPushButton(m_bottomFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_onlineBtn, "Wallpaper", "", "");
    m_onlineBtn->setText(tr("Online Pictures"));

    // 3.4 恢复默认
    //~ contents_path /Wallpaper/Reset To Default
    m_resetBtn  = new KPushButton(m_bottomFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_resetBtn, "Wallpaper", "", "");
    m_resetBtn->setText(tr("Reset To Default"));

    QHBoxLayout *bottomLayout = new QHBoxLayout(m_bottomFrame);
    bottomLayout->setContentsMargins(16, 8, 16, 8);
    bottomLayout->addWidget(chooseLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_localBtn);
    bottomLayout->addWidget(m_onlineBtn);
    bottomLayout->addWidget(m_resetBtn);

    QVBoxLayout *picBtnLayout = new QVBoxLayout(m_picBtnFrame);
    picBtnLayout->setMargin(0);
    picBtnLayout->setSpacing(0);
    picBtnLayout->addWidget(m_bottomFrame);
    picBtnLayout->addWidget(m_picFrame);

    m_colorGroup->addWidget(m_colorFrame);
    m_colorGroup->addWidget(m_addbuttonFrame);

    // 颜色/图片列表滚动区同样不参与 Tab
    m_colorScrollArea = new QScrollArea;
    m_colorScrollArea->setFocusPolicy(Qt::NoFocus);
    m_colorScrollArea->viewport()->setFocusPolicy(Qt::NoFocus);
    m_colorScrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    m_colorScrollArea->setStyleSheet("QScrollArea{background-color : transparent;}");
    m_colorScrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_colorScrollArea->verticalScrollBar()->setEnabled(false);
    m_colorScrollArea->horizontalScrollBar()->setEnabled(false);
    m_colorScrollArea->setWidgetResizable(true);
    m_colorScrollArea->setWidget(m_colorGroup);
    m_colorScrollArea->setMinimumHeight(212);

    m_contentScrollArea = new QScrollArea;
    m_contentScrollArea->setFocusPolicy(Qt::NoFocus);
    m_contentScrollArea->viewport()->setFocusPolicy(Qt::NoFocus);
    m_contentScrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_contentScrollArea->horizontalScrollBar()->setEnabled(false);
    m_contentScrollArea->setWidgetResizable(true);
    m_contentScrollArea->setFrameShape(QFrame::StyledPanel);
    m_contentScrollArea->setFrameShadow(QFrame::Plain);
    m_contentScrollArea->setWidget(m_settingGroup);

    QWidget* colorWidget = new QWidget();
    QVBoxLayout* colorLayout = new QVBoxLayout(colorWidget);
    colorLayout->setContentsMargins(0, 0, 16, 0);  // 调整colorFrame右边距

    QWidget* pictureWidget = new QWidget();
    QVBoxLayout* pictureLayout = new QVBoxLayout(pictureWidget);
    pictureLayout->setMargin(0);

    colorLayout->addWidget(m_colorScrollArea);
    pictureLayout->addWidget(m_contentScrollArea);

    // 将颜色区域和图片区域添加到stackwidget中
    m_stackedWidget->addWidget(colorWidget);
    m_stackedWidget->addWidget(pictureWidget);

    // 重新组织m_settingGroup，只包含图片相关的内容
    m_settingGroup->addWidget(m_picBtnFrame);

    m_uiLayout->addWidget(m_wallpaperTitleLabel);
    m_uiLayout->addWidget(previewScrollArea);
    m_uiLayout->addWidget(m_stackedWidget);

    modeLabel->setAccessibleName(tr("Background"));
    modeLabel->setBuddy(m_modeComboBox);
    m_modeComboBox->setAccessibleName(tr("Background"));
    typeLabel->setAccessibleName(tr("Type"));
    typeLabel->setBuddy(m_typeComboBox);
    m_typeComboBox->setAccessibleName(tr("Type"));
    m_localBtn->setAccessibleName(tr("Local Pictures"));
    m_onlineBtn->setAccessibleName(tr("Online Pictures"));
    m_resetBtn->setAccessibleName(tr("Reset To Default"));

    connect(m_modeComboBox, QOverload<const QString&>::of(&QComboBox::activated), this, [=](const QString &mode) {
        modeComboBoxSlot();
    });
}

void WallpaperUi::initColors()
{
    QStringList colors;
    colors << "#2d7d9a" << "#018574" << "#107c10" << "#10893e" << "#038387" << "#486860" << "#525e54" << "#7e735f" << "#4c4a48" << "#000000";
    colors << "#ff8c00" << "#e81123" << "#d13438" << "#c30052" << "#bf0077" << "#9a0089" << "#881798" << "#744da9" << "#8764b8" << "#e9e9e9";

    QStringList colorsTr;
    colorsTr << tr("Blue cyan") << tr("Pine green") << tr("Emerald green") << tr("Green") << tr("Dark cyan") << tr("Slate green") << tr("Mineral green");
    colorsTr << tr("Taupe") << tr("Dark brown") << tr("Black") << tr("Aurantiacus") << tr("Red") << tr("Brick-red") << tr("Rose red") << tr("Purplish red");
    colorsTr << tr("Dark magenta") << tr("Purple") << tr("Violet") << tr("Medium purple") << tr("Grey");
    int index = 0;
    for (QString color : colors){
        PictureUnit *colorUnit = new PictureUnit(m_colorFrame, QSize(56, 56));
        colorUnit->setColor(QColor(color));
        colorUnit->setColorName(color);
        if (currentFile == color) {
            preColorUnit = colorUnit;
            colorUnit->changeClickedFlag(true);
        }
        QString tipColor;
        if (index < colorsTr.size()) {
            tipColor = colorsTr.at(index++);
        }
        colorUnit->setToolTip(tipColor);
        // KDK 后再设 accessibleName，读屏报颜色名而非物件名；setColor 已打开 StrongFocus
        KDK_EXTEND_ALL_INFO_FORMAT(colorUnit, "Wallpaper", "", "color unit " + color);
        colorUnit->setAccessibleName(tipColor);
        connect(colorUnit, &PictureUnit::clicked, [=, color]() {
            picButtonClickSlot(colorUnit, color);
        });
        m_colorLayout->addWidget(colorUnit);
    }

    // 颜色组仅保留一个 StrongFocus（当前选中或首项），Tab 一次进入，方向键切换
    if (preColorUnit) {
        preColorUnit->takeColorGroupTabStop();
    } else if (m_colorLayout->count() > 0) {
        if (PictureUnit *first = qobject_cast<PictureUnit *>(m_colorLayout->itemAt(0)->widget())) {
            first->takeColorGroupTabStop();
        }
    }

    // 颜色块全部建完后再建「添加」，焦点链与视觉顺序一致：颜色组 → 添加
    m_colorAddButton = new AddButton(m_addbuttonFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_colorAddButton, "Wallpaper", "", "");
    // KDK 会覆盖 accessibleName，按 AddButton 上下文恢复「添加」（libukcc 已有翻译）
    m_colorAddButton->setAccessibleName(QCoreApplication::translate("AddButton", "Add"));
    m_addbuttonlayout->addWidget(m_colorAddButton);
}

void WallpaperUi::initSignals()
{
    connect(m_typeComboBox, QOverload<const QString&>::of(&QComboBox::activated), this, [=](const QString &type){
        typeComboBoxSlot();
    });

    connect(m_localBtn, &QPushButton::clicked, this, [=](){
        localBtnClickSlot();
    });

    connect(m_onlineBtn, &QPushButton::clicked, this, [=](){
        onlineBtnClickSlot();
    });

    connect(m_resetBtn, &QPushButton::clicked, this, [=](){
        resetBtnClickSlot();
    });

    connect(m_colorAddButton, &AddButton::clicked, this, [=](){
        colorAddButtonSlot();
    });
}

void WallpaperUi::modeComboBoxSlot() {
    if (m_modeComboBox->currentData().toString() == "picture") {
        // 切换到图片视图
        m_stackedWidget->setCurrentIndex(1);
        showPictures();
        UkccCommon::buriedSettings("Wallpaper", QString("Background"), QString("select"), "picture");
    } else {
        // 切换到颜色视图
        m_stackedWidget->setCurrentIndex(0);
        showColors();
        UkccCommon::buriedSettings("Wallpaper", QString("Background"), QString("select"), "color");
    }
}

void WallpaperUi::picButtonClickSlot(PictureUnit* colorUnit, QString color) {
    if (preColorUnit && preColorUnit != colorUnit) {
        preColorUnit->changeClickedFlag(false);
    }
    preColorUnit = colorUnit;
    colorUnit->changeClickedFlag(true);
    // 选中项作为颜色组唯一 Tab 落点
    colorUnit->takeColorGroupTabStop();
    m_previewLabel->setColor(color);
    m_previewLabel->update();
    Q_EMIT wallpaperChanged(m_modeComboBox->currentData().toString(), color);
}

void WallpaperUi::resetBtnClickSlot() {
    m_focusSelectedPicAfterReset = true;
    Q_EMIT resetButtonClicked();
}

void WallpaperUi::onlineBtnClickSlot() {
    Q_EMIT onlineButtonClicked();
}

void WallpaperUi::localBtnClickSlot() {
    Q_EMIT localButtonClicked();
}

void WallpaperUi::typeComboBoxSlot() {
    m_previewLabel->setMode(m_typeComboBox->currentData().toString());
    m_previewLabel->update();
    Q_EMIT typeChanged(m_typeComboBox->currentData().toString());
}

void WallpaperUi::colorAddButtonSlot() {
    ColorDialog *colordialog = new ColorDialog(this);
    colordialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(colordialog, &ColorDialog::colorSelected, this, [=](QColor color){
        colorDialogSlot(color);
    });
    colordialog->exec();
}

void WallpaperUi::colorDialogSlot(QColor color) {
    m_previewLabel->setColor(color.name());
    m_previewLabel->update();
    Q_EMIT wallpaperChanged(m_modeComboBox->currentData().toString(), color.name());
}

void WallpaperUi::setWallpaperModes(const QStringList &modeList, const QStringList &modeDataList)
{
    if (modeList.size() != modeDataList.size()) {
        qWarning()<<"modeList.size():"<<modeList.size()<<" != modeDataList.size():"<<modeDataList.size();
        return;
    }
    m_modeComboBox->clear();
    for (int i = 0; i < modeList.size(); ++i) {
        m_modeComboBox->addItem(modeList.at(i), modeDataList.at(i));
    }
}

void WallpaperUi::setDisplayTypes(const QStringList &typeList, const QStringList &typeDataList)
{
    if (typeList.size() != typeDataList.size()) {
        qWarning()<<"typeList.size():"<<typeList.size()<<" != typeDataList.size():"<<typeDataList.size();
        return;
    }
    m_typeComboBox->clear();
    for (int i = 0; i < typeList.size(); ++i) {
        m_typeComboBox->addItem(typeList.at(i), typeDataList.at(i));
    }
}

void WallpaperUi::setModeWidgetVisible(bool visible) {
    if (m_modeComboBox->currentData().toString() == "color") {
        m_stackedWidget->setCurrentIndex(0);
        m_stackedWidget->setVisible(visible);
    } else {
        m_stackedWidget->setCurrentIndex(1);
        m_stackedWidget->setVisible(visible);
    }
}

void WallpaperUi::setWallpaperMode(const QString &mode, const QString &data)
{
    m_modeComboBox->setCurrentText(mode);
    if (m_modeComboBox->currentData().toString() == "color") {
        m_previewLabel->update();
        if (prePictureUnit != nullptr) {
            prePictureUnit->changeClickedFlag(false);
            prePictureUnit->setStyleSheet("border-width: 0px;");
            prePictureUnit = nullptr;
        }
        // 切换到颜色视图
        m_stackedWidget->setCurrentIndex(0);
        currentFile = data;
        QColor color(data);
        m_previewLabel->setColor(color);
        resetClickedColor();
    } else {
        if (preColorUnit != nullptr) {
            preColorUnit->changeClickedFlag(false);
            preColorUnit->setStyleSheet("border-width: 0px;");
            preColorUnit = nullptr;
        }
        // 切换到图片视图
        m_stackedWidget->setCurrentIndex(1);
        currentFile = data;
        m_previewLabel->setLocalPixmap(loadPixmapWithDibSupport(currentFile));
        m_previewLabel->update();
        resetClickedPic();
    }
}

void WallpaperUi::setDisplayType(const QString &type)
{
    m_typeComboBox->setCurrentText(type);
    m_previewLabel->setMode(m_typeComboBox->currentData().toString());
    m_previewLabel->update();
}

void WallpaperUi::setPicturesFile(QList<PictureInfo*> picInfoList) {
    bool hitCurrentFile = false;
    for (int i = 0; i < picInfoList.size(); ++i) {
        QString name = picInfoList.at(i)->fileName;
        bool clicked = false;
        if (name == currentFile) {
            clicked = true;
            hitCurrentFile = true;
        }
        PictureUnit *picUnit = new PictureUnit();
        // KDK 物件名各图相同；随后 setFilenameText 用文件名覆盖 accessibleName
        KDK_EXTEND_ALL_INFO_FORMAT(picUnit, "Wallpaper", "", "");
        if (picUnit == nullptr) {
            continue;
        }
        picUnit->setPixmap(picInfoList.at(i)->pixmap);
        picUnit->setFilenameText(name);
        picUnit->setBackground(picInfoList.at(i)->pixmap);
        createPictureUnit(picUnit, clicked);
    }
    // 是自定义的图片
    if (!currentFile.isEmpty() && !hitCurrentFile && QFile::exists(currentFile)) {
        m_previewLabel->setLocalPixmap(loadPixmapWithDibSupport(currentFile));
        m_previewLabel->update();
    }
    ensurePictureTabOrder();
}

void WallpaperUi::createPictureUnit(PictureUnit* picUnit, const bool &clicked) {
    if (picUnit == nullptr)
        return;
    const QString &filename = picUnit->filenameText();
    if (clicked) {
        prePictureUnit = picUnit;
        picUnit->changeClickedFlag(true);
        m_previewLabel->setLocalPixmap(loadPixmapWithDibSupport(filename));
        m_previewLabel->update();
    }
    connect(picUnit, &PictureUnit::clicked, [=](QString file){
        if (prePictureUnit) {
            prePictureUnit->changeClickedFlag(false);
        }
        prePictureUnit = picUnit;
        picUnit->changeClickedFlag(true);
        picUnit->takeGroupTabStop();
        ensurePictureTabOrder();
        m_previewLabel->setLocalPixmap(loadPixmapWithDibSupport(file));
        m_previewLabel->update();
        Q_EMIT wallpaperChanged("picture", file);
    });
    m_pictureLayout->addWidget(picUnit);
    if (clicked) {
        picUnit->takeGroupTabStop();
    }
}

void WallpaperUi::ensurePictureTabOrder()
{
    if (!m_resetBtn || !m_pictureLayout) {
        return;
    }

    PictureUnit *tabStop = prePictureUnit;
    if (!tabStop && m_pictureLayout->count() > 0) {
        tabStop = qobject_cast<PictureUnit *>(m_pictureLayout->itemAt(0)->widget());
    }
    if (!tabStop) {
        return;
    }

    tabStop->takeGroupTabStop();
    // 缩略图创建早于底部按钮，默认焦点链在「恢复默认」之前；显式接到按钮之后
    if (m_localBtn && m_onlineBtn) {
        QWidget::setTabOrder(m_localBtn, m_onlineBtn);
        QWidget::setTabOrder(m_onlineBtn, m_resetBtn);
    }
    QWidget::setTabOrder(m_resetBtn, tabStop);
}

void WallpaperUi::showPictures()
{
    m_colorGroup->setHidden(true);
    m_typeWidget->setHidden(false);
    m_picFrame->setHidden(false);
    m_bottomFrame->setHidden(false);
    ensurePictureTabOrder();
}

void WallpaperUi::showColors()
{
    m_picFrame->setHidden(true);
    m_typeWidget->setHidden(true);
    m_bottomFrame->setHidden(true);
    m_colorGroup->setHidden(false);
}

void WallpaperUi::resetClickedColor() {
    if (preColorUnit != nullptr) {
        preColorUnit->changeClickedFlag(false);
        preColorUnit = nullptr;
    }
    for (int i = m_colorLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *it = m_colorLayout->itemAt(i);
        PictureUnit *colorUnit = static_cast<PictureUnit*>(it->widget());
        if (currentFile == colorUnit->getColorName()) {
            colorUnit->changeClickedFlag(true);
            preColorUnit = colorUnit;
            colorUnit->takeColorGroupTabStop();
        }
    }
}

void WallpaperUi::resetClickedPic() {
    if (prePictureUnit != nullptr) {
        prePictureUnit->changeClickedFlag(false);
        prePictureUnit = nullptr;
    }
    for (int i = m_pictureLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *it = m_pictureLayout->itemAt(i);
        PictureUnit *picUnit = static_cast<PictureUnit*>(it->widget());
        if (currentFile == picUnit->filenameText()) {
            picUnit->changeClickedFlag(true);
            prePictureUnit = picUnit;
            picUnit->takeGroupTabStop();
        }
    }
    ensurePictureTabOrder();
    if (m_focusSelectedPicAfterReset && prePictureUnit) {
        m_focusSelectedPicAfterReset = false;
        PictureUnit *target = prePictureUnit;
        // dbus 刷新后延后一拍落焦，避免仍停在「恢复默认」再 Tab 到主页
        QTimer::singleShot(0, this, [this, target]() {
            if (target) {
                target->takeGroupTabStop();
                ensurePictureTabOrder();
                target->setFocus(Qt::OtherFocusReason);
            }
        });
    } else {
        m_focusSelectedPicAfterReset = false;
    }
}
