/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "icon-view-index-widget.h"
#include "icon-view-delegate.h"
#include "icon-view.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QLabel>

#include <QApplication>
#include <QStyle>
#include <QTextDocument>
#include <QScrollBar>

#include <QMouseEvent>
#include <QStyleHints>
#include <QStyleOptionButton>
#include <QTimer>

#include "file-info.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-item.h"
#include "file-utils.h"
#include "emblem-provider.h"
#include "global-settings.h"
#include "idm-dbus-manager.h"

#include <QDebug>
#include <QTextLayout>

using namespace Peony;
using namespace Peony::DirectoryView;

IconViewIndexWidget::IconViewIndexWidget(const IconViewDelegate *delegate, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *parent) : QWidget(parent)
{
    setObjectName("peony_icon_view_index_widget");
    installEventFilter(parent);

    setMouseTracking(true);
    setProperty("needTranslucent", false);
    m_edit_trigger.setInterval(3000);
    m_edit_trigger.setSingleShot(true);

    QTimer::singleShot(750, this, [=]() {
        m_edit_trigger.start();
    });

    //use QTextEdit to show full file name when select
    m_edit = new QTextEdit();

    m_option = option;
    m_index = index;

    m_delegate = delegate;

//    m_delegate->getView()->m_renameTimer->stop();
//    m_delegate->getView()->m_editValid = false;
//    m_delegate->getView()->m_renameTimer->start();

    m_is_dragging = m_delegate->getView()->isDraggingState();

    QSize size = delegate->sizeHint(option, index);
    setMinimumSize(size);

    //extra emblems
    auto proxy_model = static_cast<FileItemProxyFilterSortModel*>(delegate->getView()->model());
    auto item = proxy_model->itemFromIndex(index);
    if (item) {
        m_info = item->info();
    }

    m_delegate->initStyleOption(&m_option, m_index);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    m_option.features.setFlag(QStyleOptionViewItem::WrapText);
#else
    m_option.features |= QStyleOptionViewItem::WrapText;
#endif
    m_option.textElideMode = Qt::ElideNone;

    auto opt = m_option;
    //opt.rect.setHeight(9999);
    opt.rect.moveTo(0, 0);

    //qDebug()<<m_option.rect;
    auto iconExpectedSize = m_delegate->getView()->iconSize();
    QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    auto y_delta = iconExpectedSize.height() - iconRect.height();
    opt.rect.moveTo(opt.rect.x(), opt.rect.y() + y_delta);

    m_option = opt;

    adjustPos();

    auto textSize = IconViewTextHelper::getTextSizeForIndex(opt, index, 2);
    int fixedHeight = 5 + iconExpectedSize.height() + 5 + textSize.height() + 5;
    int y_bottom = option.rect.y() + fixedHeight + 20;
    //qDebug() << "Y:" <<option.rect.y() <<fixedHeight <<m_delegate->getView()->height();
    b_elide_text = false;
    if ( y_bottom > m_delegate->getView()->height() && opt.text.length() > ELIDE_TEXT_LENGTH)
    {
        b_elide_text = true;
        int  charWidth = opt.fontMetrics.averageCharWidth();
        opt.text = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
        //recount size
        textSize = IconViewTextHelper::getTextSizeForIndex(opt, index, 2);
        fixedHeight = 5 + iconExpectedSize.height() + 5 + textSize.height() + 5;
    }

    if (fixedHeight >= option.rect.height())
        setFixedHeight(fixedHeight);
    else
        setFixedHeight(option.rect.height());

    m_option.rect.setHeight(fixedHeight - y_delta);

    // 进入/退出多选时刷新，否则 IndexWidget 不会随 viewport()->update() 重绘复选框
    connect(m_delegate->getView(), &IconView::updateSelectStatus, this, [this](bool) {
        update();
    });

    connect(m_delegate, &IconViewDelegate::updateIndexWidget, this, [=](const QStyleOptionViewItem &option){
        m_option = option;
        // try fix task #575255, 右键菜单打开时文管崩溃
        m_index = m_option.index;
        m_delegate->initStyleOption(&m_option, m_index);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
        m_option.features.setFlag(QStyleOptionViewItem::WrapText);
#else
        m_option.features |= QStyleOptionViewItem::WrapText;
#endif
        m_option.textElideMode = Qt::ElideNone;

        auto opt = m_option;
        opt.rect.moveTo(0, 0);

        auto iconExpectedSize = m_delegate->getView()->iconSize();
        QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
        auto y_delta = iconExpectedSize.height() - iconRect.height();
        opt.rect.moveTo(opt.rect.x(), opt.rect.y() + y_delta);
        m_option = opt;

        m_option.rect.setHeight(fixedHeight - y_delta);
        update();
    });

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    connect(qApp, &QApplication::fontChanged, this, [=]() {
        m_delegate->getView()->setIndexWidget(m_index, nullptr);
    });
#else
    //FIXME: handle font change.
#endif
}

IconViewIndexWidget::~IconViewIndexWidget()
{
    delete m_edit;
}

bool IconViewIndexWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        adjustPos();
        update();
    }
    return false;
}

void IconViewIndexWidget::clearModelIndex()
{
    m_delegate->getView()->setIndexWidget(m_index, nullptr);
}

void IconViewIndexWidget::paintEvent(QPaintEvent *e)
{
    bool isUpdateEnable = updatesEnabled();
    if (!isUpdateEnable)
        return;

    if (!m_info.lock()) {
        return;
    }

    IconView *view = m_delegate->getView();
    auto expectedRect = view->visualRect(m_index);
    auto rectPos = this->mapToParent(QPoint(0, 0));
    if (expectedRect.topLeft() != rectPos) {
        adjustPos();
        update();
        return;
    }

    QWidget::paintEvent(e);
    QPainter p(this);

    auto opt = m_option;
    auto rawRect = m_option.rect;
    opt.rect = this->rect();

    auto info = m_info.lock();
    if (info->isEmptyInfo()) {
        return;
    }
    auto colors = info->getColors();
    if(info->uri().startsWith("favorite://")){/* 快速访问须特殊处理 */
        //快速访问目录，颜色标记设置后更新不及时问题单独处理,修复bug#118015
        auto matchInfo = FileInfo::fromUri(FileUtils::getEncodedUri(FileUtils::getTargetUri(info->uri())));
        colors = matchInfo->getColors();
    }
    int colorWidth = colors.count()>3 ? 12/2*3+10 : colors.count() > 0? 12/2*colors.count()+10 : 0;
    int horizalMargin = 2;
    auto fontMetrics = opt.fontMetrics;
    int pixelsWide = fontMetrics.width(opt.text) + colorWidth;
    int width = opt.rect.width() - 2*horizalMargin;
    if(pixelsWide < width){
        opt.rect = opt.rect.adjusted(0,0,0,-31);
    }

    auto bgColor = QApplication::palette().base().color();
    p.save();
    p.setPen(Qt::transparent);
    p.setBrush(bgColor);
    p.drawRoundedRect(opt.rect, 6, 6);
    p.restore();
    //qDebug()<<m_option.backgroundBrush;
    //qDebug()<<this->size() << m_delegate->getView()->iconSize();

    auto model = static_cast<FileItemProxyFilterSortModel*>(view->model());
    auto item = model->itemFromIndex(m_index);

#ifdef KY_UDF_BURN
    if (item) {
        /* R类型光盘，所有用于刻录的文件（夹）展示在挂载点时都应该半透明显示，区别于普通文件 ,linkto task#122470 */
        if(item->property("isFileForBurning").toBool()){
            p.setOpacity(0.5);
        }else{
            p.setOpacity(1.0);
        }
    }//end
#endif

    auto rawDecoSize = opt.decorationSize;
    opt.decorationSize = m_delegate->getView()->iconSize();
    opt.state &= ~QStyle::State_MouseOver;
    QPalette savedPalette = palette();
    QColor hlColor = savedPalette.color(QPalette::Highlight);
    hlColor.setAlphaF(0.24);
    QPalette selectionPalette = savedPalette;
    selectionPalette.setColor(QPalette::Highlight, hlColor);
    setPalette(selectionPalette);
    opt.palette = selectionPalette;
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem,
                                         &opt,
                                         &p,
                                         nullptr);
    setPalette(savedPalette);
    opt.palette = savedPalette;
    opt.decorationSize = rawDecoSize;

    opt.rect = rawRect;
    auto tmp = opt.text;
    opt.text = nullptr;
    //bug#99340,修改图标选中状态，会变暗
    auto state = opt.state;
    if((opt.state & QStyle::State_Enabled) && (opt.state & QStyle::State_Selected)) {
        opt.state &= ~QStyle::State_Selected;
    }

    if (info->uri().startsWith("filesafe:///")) {/* hotfix bug#225885 保护箱锁定后图标没有变 */
        opt.icon = qvariant_cast<QIcon>(m_index.data(Qt::DecorationRole));
    }
    p.save();
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    // p.translate(0, -5);
    // 复选框由下方自绘，避免与 CE_ItemViewItem 默认指示器叠画
    opt.features &= ~QStyleOptionViewItem::HasCheckIndicator;
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, &p, opt.widget);
    p.restore();
    opt.state = state;
    if (b_elide_text)
    {
        int  charWidth = opt.fontMetrics.averageCharWidth();
        tmp = opt.fontMetrics.elidedText(tmp, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
    }
    opt.text = std::move(tmp);

    //extra emblems
    if(info->uri().startsWith("favorite://")){/* 快速访问须特殊处理 */
        info = FileInfo::fromUri(FileUtils::getEncodedUri(FileUtils::getTargetUri(info->uri())));
    }
    if (info->uri().startsWith("filesafe:///")) {
        opt.icon = qvariant_cast<QIcon>(m_index.data(Qt::DecorationRole));
    }

    auto lineSpacing = opt.fontMetrics.lineSpacing();
    int yoffset = 0;
    // draw color symbols
    int xoffset = 0;
    if(0 < colors.count())
    {
        const int MAX_LABEL_NUM = 3;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num =  colors.count() - startIndex;

        QString text = opt.text;
        QFont font = opt.font;
        QTextLayout textLayout(text, font);

        QTextOption textOpt;
        textOpt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

        textLayout.setTextOption(textOpt);
        textLayout.beginLayout();


        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            return;
        int width = opt.rect.width() - (num+1)*6 - 2*2 - 4;
        line.setLineWidth(width);

        xoffset = (width - line.naturalTextWidth())/2 ;
        if(xoffset < 0)
        {
            xoffset = 2;
        }
        yoffset = (lineSpacing -12 )/2;

        for (int i = startIndex; i < colors.count(); ++i) {
            auto color = colors.at(i);
            p.save();
            //fix bug#147348
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.translate(0, m_delegate->getView()->iconSize().height() + 5 + 4);

           // p.translate(2, 2);
            p.setPen(opt.palette.highlightedText().color());
            p.setBrush(color);
            p.drawEllipse(QRectF(xoffset, yoffset, 12, 12));
            p.restore();
            xoffset += 12/2;
        }

        yoffset = 0;
        xoffset += 10;
    }
    QString regFindKeyWords = m_delegate->getRegFindKeyWords();
    p.save();
    p.translate(0, m_delegate->getView()->iconSize().height() + 5 + yoffset + 5);
    p.setPen(opt.palette.highlightedText().color());

    qreal textHeight = IconViewTextHelper::drawText(&p,
                                                     opt,
                                                     9999,
                                                     xoffset,
                                                     regFindKeyWords,
                                                     2,
                                                     4);

    //fix#bug182191 【文件管理器】文件添加标记后选中状态名称显示不全
    int fixedHeight = 5 + m_delegate->getView()->iconSize().height() + 5 + textHeight + 5;
    if (fixedHeight > this->height())
        setFixedHeight(fixedHeight);

    p.restore();

    QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    iconRect.translate(0, -5);
    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left
    int emblemsSize = iconRect.width() / 3;
    int topLeftX = iconRect.x() - emblemsSize * 0.2;
    int topLeftY = iconRect.y();
    int bottomRightX = iconRect.x() + iconRect.width() - emblemsSize * 0.8;
    int bottomRightY = iconRect.y() + iconRect.height() - emblemsSize * 0.8;

    //paint symbolic link emblems
    if (info->isSymbolLink()) {
        emblemPoses.removeOne(3);
        QIcon icon = QIcon::fromTheme("emblem-link-symbolic");
        //qDebug()<< "symbolic:" << info->symbolicIconName();
        //icon.paint(&p, this->width() - 30, 10, 20, 20, Qt::AlignCenter);
        //Adjust link emblem to topLeft.link story#8354
        p.save();
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        icon.paint(&p, topLeftX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
        p.restore();
    }

    // 多选复选框：有 IndexWidget 时 delegate 绘制会被盖住，必须在此自绘。
    // 位置由样式 SE_ItemViewItemCheckIndicator 决定，与普通图标项保持一致（跟随主题）。
    // 命中区在 mousePressEvent 中以同源 rect 加 padding 计算，保证点中即可见。
    if (view->isEnableMultiSelect()
            && GlobalSettings::getInstance()->getValue(SHOW_MULTISELECT_CHECKBOX).toBool()) {
        QStyleOptionViewItem checkOpt = m_option;
        checkOpt.rect = this->rect();
        checkOpt.features |= QStyleOptionViewItem::HasCheckIndicator;
        const QRect indicator = style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &checkOpt, view);

        QStyleOptionButton cbOpt;
        cbOpt.initFrom(this);
        cbOpt.rect = indicator;
        cbOpt.state |= QStyle::State_Enabled;
        if (view->selectionModel()->isSelected(m_index)) {
            cbOpt.state |= QStyle::State_On;
        } else {
            cbOpt.state |= QStyle::State_Off;
        }
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &cbOpt, &p, this);
    }

    //paint access emblems
    //NOTE: we can not query the file attribute in smb:///(samba) and network:///.
    if (!info->uri().startsWith("file:") && !info->uri().startsWith("filesafe:") && !info->uri().startsWith("idm:") && !info->uri().startsWith("filesafe:")) {
        return;
    }

    /**
     * @bug #262561: [File Manager] PDF desktop shortcut files with deleted source files
     *  do not display the same icon on the desktop folder as on the desktop.
     *
     * If the source file of a symbolic link is deleted, an “X” icon will be displayed in the upper left corner.
     *
     * @author: Renyg <renyangguang@kylinos.cn>
     * @date:   2024-09-11
     */
    if (!info->canRead() || !info->isExistTargetOfSymlink()) {
        emblemPoses.removeOne(1);
        QIcon icon = QIcon::fromTheme("emblem-unreadable");
        p.save();
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        icon.paint(&p, topLeftX, topLeftY, emblemsSize, emblemsSize);
        p.restore();
    } else if (!info->canWrite()/* && !info->canExecute()*/) {
        //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
        emblemPoses.removeOne(1);
        QIcon icon = QIcon::fromTheme("emblem-readonly");
        p.save();
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        icon.paint(&p, topLeftX, topLeftY, emblemsSize, emblemsSize);
        p.restore();
    }

    if (info->uri().startsWith("idm://") && info->uri() != "idm:///") {
        QString spaceName = FileUtils::urlDecode(info->uri().remove("idm://").split('/', QString::SkipEmptyParts).first());
        QString realPath = FileUtils::urlDecode(info->targetUri());
        realPath = realPath.remove("file://");
        if (!spaceName.isEmpty() && !realPath.isEmpty()) {
            bool userAdded = IDMDBusManager::getInstance()->isUserAddFile(spaceName, realPath);
            if (userAdded) {
                emblemPoses.removeOne(3);
                QIcon icon = QIcon::fromTheme("emblem-personal-symbolic");
                p.save();
                p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                icon.paint(&p, topLeftX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
                p.restore();
            }
        }
    }


    // paint extension emblems, FIXME: adjust layout, and implemet on indexwidget, other view.
    auto extensionsEmblems = EmblemProviderManager::getInstance()->getAllEmblemsForUri(info->uri());

    for (auto extensionsEmblem : extensionsEmblems) {
        if (emblemPoses.isEmpty()) {
            break;
        }

        QIcon icon = QIcon::fromTheme(extensionsEmblem);
        if (!icon.isNull()) {
            p.save();
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            int pos = emblemPoses.takeFirst();
            switch (pos) {
            case 1: {
                icon.paint(&p, topLeftX, topLeftY, emblemsSize, emblemsSize, Qt::AlignCenter);
                break;
            }
            case 2: {
                icon.paint(&p, bottomRightX, topLeftY, emblemsSize, emblemsSize, Qt::AlignCenter);
                break;
            }
            case 3: {
                icon.paint(&p, topLeftX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
                break;
            }
            case 4: {
                icon.paint(&p, bottomRightX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
                break;
            }
            default:
                break;
            }
            p.restore();
        }
    }
}

void IconViewIndexWidget::mousePressEvent(QMouseEvent *e)
{
    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    if (singleClicked) {
        IconView *view = m_delegate->getView();
        if (!view->m_touch_active_timer->isActive()) {
            view->m_touch_active_timer->start(1100);
        }
    }

    if (e->button() == Qt::LeftButton || e->button() == Qt::MiddleButton) {
        IconView *view = m_delegate->getView();
        const QPoint viewportPos = mapTo(view->viewport(), e->pos());
        view->m_dragStartPos = viewportPos;
    }

    if (e->button() == Qt::LeftButton) {
        IconView *view = m_delegate->getView();
        if (view->isDraggingState() || m_is_dragging || view->m_ctrl_key_pressed) {
            view->m_renameTimer->stop();
            view->m_editValid = false;
            return QWidget::mousePressEvent(e);
        }
        if (view->isEnableMultiSelect()) {
            view->m_skipClickDeselect = false;
            const bool isSelected = view->selectionModel()->isSelected(m_index);
            QRect textRect = m_option.rect;
            QSize iconExpectedSize = view->iconSize();
            textRect.setY(iconExpectedSize.height());
            textRect.setHeight(m_option.rect.height() - iconExpectedSize.height());
            // 复选框命中区：与 paintEvent 同源（SE_ItemViewItemCheckIndicator），
            // 在指示器周围加 padding 扩大点击区域，保证易用性
            QStyleOptionViewItem checkOpt = m_option;
            checkOpt.rect = this->rect();
            checkOpt.features |= QStyleOptionViewItem::HasCheckIndicator;
            const QRect indicator = view->style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &checkOpt, view);
            const QRect checkboxRect = indicator.adjusted(-10, -4, 10, 4);

            // 复选框区域单独记录，不触发手动拖拽
            m_pressedInCheckboxArea = checkboxRect.contains(e->pos());

            // Shift+点击：从锚点到当前项做区间选择（保留原有单点切换/拖拽/框选逻辑）
            const bool shiftPressed = e->modifiers() & Qt::ShiftModifier;
            const QModelIndex &anchor = view->m_multiSelectAnchor;
            if (shiftPressed && anchor.isValid() && m_index.isValid()
                    && anchor.model() == m_index.model()
                    && anchor.parent() == m_index.parent()) {
                view->m_multiSelectToggleHandled = true;
                view->m_noSelectOnPress = false;
                view->m_toggleClickIndex = QModelIndex();
                view->m_pressedOnBlank = false;
                view->m_pressedGlobalPos = e->globalPos();
                view->m_dragStarted = false;

                QItemSelection range(anchor, m_index);
                const QItemSelectionModel::SelectionFlags flag =
                    (e->modifiers() & Qt::ControlModifier)
                        ? QItemSelectionModel::Select        // Ctrl+Shift：在已有选中上追加区间
                        : QItemSelectionModel::ClearAndSelect; // Shift：替换为区间（标准语义）
                view->selectionModel()->select(range, flag);
                // 锚点保持不变，与 Qt ExtendedSelection 行为一致

                view->viewport()->update();
                this->update();
                if (checkboxRect.contains(e->pos()) || textRect.contains(e->pos())) {
                    e->accept();
                } else {
                    e->ignore();
                }
                return;
            }

            // 统一处理选中逻辑
            view->m_toggleClickIndex = m_index;
            view->m_multiSelectToggleHandled = true;
            view->m_noSelectOnPress = isSelected;
            view->m_pressedOnBlank = false;       // 在项上按下，非空白
            view->m_pressedGlobalPos = e->globalPos();  // 用于手动拖拽检测（IndexWidget 截获事件，不会进入 IconView::mousePressEvent）
            view->m_dragStarted = false;
            if (!isSelected) {
                view->selectionModel()->select(m_index, QItemSelectionModel::Select);
                view->viewport()->update();
                this->update();
            }
            // 非 Shift 点击刷新锚点，为下一次 Shift 区间选择提供起点
            view->m_multiSelectAnchor = m_index;

            // 复选框和文本区域不传递事件，图标区域传递用于拖拽
            if (checkboxRect.contains(e->pos()) || textRect.contains(e->pos())) {
                e->accept();
            } else {
                e->ignore();
            }
            return;
        }

        //FIXME: Modify the icon style, only click on the text to respond, click on the icon to not respond
        QRect rect =  m_option.rect;
        QSize iconExpectedSize = m_delegate->getView()->iconSize();
        rect.setY(iconExpectedSize.height());
        rect.setHeight(m_option.rect.height()-iconExpectedSize.height());
        if(!rect.contains(e->pos()))
        {
            view->m_editValid = false;
            view->m_renameTimer->start();
            return ;
        }

        view->m_editValid = true;
        if (view->m_renameTimer->isActive()) {
            if (view->m_renameTimer->remainingTime() < 3000 - qApp->styleHints()->mouseDoubleClickInterval() && view->m_renameTimer->remainingTime() > 0) {
                view->slotRename();
            } else if(view->m_renameTimer->remainingTime() >= 3000 - qApp->styleHints()->mouseDoubleClickInterval()){
                //优化文件点击策略，提升用户体验，关联bug#125368
                //在双击时间间隔内，如果未触发双击事件，但是点击的是同一个有效图标，触发双击事件
                //系统默认双击间隔为400ms, 策略为[0,400]，触发双击，(400,3000)触发重命名
                mouseDoubleClickEvent(e);
            }
            else {
                view->m_editValid = false;
                view->m_renameTimer->stop();
            }
        } else {
            view->m_editValid = false;
            view->m_renameTimer->start();
        }
        //e->accept();
        //return;
        /**
         * @bug #239593: [File Manager] Click on any folder to enter and then return. After returning, cannot drag the folder directly
         *
         * Transfer the mousePressEvent event of IconViewIndexWidget to the IconView class for further processing
         * Used to handle drag and drop events
         *
         * @author Renyg
         * @date 2024-07-11
         */
        return QWidget::mousePressEvent(e);
//        if (m_edit_trigger.isActive()) {
//            qDebug()<<"IconViewIndexWidget::mousePressEvent: edit"<<e->type();
//            m_delegate->getView()->setIndexWidget(m_index, nullptr);
//            m_delegate->getView()->edit(m_index);
//            return;
//        }
    }
    if(e->button() == Qt::RightButton){
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void IconViewIndexWidget::mouseMoveEvent(QMouseEvent *e)
{
    IconView *view = m_delegate->getView();
    // 多选模式下，文件名/文本区域的 mousePress 被 IndexWidget 接收，导致后续 move
    // 不会进入 IconView::mouseMoveEvent。在此处直接执行手动拖拽阈值检测，并单独
    // 排除复选框区域，避免勾选复选框时误触发拖拽。
    if (view->isEnableMultiSelect() && (e->buttons() & Qt::LeftButton) && !m_pressedInCheckboxArea) {
        if (!view->m_dragStarted) {
            const int dragDistance = (e->globalPos() - view->m_pressedGlobalPos).manhattanLength();
            if (dragDistance >= QApplication::startDragDistance()) {
                view->m_dragStarted = true;
                view->m_noSelectOnPress = false;
                if (!view->selectedIndexes().isEmpty()) {
                    view->startDrag(view->defaultDropAction());
                }
                e->accept();
                return;
            }
        }
        e->accept();
        return;
    }
    QWidget::mouseMoveEvent(e);
}

void IconViewIndexWidget::mouseReleaseEvent(QMouseEvent *e)
{
    IconView *view = m_delegate->getView();
    const bool wasDragStarted = view->m_dragStarted;
    // 修复：非多选模式下，Ctrl+点击也应支持取消选中
    const bool deferClickDeselect = view->m_noSelectOnPress && (view->isEnableMultiSelect() || view->m_ctrl_key_pressed) && !wasDragStarted;

    const QModelIndex deselectIndex = view->m_toggleClickIndex.isValid() ? view->m_toggleClickIndex : m_index;
    if (deferClickDeselect && deselectIndex.isValid()) {
        // 立即取消选中，不使用延迟，避免时序问题
        if (!view->m_skipClickDeselect) {
            if (view->selectionModel()->isSelected(deselectIndex)) {
                view->selectionModel()->select(deselectIndex, QItemSelectionModel::Deselect);
                this->update();
            }
        }
    }
    view->m_noSelectOnPress = false;
    view->m_toggleClickIndex = QModelIndex();
    view->m_multiSelectToggleHandled = false;
    view->m_dragStarted = false;
    m_pressedInCheckboxArea = false;

    QWidget::mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton || e->button() == Qt::MiddleButton) {
        m_delegate->getView()->m_dragStartPos = QPoint(-1, -1);
    }
    e->accept();
}

void IconViewIndexWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    /**
     * @bug #250731: [File Manager] Right clicking on the same folder several times in the file manager will take you to the folder
     *
     * Prevent double-click events from triggering on right-click
     * Only double left clicks will be processed
     *
     * @author Renyg
     * @date 2024-08-12
     */
    if (event->button() == Qt::RightButton) {
        return;
    }

    IconView *view = m_delegate->getView();
    // 双击事件在 Qt 内部会代替第二次 mousePressEvent，需要与 mousePressEvent
    // 同步刷新 m_dragStartPos（注意要映射到 viewport 坐标，与 mousePressEvent 一致），
    // 否则双击后若触发拖拽，startDrag 拿到的是 release 中被清空的 (-1,-1)。
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        const QPoint viewportPos = mapTo(view->viewport(), event->pos());
        view->m_dragStartPos = viewportPos;
    }

    view->m_skipClickDeselect = true;
    if (view->isEnableMultiSelect() && m_index.isValid()) {
        const QString uri = m_index.data(FileItemModel::UriRole).toString();
        if (!uri.isEmpty()) {
            view->setSelections(QStringList() << uri);
            view->activated(m_index);
        }
        const int guardMs = qApp->styleHints()->mouseDoubleClickInterval() + 50;
        QTimer::singleShot(guardMs, view, [view]() {
            view->m_skipClickDeselect = false;
        });
        return;
    }

    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    bool isPreviewMode = view->topLevelWidget()->property("isPreviewMode").toBool();
    if (!singleClicked || isPreviewMode) {
        view->activated(m_index);
    }
    view->m_skipClickDeselect = false;
}

void IconViewIndexWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    hide();
}

void IconViewIndexWidget::adjustPos()
{
    IconView *view = m_delegate->getView();
    if (m_index.model() != view->model())
        return;

    if (!view->selectionModel()->selectedIndexes().contains(m_index)) {
        this->close();
        return;
    }

    auto visualRect = view->visualRect(m_index);
    if (this->mapToParent(QPoint()) != visualRect.topLeft())
        this->move(visualRect.topLeft());
}
