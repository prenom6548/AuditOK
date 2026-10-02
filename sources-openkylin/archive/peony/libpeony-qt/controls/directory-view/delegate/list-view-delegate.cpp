/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "list-view-delegate.h"
#include "file-operation-manager.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-item.h"

#include "list-view.h"
#include "checkbox-header.h"
#include "clipboard-utils.h"

#include "file-info.h"
#include "file-info-job.h"
#include "emblem-provider.h"
#include "global-settings.h"
#include "list-view-style.h"
#include "idm-dbus-manager.h"

#include <memory>
#include <QTimer>
#include <QPushButton>

#include <QPainter>
#include <QDBusReply>
#include <QKeyEvent>
#include <QItemDelegate>
#include <QMouseEvent>
#include <file-label-model.h>
#include <QHeaderView>
#include <QApplication>

using namespace Peony;

ListViewDelegate::ListViewDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_styled_button = new QPushButton;
}

ListViewDelegate::~ListViewDelegate()
{
    m_styled_button->deleteLater();
}

void ListViewDelegate::initIndexOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    return initStyleOption(option, index);
}

void ListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.displayAlignment = Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter);

    auto view = qobject_cast<DirectoryView::ListView *>(parent());
    opt.decorationSize = view->iconSize();

    // 复选框列：仅绘制单元格背景（含选中态圆角）+ 与表头对齐的复选框指示器（不使用 QSS）
    // 复选框位置由 CheckBoxHeader::indicatorRect 统一计算，保证表头/数据行严格对齐
    const int checkCol = view ? view->checkboxColumn() : -1;
    if (checkCol >= 0 && index.column() == checkCol) {
        // 1) 绘制单元格背景：将 viewItemPosition 设为 Beginning，
        //    让 style 在选中态下对左侧（视觉第一列）做圆角处理
        opt.text = QString();
        opt.icon = QIcon();
        opt.features &= ~QStyleOptionViewItem::HasCheckIndicator;
        opt.viewItemPosition = QStyleOptionViewItem::Beginning;
        if (opt.widget) {
            opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
        }
        // 2) 绘制 16x16 复选框指示器（左对齐 8px 留白 + 垂直居中）
        if (opt.widget) {
            const QRect indicatorR = Peony::CheckBoxHeader::indicatorRect(option.rect);
            QStyleOptionButton cbOpt;
            cbOpt.initFrom(opt.widget);
            cbOpt.rect = indicatorR;
            cbOpt.state |= QStyle::State_Enabled;
            if (option.state & QStyle::State_Selected) {
                cbOpt.state |= QStyle::State_Selected;
            }
            const Qt::CheckState state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
            switch (state) {
            case Qt::Checked:
                cbOpt.state |= QStyle::State_On;
                break;
            case Qt::PartiallyChecked:
                cbOpt.state |= QStyle::State_NoChange;
                break;
            default:
                cbOpt.state |= QStyle::State_Off;
                break;
            }
            opt.widget->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &cbOpt, painter, opt.widget);
        }
        return;
    }

    auto model = static_cast<FileItemProxyFilterSortModel*>(view->model());
    auto item = model->itemFromIndex(index);
    if (!item) {
        return;
    }
    auto info = item->info();
    if (info->isEmptyInfo()) {
        return;
    }
    auto colors = info->getColors();

    /* 此处以中文命名的文件保护箱标记实时同步还存在问题，是由于uri编码（尽管使用FileUtils::urlEncoded进行转换）与底层(info的uri)不匹配 */
    QString uri = index.data(Qt::UserRole).toString();
    if(uri.startsWith("favorite://")){/* 快速访问须特殊处理 */
        //快速访问目录，颜色标记设置后更新不及时问题单独处理,修复bug#118015
        uri =FileUtils::getEncodedUri(FileUtils::getTargetUri(uri));
        auto matchInfo = FileInfo::fromUri(uri);
        colors = matchInfo->getColors();
    }

    int labelAreaWidth = 0;
    if (index.column() == 0 && colors.count() >0) {
        if (!view->isDragging() || !view->selectionModel()->selectedIndexes().contains(index)) {
            // 先只计算标记区域宽度，圆点稍后绘制（在背景之上）
            int labelSize = 12;
            const int MAX_LABEL_NUM = 3;
            int alignment = view->getLabelAlignment();
            if (alignment == DirectoryView::ListView::LabelAlignment::AlignHorizontal) {
                int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
                int num = colors.count() - startIndex;
                if (num > 0) {
                    labelAreaWidth = (num+1)*labelSize/2 + 20;
                }
            }
        }
    }


    auto clipedUris = ClipboardUtils::getInstance()->getCutFileUris();
    if (!clipedUris.isEmpty()){
        QString actualDirUri = view->getDirectoryUri();
        bool bSearchTab = false;
        if(actualDirUri.startsWith("search:///search_uris")){
            actualDirUri = FileUtils::getActualDirFromSearchUri(actualDirUri);
            bSearchTab = true;
        }

        QString realActualDirUri = actualDirUri;
        if (actualDirUri.startsWith("smb://") || actualDirUri.startsWith("ftp://") || actualDirUri.startsWith("sftp://")) {
            auto actualInfo = FileInfo::fromUri(actualDirUri);
            realActualDirUri = "file://" + actualInfo->filePath();
        }

        QString clipedFilesParentUri = ClipboardUtils::getClipedFilesParentUri();
        if ((FileUtils::isSamePath(clipedFilesParentUri, actualDirUri) || (bSearchTab && clipedFilesParentUri.startsWith(actualDirUri))
             || FileUtils::isSamePath(clipedFilesParentUri, realActualDirUri)) && !clipedUris.isEmpty()) {
            if (clipedUris.contains(index.data(Qt::UserRole).toString())) {
                painter->setOpacity(0.5);
                //qDebug()<<"cut item in list view"<<index.data();
            }
            else
                painter->setOpacity(1.0);
        }
    }
    else
       painter->setOpacity(1.0);
//    if (!opt.state.testFlag(QStyle::State_Selected)) {
//        if (opt.state & QStyle::State_Sunken) {
//            opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
//        }
//        if (opt.state & QStyle::State_MouseOver) {
//            opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
//        }
//    }

    if (index.column() == 0 && !m_regFindKeyWords.isEmpty()) {
        QString text1 = opt.text;
        opt.text = QString();
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget); //绘制非文本区域内容
        opt.text = text1;
        painter->save();

        setElidedNamePolicy(info, opt);
        QString text = opt.text;
        QFont font = opt.font;
        QFontMetrics fontMetrics = opt.fontMetrics;
        int lineSpacing = fontMetrics.lineSpacing();
        //计算text的长度
        QRect textRect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
        // 搜索高亮模式下也要扣除标记区域宽度
        if (labelAreaWidth > 0) {
            textRect.setWidth(textRect.width() - labelAreaWidth);
        }
        QString elidedText = fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());
        painter->translate(textRect.topLeft());
        int yoffset = (textRect.height() - lineSpacing)/2;
        painter->translate(0, yoffset);
        textRect.moveTo(0,0);

        painter->setPen(opt.palette.highlightedText().color());
        QTextOption textOpt;
        textOpt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textOpt.setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

        QTextDocument document;
        document.setDefaultTextOption(textOpt);
        document.setTextWidth(textRect.width());
        document.setDefaultFont(font);
        document.setIndentWidth(0);
        document.setDocumentMargin(0);
        //此处应该设置elidled text
        document.setPlainText(elidedText);

        QTextCursor highlightCursor(&document);
        QTextCursor cursor(&document);
        cursor.beginEditBlock();
        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setBackground(QBrush(QColor(95,208,101)));
        if (opt.state.testFlag(QStyle::State_Selected)) {
            QTextCharFormat selectColorFormat(cursor.charFormat());
            selectColorFormat.setForeground(Qt::white);
            cursor.select(QTextCursor::Document);
            cursor.mergeCharFormat(selectColorFormat);
        }

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document.find(m_regFindKeyWords, highlightCursor);
            if (!highlightCursor.isNull()) {
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }

        cursor.endEditBlock();
        document.drawContents(painter, textRect);
        painter->restore();
    } else {
        setElidedNamePolicy(info, opt);
        if (labelAreaWidth > 0) {
            // 有水平标记时：将文字预先按扣除标记区域后的宽度省略，再用完整 rect 一次性
            // 绘制整个 item。这样背景（含边框/渐变）只在 item 真实边缘绘制一次，不会在
            // 标记区域两侧出现竖线；文字颜色完全由 style 按当前状态决定，与无标记项一致；
            // 预省略后的文字不会延伸到标记区域，避免与圆点重叠。
            QRect textRect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
            int availWidth = textRect.width() - labelAreaWidth;
            QString textBackup = opt.text;
            opt.text = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, availWidth);
            opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
            opt.text = textBackup;
        } else {
            opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
        }
    }

    // 在背景和文字都绘制完成后，在最上层绘制标记圆点
    if (index.column() == 0 && colors.count() > 0) {
        if (!view->isDragging() || !view->selectionModel()->selectedIndexes().contains(index)) {
            paintLabel(opt, view->getLabelAlignment(), colors, painter);
        }
    }

    QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left

    //add link and read only icon support
    if (index.column() == 0) {
        bool isSymbolicLink = info->isSymbolLink();
        int emblemsSize = view->iconSize().width() / 2;
        auto iconSize = view->iconSize();
        int topLeftX = iconRect.x() - emblemsSize * 0.2;
        int topLeftY = iconRect.y();
        int bottomRightX = iconRect.x() + iconSize.width() - emblemsSize * 0.8;
        int bottomRightY = iconRect.y() + iconSize.height() - emblemsSize * 0.8;

        //paint symbolic link emblems
        if (isSymbolicLink) {
            emblemPoses.removeOne(3);
            QIcon icon = QIcon::fromTheme("emblem-link-symbolic");
            //qDebug()<<info->symbolicIconName();
            //icon.paint(painter, loc_x, loc_y, size, size);
            //Adjust link emblem to topLeft.link story#8354
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            icon.paint(painter, topLeftX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
            painter->restore();
        }

        //paint access emblems
        //NOTE: we can not query the file attribute in smb:///(samba) and network:///.
        if (info->uri().startsWith("file:") || info->uri().startsWith("mtp:") || info->uri().startsWith("gphoto2:") || info->uri().startsWith("filesafe:") || info->uri().startsWith("idm:")) {
            /**
             * @bug #262561: [File Manager] PDF desktop shortcut files with deleted source files
             *  do not display the same icon on the desktop folder as on the desktop.
             *
             * If the source file of a symbolic link is deleted, an “X” icon will be displayed in the upper left corner.
             *
             * @author: Renyg <renyangguang@kylinos.cn>
             * @date:   2024-09-11
             */
            if (!info->canRead()  || !info->isExistTargetOfSymlink()) {
                emblemPoses.removeOne(1);
                QIcon icon = QIcon::fromTheme("emblem-unreadable");
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                icon.paint(painter, topLeftX, topLeftY, emblemsSize, emblemsSize);
                painter->restore();
            } else if (!info->canWrite()/* && !info->canExecute()*/) {
                //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
                emblemPoses.removeOne(1);
                QIcon icon = QIcon::fromTheme("emblem-readonly");
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                icon.paint(painter, topLeftX, topLeftY, emblemsSize, emblemsSize);
                painter->restore();
            }
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
                    painter->save();
                    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                    icon.paint(painter, topLeftX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
                    painter->restore();
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
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                int pos = emblemPoses.takeFirst();
                switch (pos) {
                case 1: {
                    icon.paint(painter, topLeftX, topLeftY, emblemsSize, emblemsSize, Qt::AlignCenter);
                    break;
                }
                case 2: {
                    icon.paint(painter, bottomRightX, topLeftY, emblemsSize, emblemsSize, Qt::AlignCenter);
                    break;
                }
                case 3: {
                    icon.paint(painter, topLeftX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
                    break;
                }
                case 4: {
                    icon.paint(painter, bottomRightX, bottomRightY, emblemsSize, emblemsSize, Qt::AlignCenter);
                    break;
                }
                default:
                    break;
                }
                painter->restore();
            }
        }
    }
}

bool ListViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    auto view = qobject_cast<DirectoryView::ListView *>(parent());
    const int checkCol = view ? view->checkboxColumn() : -1;

    // 仅复选框列需要自定义命中检测；其它列走 Qt 默认行为
    if (checkCol >= 0 && index.isValid() && index.column() == checkCol) {
        // 只处理鼠标左键释放（点击切换的常规时机），避免与 press 阶段的选区/拖拽逻辑冲突
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                // 点击复选框列任意位置都切换选中/取消选中
                const Qt::CheckState cur = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
                const Qt::CheckState next = (cur == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
                return model->setData(index, next, Qt::CheckStateRole);
            }
            return false;
        }
        // press/dblclick 不处理（交给 view 的鼠标事件链路），但不要走默认 checkable toggle
        if (event->type() == QEvent::MouseButtonPress
                || event->type() == QEvent::MouseButtonDblClick) {
            return false;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QWidget *ListViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    TextEdit *edit = new TextEdit(parent);
    edit->setAcceptRichText(false);
    //edit->setContextMenuPolicy(Qt::CustomContextMenu);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setWordWrapMode(QTextOption::NoWrap);

    edit->blockSignals(true);
    auto displayString = index.data(Qt::DisplayRole).toString();
    auto displayName = index.data(Qt::UserRole + 1).toString();
    auto uri = index.data(Qt::UserRole).toString();
    auto suffix = displayName.remove(displayString);
    auto fsType = FileUtils::getFsTypeFromFile(uri);
    auto info = FileInfo::fromUri(uri);
    int32_t maxLength = 255;
    if (info->isDesktopFile()) {
        suffix = ".desktop";
    }
    if (FileUtils::isFuseFileSystem(uri)) {
        fsType = "fuse.kyfs";
    }
    if (fsType.contains("ext")) {
        maxLength = 255 - suffix.toLocal8Bit().length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType == "ecryptfs") {
        maxLength = 143 - suffix.toLocal8Bit().length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType == "udf") {
        maxLength = 254 - suffix.toLocal8Bit().length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType.contains("ntfs")) {
        edit->setLimitBytes(false);
        maxLength = 255 - suffix.length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType.contains("fuse.kyfs")) {
        edit->setLimitBytes(false);
        QDBusInterface iface ("com.kylin.file.system.fuse","/com/kylin/file/system/fuse","com.kylin.file.system.fuse",QDBusConnection::systemBus());
        QDBusReply<int32_t> reply = iface.call("GetFilenameLength");
        if (reply.isValid()) {
            maxLength = reply.value();
        }
        maxLength = maxLength - suffix.length();
        edit->setMaxLengthLimit(maxLength);
    }
    edit->blockSignals(false);

//    QTimer::singleShot(1, parent, [=]() {
//        this->updateEditorGeometry(edit, option, index);
//    });

//    connect(edit, &TextEdit::textChanged, this, [=]() {
//        updateEditorGeometry(edit, option, index);
//    });

    connect(edit, &TextEdit::textChanged, this, [=]() {
        auto text = edit->toPlainText();
        //fix bug#220283, rename edit position wrong issue
        //short file name no need update to avoid position wrong
        edit->adjustText();
        //updateEditorGeometry(edit, option, index);
    });

    connect(edit, &TextEdit::finishEditRequest, this, &ListViewDelegate::slot_finishEdit);

    connect(edit, &QWidget::destroyed, this, [=]() {
        Q_EMIT isEditing(false);
    });

    return edit;
}

void ListViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    TextEdit *edit = qobject_cast<TextEdit *>(editor);
    if (!edit)
        return;

    Q_EMIT isEditing(true);
    edit->setText(index.data(Qt::DisplayRole).toString());
    auto cursor = edit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    bool isDir = FileUtils::getFileIsFolder(index.data(Qt::UserRole).toString());
    bool isDesktopFile = index.data(Qt::UserRole).toString().endsWith(".desktop");
    bool isSoftLink = FileUtils::getFileIsSymbolicLink(index.data(Qt::UserRole).toString());
    if (!isDesktopFile && !isSoftLink && !isDir && edit->toPlainText().contains(".") && !edit->toPlainText().startsWith(".")) {
        int n = 1;
        if(index.data(Qt::DisplayRole).toString().contains(".tar.")) //ex xxx.tar.gz xxx.tar.bz2
            n = 2;
        while(n){
            cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor, 1);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            --n;
        }
    }
    //qDebug()<<cursor.anchor();
    edit->setTextCursor(cursor);
}

void ListViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    TextEdit *edit = qobject_cast<TextEdit*>(editor);
    edit->m_backgroundEdit->setGeometry(1, 2, edit->size().width() - 2, edit->size().height() - 4);
    QTimer::singleShot(0, edit, [=]() {
        int top = (edit->height() - edit->document()->size().height())/2;
        top = top < 2 ? 2 : top;
        edit->setMargins(1, top, 1, 2);
    });
}

void ListViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    TextEdit *edit = qobject_cast<TextEdit*>(editor);
    if (!edit)
        return;

    auto text = edit->toPlainText();
    if (text.isEmpty())
        return;
    if (text == index.data(Qt::DisplayRole).toString())
        return;
    //process special name . or .. or only space
    if (text == "." || text == ".." || text.trimmed() == "")
        return;

    if (! index.isValid())
        return;

    auto view = qobject_cast<DirectoryView::ListView *>(parent());
    auto oldName = index.data(Qt::DisplayRole).toString();
    if (text == oldName)
    {
        //create new file, should select the file or folder
        auto flags = QItemSelectionModel::Select|QItemSelectionModel::Rows;
        view->selectionModel()->select(index, flags);
        view->setFocus();
        return;
    }

    if (view->getSelections().count() > 1) {
        auto fileOpMgr = FileOperationManager::getInstance();
        QStringList lists = view->getSelections();
        auto renameOp = new FileBatchRenameOperation(lists, text);

        connect(renameOp, &FileBatchRenameOperation::operationFinished, view, [=](){
            auto info = renameOp->getOperationInfo().get();
            auto uri = info->target();
            QTimer::singleShot(100, view, [=](){
                view->setSelections(QStringList()<<uri);
                //after rename will nor sort immediately, comment to fix bug#60482
                //view->scrollToSelection(uri);
                view->setFocus();
            });
        }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    } else {
        auto fileOpMgr = FileOperationManager::getInstance();
        auto renameOp = new FileRenameOperation(index.data(FileItemModel::UriRole).toString(), text);

        connect(renameOp, &FileRenameOperation::operationFinished, view, [=](){
            auto info = renameOp->getOperationInfo().get();
            auto uri = info->target();
            QTimer::singleShot(100, view, [=](){
                view->setSelections(QStringList()<<uri);
                //after rename will nor sort immediately, comment to fix bug#60482
                //view->scrollToSelection(uri);
                view->setFocus();
            });
        }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    }
}

//not comment this bug to fix bug#93314
QSize ListViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    auto view = qobject_cast<DirectoryView::ListView *>(parent());
    int expectedHeight = view->iconSize().height() + 4;
    size.setHeight(qMax(expectedHeight, size.height()));
    return size;
}

void ListViewDelegate::slot_finishEdit()
{
    auto edit = qobject_cast<QWidget *>(sender());
    commitData(edit);
    closeEditor(edit, QAbstractItemDelegate::SubmitModelCache);
    if(edit){
        delete edit;
        edit = nullptr;
    }
}

void ListViewDelegate::setSearchKeyword(QString regFindKeyWords)
{
    m_regFindKeyWords = regFindKeyWords;
}

int ListViewDelegate::paintLabel(QStyleOptionViewItem &opt, int aalignment, QList<QColor> colors, QPainter *painter) const
{
    //修改标记个数最多为3个，以及标记位置
    int xOffSet = 0;
    int yOffSet = 0;
    int labelSize = 12;
    int labelAreaWidth = 0;
    const int MAX_LABEL_NUM = 3;
    switch(aalignment) {
    case DirectoryView::ListView::LabelAlignment::AlignVertical: {
        xOffSet = 5;
        int index = 0;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num = colors.count() - startIndex + 1;

        //set color label on center, fix bug#40609
        auto iconSize = opt.decorationSize;
        labelSize = iconSize.height()/3;
        if (labelSize > 10)
            labelSize = 10;
        if (labelSize <6)
            labelSize = 6;

        yOffSet = (opt.rect.height()-labelSize*num/2)/2;
        if(yOffSet < 2)
        {
            yOffSet = 2;
        }
        for (int i = startIndex; i < colors.count(); ++i, ++index) {
            auto color = colors.at(i);
            painter->save();
            //fix bug#147348
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->translate(0, opt.rect.topLeft().y());
            painter->translate(2, 0);
            painter->setPen(opt.palette.highlightedText().color());
            painter->setBrush(color);
            painter->drawEllipse(QRectF(xOffSet, yOffSet, labelSize, labelSize));
            painter->restore();
            yOffSet += labelSize/2;
        }
        break;
    }
    case DirectoryView::ListView::LabelAlignment::AlignHorizontal:{
        QRect rect = opt.rect;
        xOffSet = rect.topRight().x() - labelSize/2 - 20;
        yOffSet = rect.height()/2 - labelSize/2;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num =  colors.count() - startIndex;
        if(num > 0){
            //计算标记区域占用的宽度（不再绘制背景和修改opt.rect）
            labelAreaWidth = (num+1)*labelSize/2 + 20;
        }
        for (int i = startIndex; i < colors.count(); ++i) {
            auto color = colors.at(i);
            painter->save();
            //fix bug#147348
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->translate(0, rect.topLeft().y());
            painter->translate(2, 2);
            painter->setPen(opt.palette.highlightedText().color());
            painter->setBrush(color);
            painter->drawEllipse(QRectF(xOffSet, yOffSet, labelSize, labelSize));
            painter->restore();
            xOffSet -= labelSize/2;
        }
        break;
    }
    }
    return labelAreaWidth;
}

void ListViewDelegate::setElidedNamePolicy(const std::shared_ptr<FileInfo>& info, const QStyleOptionViewItem &opt)
{
    // Early return if the display name doesn't match the text in the style option
    if (info->displayName() != opt.text) {
        return;
    }

    const QFontMetrics fm(opt.font);
    const QRect textRect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
    const QString elidedText = fm.elidedText(info->displayName(), Qt::ElideRight, textRect.width());

    // Set the 'isElided' property based on whether the text is truncated
    info->setProperty("isElided", elidedText != info->displayName());
}

//TextEdit
TextEdit::TextEdit(QWidget *parent) : QTextEdit (parent)
{
    // fix #164278, icon view text editor doesn't cover view item.
    // note on ukui platform theme, style panel frame is not visible.
    setFrameShape(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    setViewportMargins(1, 2, 1, 2);
    if (m_backgroundEdit == nullptr) {
        m_backgroundEdit = new QTextEdit(this);
        m_backgroundEdit->setReadOnly(true);
        m_backgroundEdit->setFrameShape(QTextEdit::NoFrame);

        m_backgroundEdit->setGeometry(1, 2, size().width() - 2, size().height() - 4);
        m_backgroundEdit->lower();
    }
}

void TextEdit::adjustText()
{
    if (m_max_length_limit) {
        //fix #154584
        blockSignals(true);
        auto position = textCursor().position();
        while (true) {
            if (m_limit_bytes) {
                auto local8Bit = toPlainText().toLocal8Bit();
                if (local8Bit.length() <= m_max_length_limit) {
                    break;
                }
            } else {
                if (toPlainText().length() <= m_max_length_limit) {
                    break;
                }
            }
            if (position > 0) {
                position--;
                textCursor().beginEditBlock();
                textCursor().setPosition(position);
                textCursor().deletePreviousChar();
                textCursor().endEditBlock();
            } else {
                break;
            }
        }
        blockSignals(false);
    }
}

void TextEdit::setMaxLengthLimit(int length)
{
    m_max_length_limit = length;
}

void TextEdit::setLimitBytes(bool limitBytes)
{
    m_limit_bytes = limitBytes;
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        Q_EMIT finishEditRequest();
        return;
    }
    return QTextEdit::keyPressEvent(e);
}

void TextEdit::setMargins(int left, int top, int right, int bottom)
{
    setViewportMargins(left, top, right, bottom);
}
