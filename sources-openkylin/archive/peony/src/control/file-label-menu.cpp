/*
 * Peony-Qt
 *
 * Copyright (C) 2026, Tianjin KYLIN Information Technology Co., Ltd.
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
 *
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */
#include "file-label-menu.h"
#include "file-label-model.h"
#include "file-info.h"
#include "tag-management.h"

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QStyleOptionMenuItem>
#include <QDebug>
#include <QFontMetrics>
#include <QPainterPath>

#define MAX_WIDTH 200

void LabelItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 1. 初始化视图项选项
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

    // 2. 创建菜单项选项并绘制背景
    QStyleOptionMenuItem menuOpt;
    menuOpt.initFrom(opt.widget);
    menuOpt.rect = opt.rect;
    menuOpt.menuItemType = QStyleOptionMenuItem::Normal;
    menuOpt.text = "";
    menuOpt.checked = opt.state & QStyle::State_Selected;
    menuOpt.state = QStyle::State_Enabled | QStyle::State_Active;

    // 处理悬停状态
    if (opt.widget) {
        QPoint cursorPos = QCursor::pos();
        QPoint widgetPos = opt.widget->mapFromGlobal(cursorPos);
        if (menuOpt.rect.contains(widgetPos)) {
            menuOpt.state |= QStyle::State_MouseOver;
            menuOpt.state |= QStyle::State_Selected;
        }
    }

    if (opt.state & QStyle::State_Selected) {
        menuOpt.state |= QStyle::State_Selected;
    }

    // 处理高DPI
    if (qApp->devicePixelRatio() != 1.0) {
        menuOpt.rect.adjust(1, 1, -1, -1);
    }

    // 绘制菜单项背景
    style->drawControl(QStyle::CE_MenuItem, &menuOpt, painter, opt.widget);

    // 3. 获取数据
    QString color = index.data(Qt::UserRole).toString();
    QString text = index.data(Qt::DisplayRole).toString();
    bool checked = index.data(Qt::UserRole + 1).toBool();

    // 4. 使用原始的 option.rect
    QRect itemRect = opt.rect;

    // 5. 从样式系统获取标准度量值
    int iconSize = style->pixelMetric(QStyle::PM_SmallIconSize);
    int checkSize = 16;
    int leftMargin = style->pixelMetric(QStyle::PM_MenuHMargin);
    int rightMargin = style->pixelMetric(QStyle::PM_MenuHMargin);
    int spacing = style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);

    if (spacing <= 0) {
        spacing = 6;
    }

    int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    if (textMargin <= 0) {
        textMargin = 2;
    }

    // 6. 获取菜单项的内容区域
    QRect contentRect = style->subElementRect(QStyle::SE_PushButtonContents, &opt, opt.widget);
    if (contentRect.isValid()) {
        leftMargin = contentRect.left() - itemRect.left();
        rightMargin = itemRect.right() - contentRect.right();
    }

    // 7. 获取字体度量信息，用于计算文本高度
    QFontMetrics fm(painter->font());
    int textHeight = fm.height();

    // 8. 计算文本的垂直中心位置
    int textCenterY = itemRect.center().y();

    // 9. 绘制颜色圆点 - 相对于文本垂直居中
    int iconX = itemRect.left() + leftMargin;
    int iconY = textCenterY - iconSize/2;  // 基于文本中心Y坐标
    QRect iconRect(iconX, iconY, iconSize, iconSize);

    // 绘制圆形
    painter->save();
    painter->setBrush(QColor(color));
    painter->setPen(Qt::NoPen);

    int circleMargin = 2;
    QRectF circleRect = iconRect.adjusted(circleMargin, circleMargin, -circleMargin, -circleMargin);
    painter->drawEllipse(circleRect);
    painter->restore();

    // 10. 计算文本区域
    int textX = iconRect.right() + spacing;
    int textRightBound = itemRect.right() - rightMargin;

    if (checked) {
        textRightBound -= (checkSize + spacing);
    }

    int textWidth = textRightBound - textX;

    // 11. 绘制文本
    if (textWidth > 0 && !text.isEmpty()) {
        painter->save();

        // 设置文本颜色
        if (opt.state & QStyle::State_Enabled) {
            painter->setPen(opt.palette.color(QPalette::WindowText));
        } else {
            painter->setPen(opt.palette.color(QPalette::Disabled, QPalette::WindowText));
        }

        // 文本缩略处理
        QString elidedText = fm.elidedText(text, Qt::ElideRight, textWidth - textMargin);

        // 创建文本矩形，使用文本中心Y坐标
        QRect textRect(textX, textCenterY - textHeight/2, textWidth, textHeight);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

        painter->restore();
    }

    // 12. 如果选中，绘制对号（右边）- 也相对于文本垂直居中
    if (checked) {
        int checkX = itemRect.right() - rightMargin - checkSize;
        int checkY = textCenterY - checkSize/2;  // 基于文本中心Y坐标
        QRect checkRect(checkX, checkY, checkSize, checkSize);

        painter->save();

        QPen pen(opt.palette.color(QPalette::WindowText));
        pen.setWidth(2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing, true);

        int checkMarkSize = checkSize * 0.5;
        QPoint center = checkRect.center();

        QPainterPath path;
        path.moveTo(center.x() - checkMarkSize/2, center.y());
        path.lineTo(center.x(), center.y() + checkMarkSize/2);
        path.lineTo(center.x() + checkMarkSize/2, center.y() - checkMarkSize/2);

        painter->drawPath(path);

        painter->restore();
    }

    painter->restore();
}
FileLabelMenu::FileLabelMenu(QStringList &selections, QWidget *parent) : QMenu(parent)
{
    m_selections = selections;

    QWidget *container = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // QListWidget
    listWidget = new QListWidget(this);
    listWidget->viewport()->setStyleSheet("background: transparent;");

    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setFrameShape(QFrame::NoFrame);
    listWidget->setItemDelegate(new LabelItemDelegate);
    // 设置尺寸策略，让 QListWidget 不会撑大菜单
    listWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    connect(listWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        // 切换选中状态
        int index = item->data(Qt::UserRole + 2).toInt();
        bool checked = clickItem(index);
        item->setData(Qt::UserRole + 1, checked);
        // 刷新
        listWidget->viewport()->update();
    });

    loadLabels();

    layout->addWidget(listWidget);

    // 标题
    int num = selections.count();
    QString text;

    // 使用固定最小字体(9号)计算文本缩略，确定显示内容
    QFont minFont = font();
    minFont.setPointSize(9);  // 固定最小字体
    QFontMetrics minFm(minFont);

    if (num == 1) {
        auto info = Peony::FileInfo::fromUri(m_selections.first());
        QString displayName = info->displayName();

        // 模板文本的各部分
        QString prefix = QString(tr("Add labels to \"%1\"")).arg("");

        // 用最小字体计算固定部分的宽度
        int prefixWidth = minFm.horizontalAdvance(prefix);

        // 菜单可用宽度（减去左右边距）
        int availableWidth = MAX_WIDTH - 30;

        // 可用于文件名的最大宽度（用最小字体计算）
        int maxNameWidth = availableWidth - prefixWidth;

        qDebug() << "Menu width:" << this->width()
                 << "Available:" << availableWidth
                 << "Prefix:" << prefixWidth
                 << "Max name width:" << maxNameWidth;

        // 用最小字体进行文本缩略，确定显示内容
        if (maxNameWidth > 0) {
            int nameWidth = minFm.horizontalAdvance(displayName);
            if (nameWidth > maxNameWidth) {
                QString oldName = displayName;
                displayName = minFm.elidedText(displayName, Qt::ElideMiddle, maxNameWidth);
                qDebug() << "Elided:" << oldName << "->" << displayName;
            }
        }

        // 组合最终文本（用最小字体缩略后的文本）
        QString fullText = QString(tr("Add labels to \"%1\"")).arg(displayName);
        text = fullText;

        // 验证最终宽度（用最小字体）
        int finalWidth = minFm.horizontalAdvance(fullText);
        qDebug() << "Final width:" << finalWidth << "Limit:" << availableWidth;
    } else {
        text = QString(tr("Add labels to %1 items")).arg(QString::number(num));
    }

    // 使用 QWidgetAction 创建标题，使文本与下方图标位置对齐
    QWidget *titleWidget = new QWidget(this);
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    // 获取菜单左边距，使文本与图标位置对齐
    int hMargin = this->style()->pixelMetric(QStyle::PM_MenuHMargin);
    titleLayout->setContentsMargins(hMargin, 0, 0, 0);
    titleLayout->setSpacing(0);

    // 标题字体比当前字体小2号
    QFont titleFont = font();
    titleFont.setPointSize(titleFont.pointSize() - 2);

    m_titleLabel = new QLabel(text, titleWidget);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setEnabled(false);
    titleLayout->addWidget(m_titleLabel);

    QWidgetAction *titleAction = new QWidgetAction(this);
    titleAction->setDefaultWidget(titleWidget);
    addAction(titleAction);

    // 分隔线
    addSeparator();

    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(container);
    addAction(action);

    // 分隔线
    addSeparator();

    QAction *tagAction =  new QAction(QIcon::fromTheme("sign-symbolic"), tr("Label Management"), this);
    connect(tagAction, &QAction::triggered, this, [=]() {
        Peony::TagManagement *managent = Peony::TagManagement::getInstance();
        managent->show();
    });
    addAction(tagAction);
}

void FileLabelMenu::updateSelections(const QStringList &selections)
{
    // 更新选中项
    m_selections = selections;

    // 清空并重新加载标签
    clearLabels();
    loadLabels();

    int num = m_selections.count();
    QString text;

    // 使用固定最小字体(9号)计算文本缩略
    QFont minFont = font();
    minFont.setPointSize(9);  // 固定最小字体
    QFontMetrics minFm(minFont);

    if (num == 1) {
        auto info = Peony::FileInfo::fromUri(m_selections.first());
        QString displayName = info->displayName();

        // 模板文本的各部分
        QString prefix = QString(tr("Add labels to \"%1\"")).arg("");

        // 用最小字体计算固定部分的宽度
        int prefixWidth = minFm.horizontalAdvance(prefix);

        // 菜单可用宽度（减去左右边距）
        int availableWidth = MAX_WIDTH - 30;

        // 可用于文件名的最大宽度（用最小字体计算）
        int maxNameWidth = availableWidth - prefixWidth;

        qDebug() << "Menu width:" << this->width()
                 << "Available:" << availableWidth
                 << "Prefix:" << prefixWidth
                 << "Max name width:" << maxNameWidth;

        // 用最小字体进行文本缩略
        if (maxNameWidth > 0) {
            int nameWidth = minFm.horizontalAdvance(displayName);
            if (nameWidth > maxNameWidth) {
                QString oldName = displayName;
                displayName = minFm.elidedText(displayName, Qt::ElideMiddle, maxNameWidth);
                qDebug() << "Elided:" << oldName << "->" << displayName;
            }
        }

        // 组合最终文本
        QString fullText = QString(tr("Add labels to \"%1\"")).arg(displayName);
        text = fullText;

        // 验证最终宽度（用最小字体）
        int finalWidth = minFm.horizontalAdvance(fullText);
        qDebug() << "Final width:" << finalWidth << "Limit:" << availableWidth;
    } else {
        text = QString(tr("Add labels to %1 items")).arg(QString::number(num));
    }
    m_titleLabel->setText(text);
}

void FileLabelMenu::clearLabels()
{
    listWidget->clear();
    m_ids.clear();
}

void FileLabelMenu::addTag(int id, const QString &color, const QString &text, bool checked)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setText(text);
    item->setData(Qt::UserRole, color);      // 存储颜色
    item->setData(Qt::UserRole + 1, checked); // 存储选中状态
    item->setData(Qt::UserRole + 2, id); // 存储id

    // 设置自定义绘制
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    listWidget->addItem(item);
}

QList<QPair<QString, QString>> FileLabelMenu::selectedTags() const
{
    QList<QPair<QString, QString>> result;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (item->data(Qt::UserRole + 1).toBool()) {
            result.append({item->data(Qt::UserRole).toString(), item->text()});
        }
    }
    return result;
}

void FileLabelMenu::loadLabels()
{
    auto items = FileLabelModel::getGlobalModel()->getAllFileLabelItems();

    QSet<int> currentLabelIds;
    for (const QString &uri : m_selections) {
        auto ids = FileLabelModel::getGlobalModel()->getFileLabelIds(uri);
        currentLabelIds.unite(QSet<int>(ids.begin(), ids.end()));
    }
    m_ids = currentLabelIds.toList();
    for (auto item : items) {
        if (!item->isValidInMenu()) {
            continue;
        }
        bool checked = currentLabelIds.contains(item->id());
        addTag(item->id(), item->color().name(), item->name(), checked);
    }

    // 计算并设置 QListWidget 的高度，确保所有标签都能显示而不需要滚动条
    if (listWidget->count() > 0) {
        // 方法1：使用sizeHintForRow（最简单）
        int itemHeight = listWidget->sizeHintForRow(0);
        int totalHeight = listWidget->count() * itemHeight;
        listWidget->setFixedHeight(totalHeight);
    }
}

bool FileLabelMenu::clickItem(int index)
{
    bool allHaveLabel = true;
    for (QString uri : m_selections) {
        auto ids = FileLabelModel::getGlobalModel()->getFileLabelIds(uri);
        if (!ids.contains(index)) {
            allHaveLabel = false;
            break;
        }
    }

    // 根据结果统一操作
    if (allHaveLabel) {
        // 所有文件都有这个标签 → 全部移除
        for (QString uri : m_selections) {
            FileLabelModel::getGlobalModel()->removeFileLabel(uri, index);
        }
    } else {
        // 不是所有文件都有这个标签 → 全部添加（已有的不受影响）
        for (QString uri : m_selections) {
            // 只给没有这个标签的文件添加
            auto ids = FileLabelModel::getGlobalModel()->getFileLabelIds(uri);
            if (!ids.contains(index)) {
                FileLabelModel::getGlobalModel()->addLabelToFile(uri, index);
            }
        }
        return true;
    }
    return false;
}

