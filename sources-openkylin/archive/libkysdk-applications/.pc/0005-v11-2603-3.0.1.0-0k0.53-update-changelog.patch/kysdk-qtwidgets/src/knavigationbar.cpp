/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "knavigationbar.h"
#include "parmscontroller.h"
#include "themeController.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QGSettings>
#include <QLinearGradient>
#include <QListView>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QToolTip>
#include <QVBoxLayout>
#include <QtMath>

namespace kdk
{

enum ItemType {
    StandardItem = 0,
    SubItem,
    TagItem
};

class ListView : public QListView
{
public:
    ListView(QWidget *parent);

    QPoint mousePoint();

protected:
    void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void leaveEvent(QEvent *event);

private:
    QPoint m_mousePoint;
};

class Delegate : public QStyledItemDelegate, public ThemeController
{
    Q_OBJECT
public:
    Delegate(QObject *parent, ListView *view);

    void setTabValueVisible(bool visible);

    void setAddIcon(QString tag, QIcon icon);

    void setAddTagVisible(QString tag, bool visible);

    void setExtendIcon(QString tag, QIcon icon);

    void setExtendTagVisible(QString tag, bool visible);

Q_SIGNALS:
    void addClicked(QString str);
    void extendClicked(QString str, int row);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    ListView *m_listView;
    bool tabValueVisible;
    QList<QString> m_extendList;
    QList<QString> m_addList;
    QMap<QString, bool> m_addVisibleMap;
    QMap<QString, bool> m_extendVisibleMap;
    QMap<QString, QIcon> m_addIconMap;
    QMap<QString, QIcon> m_extendIconMap;
    bool addPress;
    bool extendPress;
};

class KNavigationBarPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KNavigationBar)

public:
    KNavigationBarPrivate(KNavigationBar *parent);
    void changeTheme();
    QString setTagForItem(QStandardItem *item);

private:
    KNavigationBar *q_ptr;
    ListView *m_pView;
    QStandardItemModel *m_pModel;
    Delegate *m_pDelegate;
    QMap<QString, QList<QStandardItem *>> m_itemMap;
    int num;
    QModelIndex m_index;
    bool m_indexFlag;
};

KNavigationBar::KNavigationBar(QWidget *parent)
    : QScrollArea(parent)
    , d_ptr(new KNavigationBarPrivate(this))
{
    Q_D(KNavigationBar);
    d->m_pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_pView->verticalScrollBar()->setProperty("drawScrollBarGroove",false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->num = 0;
    d->m_indexFlag = false;
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    // d->m_pView = new ListView (this);
    d->m_pView->setResizeMode(QListView::Adjust);
    // d->m_pModel = new QStandardItemModel(d->m_pView);
    d->m_pView->setModel(d->m_pModel);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setSizeConstraint(QLayout::SizeConstraint::SetMaximumSize);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(d->m_pView);

    QWidget *vp = viewport();
    vp->setLayout(vLayout);
    setWidget(d->m_pView);

    d->m_pView->setFocus();
    QPalette p = this->palette();
    QColor color(0, 0, 0, 0);
    p.setColor(QPalette::Base, color);
    d->m_pView->setPalette(p);
    this->setPalette(p);
    d->m_pView->setFrameStyle(0);
    //    d->m_pDelegate = new Delegate(this,d->m_pView);
    d->m_pView->setItemDelegate(d->m_pDelegate);
    d->m_pView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setFrameStyle(0);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    d->changeTheme();
    connect(d->m_pDelegate->m_gsetting, &QGSettings::changed, this, [=]() {
        d->changeTheme();
    });
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=]() {
        //        if(Parmscontroller::isTabletMode()) //解决导航栏滑动条切换主题为白条
        //         d->m_pView->setStyleSheet("QListView item {height : 48}");
        //        else
        //         d->m_pView->setStyleSheet("QListView item {height : 36}");
        updateGeometry();
    });
    connect(d->m_pDelegate, &Delegate::addClicked, this, &KNavigationBar::addTagClicked);
    connect(d->m_pDelegate, &Delegate::extendClicked, this, [=](QString str, int row) {
        if (d->m_itemMap.contains(str)) {
            QStringList list;
            for (auto item : d->m_itemMap[str]) {
                list.append(item->data(Qt::DisplayRole).toString());
            }

            if (list.contains(d->m_pView->currentIndex().data(Qt::DisplayRole).toString())) // 选中的为group中
            {
                d->m_index = d->m_pView->currentIndex();
                d->m_pView->setCurrentIndex(QModelIndex());
            } else if (!list.contains(d->m_pView->currentIndex().data(Qt::DisplayRole).toString()) && d->m_pView->currentIndex().isValid()) // 选中的为group外
            {
                d->m_index = d->m_pView->currentIndex();
                d->m_pView->setCurrentIndex(QModelIndex());
            }

            for (auto item : d->m_itemMap[str]) {
                bool flag = item->data(boolRole).toBool();
                if (flag) {
                    item->setData(false, boolRole);
                    d->m_pModel->takeRow(item->row());
                    d->m_pDelegate->setExtendIcon(str, QIcon::fromTheme("ukui-down.symbolic"));
                    if (list.contains(d->m_index.data(Qt::DisplayRole).toString())) // 选中的group中item才更改标志位
                        d->m_indexFlag = true;
                } else {
                    item->setData(true, boolRole);
                    d->m_pModel->insertRow(++row, item);
                    d->m_pDelegate->setExtendIcon(str, QIcon::fromTheme("ukui-up.symbolic"));
                    d->m_indexFlag = false;
                }
            }

            if (!d->m_indexFlag && d->m_index.isValid()) {
                d->m_pView->setCurrentIndex(d->m_index);
                d->m_index = QModelIndex();
            } else if (d->m_indexFlag && d->m_index.isValid()) {
                d->m_pView->setCurrentIndex(QModelIndex());
            }
        }
    });

    setMouseTracking(true);
}

void KNavigationBar::addItem(QStandardItem *item)
{
    Q_D(KNavigationBar);
    item->setData(ItemType::StandardItem, Qt::UserRole);
    item->setData(true, boolRole);
    d->m_pModel->appendRow(item);
    d->num++;
    d->setTagForItem(item);
}

void KNavigationBar::addSubItem(QStandardItem *subItem)
{
    Q_D(KNavigationBar);
    subItem->setData(ItemType::SubItem, Qt::UserRole);
    subItem->setData(true, boolRole);
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QIcon icon(pix);
    subItem->setIcon(icon);
    d->m_pModel->appendRow(subItem);
    d->num++;
    d->setTagForItem(subItem);
}

void KNavigationBar::addGroupItems(QList<QStandardItem *> items, const QString &tag)
{
    Q_D(KNavigationBar);
    QStandardItem *item = new QStandardItem(tag);

    QList<QStandardItem *> itemList;
    item->setEnabled(false);
    item->setData(ItemType::TagItem, Qt::UserRole);
    item->setData(true, boolRole);
    d->num++;
    QFileInfo cmdInfo(QApplication::arguments().at(0));
    QString accessibleName = cmdInfo.fileName();
    accessibleName += "_KNavigationBar_tag_" + tag;
    item->setAccessibleText(accessibleName);

    d->m_pModel->appendRow(item);
    for (auto item : items) {
        item->setData(true, boolRole);
        item->setData(ItemType::StandardItem, Qt::UserRole);
        itemList.append(item);
        d->m_pModel->appendRow(item);
        d->num++;
        d->setTagForItem(item);
    }
    d->m_itemMap[tag] = itemList;
}

void KNavigationBar::insertGroupItems(QString tag, int row, QStandardItem *item)
{
    Q_D(KNavigationBar);
    if (d->m_itemMap.contains(tag)) {
        QList<QStandardItem *> itemList = d->m_itemMap[tag];
        if (!itemList.contains(item)) {
            if ((0 <= row && row < itemList.count())) {
                QStandardItem *privItem = itemList.at(row);
                item->setData(itemList.at(0)->data(boolRole).toBool(), boolRole);
                itemList.insert(row, item);
                d->m_itemMap[tag] = itemList;
                d->m_pModel->insertRow(privItem->row(), item);
            } else if (0 <= row && row == itemList.count()) {
                for (int insertRow = 0; insertRow < d->m_pModel->rowCount(); ++insertRow) {
                    QStandardItem *privItem = d->m_pModel->item(insertRow);
                    if (privItem && privItem->data(Qt::DisplayRole).toString() == tag) {
                        item->setData(itemList.at(0)->data(boolRole).toBool(), boolRole);
                        itemList.insert(insertRow, item);
                        d->m_itemMap[tag] = itemList;
                        d->m_pModel->insertRow(privItem->row() + itemList.count(), item);
                        break;
                    }
                }
            }
        }
    }
}

void KNavigationBar::removeGroupItem(QString tag, QStandardItem *item)
{
    Q_D(KNavigationBar);
    if (d->m_itemMap.contains(tag)) {
        QList<QStandardItem *> itemList = d->m_itemMap[tag];
        if (itemList.contains(item)) {
            itemList.removeOne(item);
            d->m_itemMap[tag] = itemList;

            for (int row = 0; row < d->m_pModel->rowCount(); ++row) {
                QStandardItem *modelItem = d->m_pModel->item(row);
                if (modelItem == item) {
                    d->m_pModel->takeRow(modelItem->row());
                }
            }
        }
    }
}

void KNavigationBar::addTag(const QString &tag)
{
    Q_D(KNavigationBar);
    QStandardItem *item = new QStandardItem(tag);
    d->num++;
    d->setTagForItem(item);
    QFileInfo cmdInfo(QApplication::arguments().at(0));
    QString accessibleName = cmdInfo.fileName();
    accessibleName += "_KNavigationBar_tag_" + tag;
    item->setAccessibleText(accessibleName);

    item->setEnabled(false);
    item->setData(ItemType::TagItem, Qt::UserRole);
    item->setData(true, boolRole);
    d->m_pModel->appendRow(item);
}

QStandardItemModel *KNavigationBar::model()
{
    Q_D(KNavigationBar);
    return d->m_pModel;
}

QListView *KNavigationBar::listview()
{
    Q_D(KNavigationBar);
    return d->m_pView;
}

void KNavigationBar::setTabValue(QStandardItem *item, QString value)
{
    Q_D(KNavigationBar);
    item->setData(value, ValueRole);
    update();
}

void KNavigationBar::setTabValueVisible(bool visible)
{
    Q_D(KNavigationBar);
    d->m_pDelegate->setTabValueVisible(visible);
}

void KNavigationBar::setAddIcon(QString tag, QIcon icon)
{
    Q_D(KNavigationBar);
    d->m_pDelegate->setAddIcon(tag, icon);
}

void KNavigationBar::setAddTagVisible(QString tag, bool visible)
{
    Q_D(KNavigationBar);
    d->m_pDelegate->setAddTagVisible(tag, visible);
}

void KNavigationBar::setExtendTagVisible(QString tag, bool visible)
{
    Q_D(KNavigationBar);
    d->m_pDelegate->setExtendTagVisible(tag, visible);
}

KNavigationBarPrivate::KNavigationBarPrivate(KNavigationBar *parent)
    : q_ptr(parent)
{
    Q_Q(KNavigationBar);
    setParent(parent);
    m_pView = new ListView(q);
    m_pModel = new QStandardItemModel(m_pView);
    m_pDelegate = new Delegate(q, m_pView);
}

void KNavigationBarPrivate::changeTheme()
{
    Q_Q(KNavigationBar);
    m_pDelegate->initThemeStyle();
}

QString KNavigationBarPrivate::setTagForItem(QStandardItem *item)
{
    QFileInfo cmdInfo(QApplication::arguments().at(0));
    QString accessibleName = cmdInfo.fileName();
    accessibleName += "_";
    accessibleName += "QStandardItem";
    accessibleName += "_";
    accessibleName += item->data(0).toString();
    item->setAccessibleText(accessibleName);
    return accessibleName;
}

Delegate::Delegate(QObject *parent, ListView *view)
    : QStyledItemDelegate(parent)
    , m_listView(view)
{
    tabValueVisible = false;
    addPress = false;
    extendPress = false;
    m_listView->setMouseTracking(true);
}

void Delegate::setTabValueVisible(bool visible)
{
    tabValueVisible = visible;
}

void Delegate::setAddIcon(QString tag, QIcon icon)
{
    m_addIconMap[tag] = icon;
}

void Delegate::setAddTagVisible(QString tag, bool visible)
{
    if (!m_addList.contains(tag))
        m_addList.append(tag);
    if (!m_addIconMap.contains(tag))
        m_addIconMap[tag] = QIcon::fromTheme("list-add.symbolic");
    m_addVisibleMap[tag] = visible;
}

void Delegate::setExtendIcon(QString tag, QIcon icon)
{
    m_extendIconMap[tag] = icon;
}

void Delegate::setExtendTagVisible(QString tag, bool visible)
{
    if (!m_extendList.contains(tag))
        m_extendList.append(tag);
    if (!m_extendIconMap.contains(tag))
        m_extendIconMap[tag] = QIcon::fromTheme("ukui-up.symbolic");
    m_extendVisibleMap[tag] = visible;
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLinearGradient m_linearGradient;
    QRectF rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y() + 1);
    rect.setWidth(m_listView->width() - 16);
    //    rect.setHeight(option.rect.height()-2);
    rect.setHeight(option.rect.height() - Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatInterval));

    // QPainterPath画圆角矩形
    qreal radius = ThemeController::getRadiusFromDT("kradius-normal"); // 圆角半径6px
    if (radius == -1)
        radius = 6;
    QPainterPath path;
    QPalette palette = option.palette;
    if (ThemeController::widgetTheme() == ClassicTheme) {
        rect = rect.adjusted(1, 0, -1, 0);
        path.moveTo(rect.topRight());
        path.lineTo(rect.bottomRight());
        path.lineTo(rect.bottomLeft());
        path.lineTo(rect.topLeft());
        path.lineTo(rect.topRight());
    } else {
        path.moveTo(rect.topRight() - QPointF(radius, 0));
        path.lineTo(rect.topLeft() + QPointF(radius, 0));
        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
        path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
        path.lineTo(rect.bottomRight() - QPointF(radius, 0));
        path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
        path.lineTo(rect.topRight() + QPointF(0, radius));
        path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));
    }
    int flag = index.model()->data(index, Qt::UserRole).toInt();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    QColor color;
    QColor valueColor = ThemeController::getPaletteColorFromDT("windowtext-active");
    QColor borColor;
    m_linearGradient = QLinearGradient(rect.width() / 2, rect.y(), rect.width() / 2, rect.height() + rect.y());
    if (!(option.state & QStyle::State_Enabled))
    {
        color = ThemeController::getCustomColorFromDT("kfont-primary-disable");
        borColor = QColor(0, 0, 73);
    }
    else if (((m_listView->currentIndex() == index) || (option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver)) && flag != 2)
    {
        if ((m_listView->currentIndex() == index) || (option.state & QStyle::State_Selected))
        {
            color = ThemeController::getPaletteColorFromDT("highlight-active");
            borColor = ThemeController::getCustomColorFromDT("kline-brand-normal");
        }
        else
        {
            // hover时
            color = ThemeController::getCustomColorFromDT("kcomponent-hover");
            borColor = ThemeController::getCustomColorFromDT("kline-component-normal");
        }
        painter->save();
        painter->setPen(borColor);
        painter->setBrush(color);
        if (painter->pen().width() == 1)
            painter->translate(0.5, 0.5);
        painter->drawPath(path);
        painter->restore();
    }

    switch (flag) {
    case 0: // standardItem
    {
        QRect iconRect;
        if (ThemeController::systemLang())
            iconRect = QRect(rect.right() - 32, rect.y() + (rect.height() - 16) / 2, 16, 16); // 图片大小16*16 左边距16
        else
            iconRect = QRect(rect.x() + 16, rect.y() + (rect.height() - 16) / 2, 16, 16);
        auto *model = dynamic_cast<QStandardItemModel *>(const_cast<QAbstractItemModel *>(index.model()));
        auto icon = model->item(index.row())->icon();

        if (!icon.pixmap(16, 16).isNull()) {
            if (ThemeController::isPixmapPureColor(icon.pixmap(16, 16))) {
                if (ThemeController::themeMode() == DarkTheme)
                    icon = ThemeController::drawSymbolicColoredPixmap(icon.pixmap(16, 16));
                if ((m_listView->currentIndex() == index) || (option.state & QStyle::State_Selected))
                    if (ThemeController::widgetTheme() != ClassicTheme)
                        icon = ThemeController::drawColoredPixmap(icon.pixmap(16, 16), QColor(255, 255, 255));
                if (!(option.state & QStyle::State_Enabled))
                    icon = ThemeController::drawColoredPixmap(icon.pixmap(16, 16), QColor("#FF979797"));
            }
        }
        icon.paint(painter, iconRect);
        QFontMetrics fm = painter->fontMetrics();

        QString mainText = index.data(Qt::DisplayRole).toString();
        QString subText = index.data(Qt::UserRole + 1).toString();
        QString elidedText;
        QRect textRect, subRect;

        if (subText.isEmpty()) {
            elidedText = fm.elidedText(mainText, Qt::ElideRight, rect.width() - 56); // 左边距+图片宽度+文本图片间距+右边距
            if (fm.width(mainText) > rect.width() - 56)
                model->item(index.row())->setToolTip(mainText);

            if (!icon.isNull()) {
                textRect = QRect(iconRect.right() + 8, rect.y(), rect.width() - 56, rect.height());
            } else {
                textRect = QRect(rect.x() + 16, rect.y(), rect.width() - 56, rect.height());
            }
        } else {
            elidedText = fm.elidedText(mainText, Qt::ElideRight, rect.width() - 56 - fm.width(subText));
            subRect = QRect(rect.right() - 8 - fm.width(subText), rect.y(), fm.width(subText), rect.height());

            if (fm.width(mainText) > rect.width() - 56 - fm.width(subText))
                model->item(index.row())->setToolTip(mainText);

            if (!icon.isNull()) {
                textRect = QRect(iconRect.right() + 8, rect.y(), rect.width() - 56 - fm.width(subText), rect.height());
            } else {
                textRect = QRect(rect.x() + 16, rect.y(), rect.width() - 56 - fm.width(subText), rect.height());
            }
        }

        painter->save();
        if ((m_listView->currentIndex() == index) || (option.state & QStyle::State_Selected)) {
            valueColor = QColor(255, 255, 255);
            painter->setPen(ThemeController::getPaletteColorFromDT("highlightedtext-active"));
        }
        QFont font;
        font.setPointSize(ThemeController::systemFontSize());
        if (!(option.state & QStyle::State_Enabled)) {
            painter->setPen(color);
        }
        painter->setFont(font);
        if (!subText.isEmpty() && tabValueVisible) {
            if (ThemeController::themeMode() == LightTheme)
                valueColor.setAlphaF(0.4);
            painter->save();
            painter->setPen(valueColor);
            painter->drawText(subRect, Qt::AlignVCenter, subText);
            painter->restore();
        }
        if (!icon.isNull()) {
            if (ThemeController::systemLang()) {

                painter->drawText(QRect(rect.left() + 16, rect.y(),
                                        rect.width() - 56, rect.height()),
                                  Qt::AlignVCenter, elidedText); // 文本 图片间距8px
            } else
                painter->drawText(QRect(iconRect.right() + 8, rect.y(),
                                        rect.width() - 56, rect.height()),
                                  Qt::AlignVCenter, elidedText);
        } else {
            if (ThemeController::systemLang())
                painter->drawText(QRect(rect.x() + 16, rect.y(),
                                        rect.width() - 56, rect.height()),
                                  Qt::AlignVCenter, elidedText);
            else
                painter->drawText(QRect(rect.x() + 16, rect.y(),
                                        rect.width() - 56, rect.height()),
                                  Qt::AlignVCenter, elidedText);
        }
        painter->restore();
        break;
    }
    case 1: // subItem
    {
        QRect iconRect = QRect(rect.x() + 16, rect.y() + (rect.height() - 16) / 2, 16, 16); // 图片大小16*16 左边距16
        auto *model = dynamic_cast<QStandardItemModel *>(const_cast<QAbstractItemModel *>(index.model()));

        QFontMetrics fm = painter->fontMetrics();
        QString mainText = index.data(Qt::DisplayRole).toString();
        QString subText = index.data(Qt::UserRole + 1).toString();
        QString elidedText;
        QRect textRect, subRect;

        if (subText.isEmpty()) {
            elidedText = fm.elidedText(mainText, Qt::ElideRight, rect.width() - 56); // 左边距+图片宽度+文本图片间距+右边距
            textRect = QRect(iconRect.right() + 8, rect.y(), rect.width() - 56, rect.height());

            if (fm.width(mainText) > rect.width() - 56)
                model->item(index.row())->setToolTip(mainText);
        } else {
            elidedText = fm.elidedText(mainText, Qt::ElideRight, rect.width() - 56 - fm.width(subText));
            textRect = QRect(iconRect.right() + 8, rect.y(), rect.width() - 56 - fm.width(subText), rect.height());
            subRect = QRect(rect.right() - 8 - fm.width(subText), rect.y(), fm.width(subText), rect.height());
            if (fm.width(mainText) > rect.width() - 56 - fm.width(subText))
                model->item(index.row())->setToolTip(mainText);
        }
        painter->save();
        if ((option.state & QStyle::State_Selected)) {
            valueColor = QColor(255, 255, 255);
            painter->setPen(ThemeController::getPaletteColorFromDT("highlightedtext-active"));
        }
        QFont font;
        font.setPointSize(ThemeController::systemFontSize());
        if (!(option.state & QStyle::State_Enabled)) {
            painter->setPen(color);
        }
        painter->setFont(font);
        if (!subText.isEmpty() && tabValueVisible) {
            if (ThemeController::themeMode() == LightTheme)
                valueColor.setAlphaF(0.4);
            painter->setPen(valueColor);
            painter->drawText(subRect, Qt::AlignVCenter, subText);
        }
        if (ThemeController::systemLang())
            painter->drawText(QRect(rect.x() + 16, rect.y(),
                                    rect.width() - 56, rect.height()),
                              Qt::AlignVCenter, elidedText); // 文本 图片间距8px
        else
            painter->drawText(QRect(iconRect.right() + 8, rect.y(),
                                    rect.width() - 56, rect.height()),
                              Qt::AlignVCenter, elidedText);
        painter->restore();
        break;
    }
    case 2: // tagItem
    {
        painter->save();
        auto *model = dynamic_cast<QStandardItemModel *>(const_cast<QAbstractItemModel *>(index.model()));
        QFontMetrics fm = painter->fontMetrics();
        QString elidedText = fm.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, rect.width() - 56); // 左边距+图片宽度+文本图片间距+右边距

        QString mainText = index.data(Qt::DisplayRole).toString();
        if (fm.width(mainText) > rect.width() - 56)
            model->item(index.row())->setToolTip(mainText);
        painter->setPen(ThemeController::getCustomColorFromDT("kfont-secondary"));

        if (m_addList.contains(mainText) || m_extendList.contains(mainText)) {
            QPixmap extendPixmap = m_extendIconMap.value(mainText).pixmap(16, 16);
            QPixmap addPixmap = m_addIconMap.value(mainText).pixmap(16, 16);
            if (ThemeController::themeMode() == DarkTheme) {
                extendPixmap = ThemeController::drawColoredPixmap(extendPixmap, ThemeController::getPaletteColorFromDT("windowtext-active"));
                addPixmap = ThemeController::drawColoredPixmap(addPixmap, ThemeController::getPaletteColorFromDT("windowtext-active"));
            }
            if (m_addVisibleMap.value(mainText)) {
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::NoBrush);

                if (m_extendVisibleMap.value(mainText)) {
                    QRect extendRect; // = option.rect.adjusted(option.rect.width() - 28 , 22 , -12 , -10 );
                    QRect extendBkgRect; // = option.rect.adjusted(option.rect.width() - 32 , 18 , -8 , -6 );
                    if (m_listView->verticalScrollBar()->isVisible()) {
                        extendRect = option.rect.adjusted(option.rect.width() - 28, 22, -12, -10);
                        extendBkgRect = option.rect.adjusted(option.rect.width() - 32, 18, -8, -6);
                    } else {
                        extendRect = option.rect.adjusted(option.rect.width() - 28 - 16, 22, -12 - 16, -10);
                        extendBkgRect = option.rect.adjusted(option.rect.width() - 32 - 16, 18, -8 - 16, -6);
                    }
                    if (extendPress && extendBkgRect.contains(m_listView->mousePoint()))
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.2));
                    else if (extendBkgRect.contains(m_listView->mousePoint())) {
                        if (m_extendIconMap.value(mainText).name().contains("ukui-down.symbolic"))
                            QToolTip::showText(QCursor::pos(), tr("Expand"), qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                        else
                            QToolTip::showText(QCursor::pos(), tr("Fold"), qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.05));
                    }
                    painter->drawRoundedRect(extendBkgRect, 6, 6);
                    painter->drawPixmap(extendRect, extendPixmap);
                    painter->setBrush(Qt::NoBrush);

                    QRect addRect; // = option.rect.adjusted(option.rect.width() - 60, 22 , -44 , -10 );
                    QRect addBkgRect; // = option.rect.adjusted(option.rect.width() - 64, 18 , -40 , -6 );
                    if (m_listView->verticalScrollBar()->isVisible()) {
                        addRect = option.rect.adjusted(option.rect.width() - 60, 22, -44, -10);
                        addBkgRect = option.rect.adjusted(option.rect.width() - 64, 18, -40, -6);
                    } else {
                        addRect = option.rect.adjusted(option.rect.width() - 60 - 16, 22, -44 - 16, -10);
                        addBkgRect = option.rect.adjusted(option.rect.width() - 64 - 16, 18, -40 - 16, -6);
                    }

                    if (addPress && addBkgRect.contains(m_listView->mousePoint())) {
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.2));
                    } else if (addBkgRect.contains(m_listView->mousePoint())) {
                        QToolTip::showText(QCursor::pos(), tr("Add tag"), qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.05));
                    }

                    if (!addBkgRect.contains(m_listView->mousePoint()) && !extendBkgRect.contains(m_listView->mousePoint())) {
                        QToolTip::showText(QCursor::pos(), "", qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                    }
                    painter->drawRoundedRect(addBkgRect, 6, 6);
                    painter->drawPixmap(addRect, addPixmap);
                } else {
                    QRect addRect; // = option.rect.adjusted(option.rect.width() - 28 , 22 , -12 , -10 );
                    QRect addBkgRect; // = option.rect.adjusted(option.rect.width() - 32 , 18 , -8 , -6);
                    if (m_listView->verticalScrollBar()->isVisible()) {
                        addRect = option.rect.adjusted(option.rect.width() - 28, 22, -12, -10);
                        addBkgRect = option.rect.adjusted(option.rect.width() - 32, 18, -8, -6);
                    } else {
                        addRect = option.rect.adjusted(option.rect.width() - 28 - 16, 22, -12 - 16, -10);
                        addBkgRect = option.rect.adjusted(option.rect.width() - 32 - 16, 18, -8 - 16, -6);
                    }
                    if (addPress && addBkgRect.contains(m_listView->mousePoint())) {
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.2));
                    } else if (addBkgRect.contains(m_listView->mousePoint())) {
                        QToolTip::showText(QCursor::pos(), tr("Add tag"), qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.05));
                    }

                    if (!addBkgRect.contains(m_listView->mousePoint())) {
                        QToolTip::showText(QCursor::pos(), "", qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                    }
                    painter->drawRoundedRect(addBkgRect, 6, 6);
                    painter->drawPixmap(addRect, addPixmap);
                }
                painter->restore();
            } else {
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
                painter->setBrush(Qt::NoBrush);
                QPixmap extendPixmap = m_extendIconMap.value(mainText).pixmap(16, 16);
                if (ThemeController::themeMode() == DarkTheme) {
                    extendPixmap = ThemeController::drawSymbolicColoredPixmap(extendPixmap);
                }

                if (m_extendVisibleMap.value(mainText)) {
                    QRect extendRect; // = option.rect.adjusted(option.rect.width() - 28 , 22 , -12 , -10 );
                    QRect extendBkgRect; // = option.rect.adjusted(option.rect.width() - 32 , 18 , -8 , -6 );
                    if (m_listView->verticalScrollBar()->isVisible()) {
                        extendRect = option.rect.adjusted(option.rect.width() - 28, 22, -12, -10);
                        extendBkgRect = option.rect.adjusted(option.rect.width() - 32, 18, -8, -6);
                    } else {
                        extendRect = option.rect.adjusted(option.rect.width() - 28 - 16, 22, -12 - 16, -10);
                        extendBkgRect = option.rect.adjusted(option.rect.width() - 32 - 16, 18, -8 - 16, -6);
                    }
                    if (extendPress && extendBkgRect.contains(m_listView->mousePoint()))
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.2));
                    else if (extendBkgRect.contains(m_listView->mousePoint())) {
                        if (m_extendIconMap.value(mainText).name().contains("ukui-down.symbolic"))
                            QToolTip::showText(QCursor::pos(), tr("Expand"), qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                        else
                            QToolTip::showText(QCursor::pos(), tr("Fold"), qobject_cast<QWidget *>(const_cast<QWidget *>(option.widget)));
                        painter->setBrush(ThemeController::mixColor(ThemeController::getPaletteColorFromDT("button-active"), ThemeController::getPaletteColorFromDT("brighttext-active"), 0.05));
                    }
                    if (!extendBkgRect.contains(m_listView->mousePoint())) {
                        QToolTip::showText(QCursor::pos(), "");
                    }
                    painter->drawRoundedRect(extendBkgRect, 6, 6);
                    painter->drawPixmap(extendRect, extendPixmap);
                }
                painter->restore();
            }
        }
        if (ThemeController::themeMode() == ThemeFlag::DarkTheme)
            painter->setPen(QColor(115, 115, 115));
        else
            painter->setPen(QColor(140, 140, 140));
        QRect textRect = option.rect;
        textRect = textRect.adjusted(16, 12, 0, 0);
        QFont font;
        font.setPointSize(ThemeController::systemFontSize());
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter, elidedText);
        painter->restore();
        break;
    }
    default:
        break;
    }
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int flag = index.model()->data(index, Qt::UserRole).toInt();
    QSize size;
    switch (flag) {
    case 2: // tagItem
        size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatHeight) + 12);
        break;
    default:
        size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatHeight) + Parmscontroller::parm(Parmscontroller::Parm::PM_NavigationBatInterval));
        break;
    }
    size.setWidth(m_listView->width() - 16);
    return size;
}

bool Delegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Enter) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QRect addRect;
        QRect extendRect;
        QString mainText = index.data(Qt::DisplayRole).toString();
        if (m_addList.contains(mainText) || m_extendList.contains(mainText)) {
            if (m_addVisibleMap.value(mainText)) {
                if (m_extendVisibleMap.value(mainText)) {
                    if (m_listView->verticalScrollBar()->isVisible()) {
                        extendRect = option.rect.adjusted(option.rect.width() - 32, 18, -8, -6);
                        addRect = option.rect.adjusted(option.rect.width() - 64, 18, -40, -6);
                    } else {
                        extendRect = option.rect.adjusted(option.rect.width() - 32 - 16, 18, -8 - 16, -6);
                        addRect = option.rect.adjusted(option.rect.width() - 64 - 16, 18, -40 - 16, -6);
                    }

                    if (extendRect.contains(mouseEvent->pos())) {
                        if (event->type() == QEvent::MouseButtonPress) {
                            extendPress = true;
                            emit extendClicked(mainText, index.row());
                        } else {
                            extendPress = false;
                        }
                    }
                    if (addRect.contains(mouseEvent->pos())) {
                        if (event->type() == QEvent::MouseButtonPress) {
                            addPress = true;
                            emit addClicked(mainText);
                        } else {
                            addPress = false;
                        }
                    }

                } else {
                    if (m_listView->verticalScrollBar()->isVisible())
                        addRect = option.rect.adjusted(option.rect.width() - 32, 18, -8, -6);
                    else
                        addRect = option.rect.adjusted(option.rect.width() - 32 - 16, 18, -8 - 16, -6);
                    if (addRect.contains(mouseEvent->pos())) {
                        if (event->type() == QEvent::MouseButtonPress) {
                            addPress = true;
                            emit addClicked(mainText);
                        } else {
                            addPress = false;
                        }
                    }
                }
            } else {
                if (m_extendVisibleMap.value(mainText)) {
                    if (m_listView->verticalScrollBar()->isVisible())
                        extendRect = option.rect.adjusted(option.rect.width() - 32, 18, -8, -6);
                    else
                        extendRect = option.rect.adjusted(option.rect.width() - 32 - 16, 18, -8 - 16, -6);
                    if (extendRect.contains(mouseEvent->pos())) {
                        if (event->type() == QEvent::MouseButtonPress) {
                            extendPress = true;
                            emit extendClicked(mainText, index.row());
                        } else {
                            extendPress = false;
                        }
                    }
                }
            }
        }
    } else {
        addPress = false;
        extendPress = false;
    }
    m_listView->viewport()->update();
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

ListView::ListView(QWidget *parent)
    : QListView(parent)
{
    setMouseTracking(true);
}

QPoint ListView::mousePoint()
{
    return m_mousePoint;
}

void ListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton)
        return;
    else if (event->button() & Qt::LeftButton)
        QListView::mousePressEvent(event);
}

void ListView::mouseMoveEvent(QMouseEvent *e)
{
    m_mousePoint = e->pos();
    QListView::mouseMoveEvent(e);
}

void ListView::leaveEvent(QEvent *event)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    m_mousePoint = mouseEvent->pos();
    QListView::leaveEvent(event);
}

}

#include "knavigationbar.moc";
#include "moc_knavigationbar.cpp";
