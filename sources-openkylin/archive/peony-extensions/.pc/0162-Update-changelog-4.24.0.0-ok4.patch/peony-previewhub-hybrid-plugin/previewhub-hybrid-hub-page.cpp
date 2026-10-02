/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "previewhub-hybrid-hub-page.h"
#include <file-watcher.h>
#include <preview-page/default-preview-page/default-preview-page.h>
#include <QLabel>
#include <QFormLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollArea>
#include <QShowEvent>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <ukui_previewhub_api.h>

static constexpr int kPreviewMinWidth = 96;

/**
 * @class PreviewhubOpaquePreviewPage
 * @brief 带有 Base 背景色绘制的 DefaultPreviewPage 子类。
 *
 * 覆盖 paintEvent 以确保背景始终使用 palette(base) 颜色填充，
 * 与混合预览页面的整体背景保持一致。
 */
class PreviewhubOpaquePreviewPage : public Peony::DefaultPreviewPage
{
public:
    explicit PreviewhubOpaquePreviewPage(QWidget *parent = nullptr)
        : Peony::DefaultPreviewPage(parent)
    {
        setContentsMargins(10, 20, 10, 40);
        setAutoFillBackground(true);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), palette().color(QPalette::Base));
        QStackedWidget::paintEvent(event);
    }
};

/**
 * @brief 构造函数，初始化根布局、文件信息页和预览宿主控件。
 * @param parent 父控件指针
 */
PreviewhubHybridHubPage::PreviewhubHybridHubPage(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_page = new PreviewhubOpaquePreviewPage(this);
    m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rootLayout->addWidget(m_page);

    m_previewHost = new QWidget;
    m_previewHost->setAutoFillBackground(true);
    QPalette pal = m_previewHost->palette();
    pal.setColor(QPalette::Window, pal.color(QPalette::Base));
    m_previewHost->setPalette(pal);

    m_hubWidget = UkuiPreviewHub::createPreviewWidget(m_previewHost);
    m_hubWidget->setParent(m_previewHost);
}

PreviewhubHybridHubPage::~PreviewhubHybridHubPage()
{
    cancel();
}

/**
 * @brief 绘制事件，使用 Base 颜色填充背景以保持与不支持预览时一致。
 */
void PreviewhubHybridHubPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().color(QPalette::Base));
    QWidget::paintEvent(event);
}

/**
 * @brief 获取文件信息页内部的 QScrollArea。
 * @return 滚动区域指针，未找到时返回 nullptr
 */
QScrollArea *PreviewhubHybridHubPage::fileInfoScrollArea() const
{
    return m_page ? m_page->findChild<QScrollArea *>() : nullptr;
}

/**
 * @brief 获取滚动区域中的内容控件。
 * @return 内容控件指针，未找到时返回 nullptr
 */
QWidget *PreviewhubHybridHubPage::fileInfoContentWidget() const
{
    if (QScrollArea *scroll = fileInfoScrollArea())
        return scroll->widget();
    return nullptr;
}

/**
 * @brief 配置滚动区域的样式属性。
 *
 * 设置无边框、自适应大小、居中对齐、隐藏水平滚动条，
 * 并统一滚动区域及视口的背景色为 palette(base)。
 *
 * DefaultPreviewPage 会为 QScrollArea 设置透明背景 QSS；这里清空
 * 该局部样式，避免 QStyleSheetStyle 改变滚动条外观，背景由调色板设置。
 */
void PreviewhubHybridHubPage::setupScrollAreaStyle()
{
    QScrollArea *scroll = fileInfoScrollArea();
    if (!scroll)
        return;

    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setAlignment(Qt::AlignCenter);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet(QString());
    scroll->viewport()->setStyleSheet(QString());

    QPalette palette = scroll->palette();
    palette.setColor(QPalette::Window, palette.color(QPalette::Base));
    scroll->setPalette(palette);
    scroll->viewport()->setPalette(palette);
    scroll->viewport()->setAutoFillBackground(true);
}

/**
 * @brief 更新文件信息内容区域的对齐方式。
 *
 * 设置内容控件最小宽度为视口宽度（仅约束宽度，高度由布局自然撑开），
 * 将所有子控件水平居中对齐。
 */
void PreviewhubHybridHubPage::updateFileInfoContentAlignment()
{
    QScrollArea *scroll = fileInfoScrollArea();
    QWidget *content = fileInfoContentWidget();
    if (!scroll || !content)
        return;

    content->setAutoFillBackground(true);
    if (scroll->viewport())
        content->setMinimumWidth(scroll->viewport()->width());

    auto *box = qobject_cast<QVBoxLayout *>(content->layout());
    if (!box)
        return;

    QWidget *formWidget = nullptr;
    for (int i = 0; i < box->count(); ++i) {
        if (QLayoutItem *item = box->itemAt(i)) {
            if (QWidget *widget = item->widget()) {
                if (qobject_cast<QFormLayout *>(widget->layout())) {
                    formWidget = widget;
                    break;
                }
            }
        }
    }

    const int formWidth = formWidget ? qMax(formWidget->width(), formWidget->sizeHint().width()) : 0;

    box->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < box->count(); ++i) {
        if (QLayoutItem *item = box->itemAt(i)) {
            if (QWidget *widget = item->widget()) {
                box->setAlignment(widget, Qt::AlignHCenter);
                if (auto *label = qobject_cast<QLabel *>(widget)) {
                    if (label->textFormat() == Qt::RichText)
                        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                }
            }
        }
    }

    if (formWidth > 0) {
        for (int i = 0; i < box->count(); ++i) {
            if (QLayoutItem *item = box->itemAt(i)) {
                if (auto *label = qobject_cast<QLabel *>(item->widget())) {
                    if (label->textFormat() == Qt::RichText) {
                        label->setFixedWidth(formWidth);
                        box->setAlignment(label, Qt::AlignHCenter);
                    }
                }
            }
        }
    }

    if (m_previewInjected)
        box->setAlignment(m_previewHost, Qt::AlignCenter);
}

/**
 * @brief 将预览宿主控件注入到文件信息内容布局的顶部。
 *
 * 仅在首次调用时执行插入操作，后续调用为空操作。
 */
void PreviewhubHybridHubPage::injectPreviewWidget()
{
    QWidget *page = fileInfoContentWidget();
    if (!page)
        return;

    auto *box = qobject_cast<QVBoxLayout *>(page->layout());
    if (!box)
        return;

    if (m_previewHost->parent() != page)
        m_previewHost->setParent(page);

    if (!m_previewInjected) {
        box->insertWidget(0, m_previewHost, 0, Qt::AlignHCenter);
        m_previewInjected = true;
    }
}

/**
 * @brief 隐藏文件信息区域中原有的图标和间隔控件。
 *
 * 预览注入后，原图标区域不再需要显示，将其隐藏以避免重复展示。
 */
void PreviewhubHybridHubPage::hideFileInfoIconArea()
{
    QWidget *page = fileInfoContentWidget();
    if (!page || !page->layout())
        return;

    QLayout *layout = page->layout();
    const int iconIndex = m_previewInjected ? 1 : 0;
    const int spacerIndex = m_previewInjected ? 2 : 1;

    for (int index : {iconIndex, spacerIndex}) {
        if (index < layout->count()) {
            if (QLayoutItem *item = layout->itemAt(index)) {
                if (QWidget *widget = item->widget())
                    widget->hide();
            }
        }
    }
}

/**
 * @brief 计算预览区域可用的列宽。
 * @return 视口宽度与最小宽度中的较大值
 */
int PreviewhubHybridHubPage::availableColumnWidth() const
{
    if (QScrollArea *scroll = fileInfoScrollArea()) {
        if (scroll->viewport()) {
            const int viewportW = scroll->viewport()->width();
            if (viewportW > 0)
                return qMax(kPreviewMinWidth, viewportW);
        }
    }
    return qMax(kPreviewMinWidth, width());
}

/**
 * @brief 根据当前可用宽度更新预览控件的几何尺寸。
 *
 * 按 4:3 宽高比设置预览宿主及内部预览控件大小，
 * 并使内容布局重新计算，确保预览放大后下方信息区域不被遮挡。
 */
void PreviewhubHybridHubPage::updatePreviewGeometry()
{
    if (!m_previewHost || m_previewHost->isHidden())
        return;

    updateFileInfoContentAlignment();

    const int columnW = availableColumnWidth();
    const int previewH = qMax(1, columnW * 3 / 4);

    m_previewHost->setFixedSize(columnW, previewH);
    m_hubWidget->setGeometry(0, 0, columnW, previewH);

    // 预览尺寸变化后刷新内容布局，使下方信息区域随之下移而非被遮挡
    QWidget *content = fileInfoContentWidget();
    if (content && content->layout()) {
        content->layout()->invalidate();
        content->updateGeometry();
    }
}

void PreviewhubHybridHubPage::scheduleHubPreviewStart()
{
    ++m_hubPreviewGeneration;
    const int generation = m_hubPreviewGeneration;

    QTimer::singleShot(0, this, [this, generation]() {
        if (generation != m_hubPreviewGeneration || m_uri.isEmpty())
            return;
        if (!m_previewHost || m_previewHost->isHidden() || !m_hubWidget)
            return;

        updatePreviewGeometry();
        UkuiPreviewHub::startPreview(m_hubWidget);
        m_hubWidget->update();
    });
}

/**
 * @brief 准备预览资源。
 * @param uri 目标文件的 URI
 *
 * 重建文件监听器以跟踪文件位置变化，并通知预览库和文件信息页准备数据。
 */
void PreviewhubHybridHubPage::prepare(const QString &uri)
{
    m_uri = uri;

    if (m_watcher)
        m_watcher->stopMonitor();
    m_watcher = std::make_shared<Peony::FileWatcher>(uri);
    connect(m_watcher.get(), &Peony::FileWatcher::locationChanged, this, [this](const QString &, const QString &newUri) {
        prepare(newUri);
        startPreview();
    });
    m_watcher->startMonitor();

    UkuiPreviewHub::cancelPreview(m_hubWidget);
    UkuiPreviewHub::preparePreview(m_hubWidget, m_uri);
    m_page->prepare(m_uri);
}

/**
 * @brief 启动预览显示。
 *
 * 当文件信息页展示的是滚动区域时，注入并显示预览控件、隐藏原图标区域，
 * 随后启动预览库渲染并更新几何尺寸；否则隐藏预览控件并取消预览。
 */
void PreviewhubHybridHubPage::startPreview()
{
    m_page->startPreview();
    setupScrollAreaStyle();

    QScrollArea *scroll = fileInfoScrollArea();
    const bool showFileInfo = scroll && m_page->currentWidget() == scroll;

    if (showFileInfo) {
        injectPreviewWidget();
        hideFileInfoIconArea();
        m_previewHost->show();
        updatePreviewGeometry();
        scheduleHubPreviewStart();
    } else {
        ++m_hubPreviewGeneration;
        m_previewHost->hide();
        UkuiPreviewHub::cancelPreview(m_hubWidget);
    }
}

/**
 * @brief 取消预览并释放相关资源。
 */
void PreviewhubHybridHubPage::cancel()
{
    ++m_hubPreviewGeneration;

    if (m_watcher)
        m_watcher->stopMonitor();
    m_watcher.reset();
    m_uri.clear();

    m_page->cancel();
    UkuiPreviewHub::cancelPreview(m_hubWidget);
    m_previewHost->hide();
}

/**
 * @brief 尺寸变化事件，预览区域随窗口大小自适应更新。
 */
void PreviewhubHybridHubPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updatePreviewGeometry();
}

void PreviewhubHybridHubPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_uri.isEmpty() && m_previewHost && !m_previewHost->isHidden()) {
        updatePreviewGeometry();
        scheduleHubPreviewStart();
    }
}
