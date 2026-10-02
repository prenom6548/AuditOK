/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "blockwidget.h"
#include "customlabel.h"
#include "statusbutton.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "powerlistwidget.h"
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

#define SHUTDOWN_MSG_TIME 60

BlockWidget::BlockWidget(LockDialogModel *model, QWidget *parent) : QWidget(parent), m_modelLockDialog(model)
{
    initUi();
    setFocusPolicy(Qt::NoFocus);
}

BlockWidget::~BlockWidget() {}

void BlockWidget::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    this->setLayout(mainLayout);

    m_tipLabel1 = new CustomLabel(this);
    m_tipLabel1->setAlignment(Qt::AlignCenter);
    m_tipLabel1->setWordWrap(true);
    m_tipLabel2 = new CustomLabel(this);
    m_tipLabel2->setAlignment(Qt::AlignCenter);
    m_tipLabel2->setWordWrap(true);
    m_tipLabel2->setFontSize(10);
    m_msgTipLabel = new CustomLabel(this);
    m_msgTipLabel->setAlignment(Qt::AlignCenter);
    QHBoxLayout *listLayout = new QHBoxLayout();
    listLayout->setAlignment(Qt::AlignCenter);
    m_listView = new QListView(this);
    m_listView->setObjectName(QString::fromUtf8("applist"));
    listLayout->addWidget(m_listView);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_cancelButton = new QPushButton(this);
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setFocusPolicy(Qt::TabFocus);
    buttonLayout->addWidget(m_cancelButton);
    m_confirmButton = new QPushButton(this);
    m_confirmButton->setText(tr("Confirm"));
    m_confirmButton->setFocusPolicy(Qt::TabFocus);
    buttonLayout->addWidget(m_confirmButton);
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->setSpacing(46);

    mainLayout->addStretch();
    mainLayout->addWidget(m_tipLabel1);
    mainLayout->addWidget(m_tipLabel2);
    mainLayout->addWidget(m_msgTipLabel);
    //    mainLayout->addSpacing(0);
    QSpacerItem *m_dynamicSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Fixed);
    mainLayout->addItem(m_dynamicSpacer);
    mainLayout->addLayout(listLayout);
    mainLayout->addSpacing(46);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    m_curFontSize = m_modelLockDialog->getCurFontSize();
    m_ptToPx = m_modelLockDialog->getPtToPx();
    connect(m_modelLockDialog, &LockDialogModel::fontSizeChanged, this, &BlockWidget::onFontSizeChanged);
    m_tipLabel1->setFontSize((18 + m_curFontSize) * m_ptToPx);

    connect(m_cancelButton, &QPushButton::clicked, this, [&]() {
        if (shutdownTimer) {
            shutdownTimer->stop();
        }
        emit cancelButtonclicked();
    });
    connect(m_confirmButton, &QPushButton::clicked, this, [&]() { emit confirmButtonclicked(); });
}

void BlockWidget::hideEvent(QHideEvent *event)
{
    if (m_msgTipLabel) {
        m_msgTipLabel->hide();
    }
    if (shutdownTimer) {
        shutdownTimer->stop();
    }
}

void BlockWidget::onFontSizeChanged(double fontSize)
{
    m_curFontSize = fontSize;
    m_tipLabel1->setFontSize((18 + m_curFontSize) * m_ptToPx);

    sysFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
    m_confirmButton->setFont(sysFont);
    m_cancelButton->setFont(sysFont);
    adjustButtonText();
}

void BlockWidget::updateFontFamily(QString fontFamily)
{
    if (m_tipLabel1) {
        m_tipLabel1->setFontFamily(fontFamily);
    }
    sysFont.setFamily(fontFamily);
    if (m_cancelButton) {
        m_cancelButton->setFont(sysFont);
    }
    if (m_confirmButton) {
        m_confirmButton->setFont(sysFont);
    }
    adjustButtonText();
}

void BlockWidget::setConfirmButton(bool isHidden)
{
    m_confirmButton->setVisible(!isHidden);
}

void BlockWidget::adjustButtonText()
{
    if (m_cancelButton) {
        QString cancelStrEText = getElidedText(
            m_cancelButton->font(),
            m_cancelButton->width() - m_cancelButton->contentsMargins().left()
                - m_cancelButton->contentsMargins().right(),
            tr("Cancel"));
        if (cancelStrEText == tr("Cancel")) {
            m_cancelButton->setText(tr("Cancel"));
        } else if (cancelStrEText != tr("Cancel")) {
            m_cancelButton->setText(cancelStrEText);
            m_cancelButton->setToolTip(tr("Cancel"));
        }
    }
    if (m_confirmButton) {
        QString text = tr("Confirm");
        switch (m_type) {
            case REBOOT:
            case 5:
                text = tr("Still reboot");
                break;
            case SHUTDOWN:
            case 6:
                text = tr("Still shutdown");
                break;
            case SLEEP:
                text = tr("Still suspend");
                break;
            case HIBERNATE:
                text = tr("Still hibernate");
                break;
            case LOGOUT:
            case 7:
                text = tr("Still logout");
                break;
            default:
                text = tr("Confirm");
                break;
        }
        QString strEText = getElidedText(
            m_confirmButton->font(),
            m_confirmButton->width() - m_confirmButton->contentsMargins().left()
                - m_confirmButton->contentsMargins().right(),
            text);
        if (strEText == text) {
            m_confirmButton->setText(text);
        } else if (strEText != text) {
            m_confirmButton->setText(strEText);
            m_confirmButton->setToolTip(text);
        }
    }
}

void BlockWidget::setTips(const QString tips, int type)
{
    m_type = type;
    if (m_msgTipLabel) {
        m_msgTipLabel->hide();
    }
    if (m_tipLabel2 && !m_tipLabel2->isHidden()) {
        m_tipLabel2->hide();
    }
    m_cancelButton->setFixedSize(120, 48);
    m_confirmButton->setFixedSize(120, 48);
    m_cancelButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 8px;color: white;}"
                                  "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius: 8px;}"
                                  "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 8px;}"
                                  "QPushButton:focus{background: rgba(255, 255, 255, 0.4);border-radius:8px;outline: none;}");
    m_confirmButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 8px;color: white;}"
                                   "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius: 8px;}"
                                   "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 8px;}"
                                   "QPushButton:focus{background: rgba(255, 255, 255, 0.4);border-radius:8px;outline: none;}");
    sysFont.setPointSize((20 + m_curFontSize) * m_ptToPx);
    m_confirmButton->setFont(sysFont);
    m_cancelButton->setFont(sysFont);
    m_confirmButton->show();
    m_tipLabel1->show();
    m_listView->hide();
    m_tipLabel1->setText(tips);
    adjustButtonText();
}

void BlockWidget::setMsgTips(int type)
{
    msg_type = type;
    QString tips = tr("If you do not perform any operation, the system will automatically %1 after %2 seconds.")
                       .arg(getHibited_tr_lowcase(msg_type))
                       .arg(SHUTDOWN_MSG_TIME);
    if (m_tipLabel2 && !m_tipLabel2->isHidden()) {
        m_tipLabel2->hide();
    }
    if (type == REBOOT) {
        m_confirmButton->setText(tr("Still reboot"));
    } else if (type == SHUTDOWN) {
        m_confirmButton->setText(tr("Still shutdown"));
    }
    m_msgTipLabel->setText(tips);
    m_msgTipLabel->show();

    lastDateTime = QDateTime::currentDateTime();
    if (!shutdownTimer) {
        shutdownTimer = new QTimer(this);
        shutdownTimer->setInterval(500);
        connect(shutdownTimer, &QTimer::timeout, this, [&] {
            int interval = (60 - lastDateTime.secsTo(QDateTime::currentDateTime()));
            if (interval <= 0) {
                shutdownTimer->stop();
                m_confirmButton->clicked();
                m_msgTipLabel->setText("");
            } else {
                m_msgTipLabel->setText(
                    tr("If you do not perform any operation, the system will automatically %1 after %2 seconds.")
                        .arg(getHibited_tr_lowcase(msg_type))
                        .arg(interval));
            }
        });
    }
    shutdownTimer->start(500);
}

void BlockWidget::setWarning(QVector<InhibitorInfo> list, int type)
{
    if (m_msgTipLabel) {
        m_msgTipLabel->hide();
    }
    switch (type) {
        case REBOOT:
            m_tipLabel1->setText(tr(
                "The following programs prevent restarting, you can click \"Cancel\" and then close these programs."));
            break;
        case SHUTDOWN:
            m_tipLabel1->setText(tr("The following programs prevent the shutdown, you can click \"Cancel\" and then "
                                   "close these programs."));
            break;
        case SLEEP:
            m_tipLabel1->setText(
                tr("The following programs prevent suspend, you can click \"Cancel\" and then close these programs."));
            break;
        case HIBERNATE:
            m_tipLabel1->setText(tr(
                "The following programs prevent hibernation, you can click \"Cancel\" and then close these programs."));
            break;
        case LOGOUT:
            m_tipLabel1->setText(tr(
                "The following programs prevent you from logging out, you can click \"Cancel\" and then close them."));
            break;
        case 5:
            m_tipLabel1->setText(
                tr("The following programs prevent you restarting, you can click \"Cancel\" and then close them, \
                or click 'OK' to force close and proceed."));
            break;
        case 6:
            m_tipLabel1->setText(
                tr("The following programs prevent you shutdown, you can click \"Cancel\" and then close them, \
                or click 'OK' to force close and proceed."));
            break;
        case 7:
            m_tipLabel1->setText(
                tr("The following programs prevent you from logging out, you can click \"Cancel\" and then close them, \
                 or click 'OK' to force close and proceed."));
            break;
        default:
            break;
    }
    m_listView->show();

    QStandardItemModel *model = new QStandardItemModel(this);
    QIcon icon;
    for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
        QIcon icon;
        QString iconName = it->icon;

        if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
            icon = QIcon::fromTheme(iconName);
        } else if (QIcon::hasThemeIcon("application-x-desktop")) {
            icon = QIcon::fromTheme("application-x-desktop");
        }
        QStandardItem *item = new QStandardItem(icon, it->name);
        model->appendRow(item);
    }

    m_listView->verticalScrollMode();
    m_listView->setStyleSheet(
        "QListView#applist{font:10pt;color:white;background-color: rgba(255,255,255,80);border-style: outset;border-width: 0px;border-radius: 6px;}\
                              QListView#applist::item{height:48px;margin-top:2px;border-radius: 6px;}\
                              QListView#applist::item::selected {background-color: rgba(255,255,255,80);border: 1px solid #296CD9;\
                              height:48px;margin-top:2px;border-radius: 6px;}\
                              QListView#applist::item::hover {background-color: rgba(255,255,255,80);height:48px;margin-top:2px;border-radius: 6px;}");

    sysFont.setPointSize((10 + m_curFontSize) * m_ptToPx);
    m_listView->setFont(sysFont);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setIconSize(QSize(32, 32));
    m_listView->setModel(model);
    m_listView->setFixedSize(520, 320);
    m_cancelButton->setFixedSize(120, 48);
    m_confirmButton->setFixedSize(120, 48);
    m_confirmButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 24px;color: white;}"
                                   "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius:24px;}"
                                   "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 24px;}"
                                   "QPushButton:focus{background: rgba(255, 255, 255, 0.4);border-radius:24px;outline: none;}");
    m_cancelButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 24px;color: white;}"
                                  "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius:24px;}"
                                  "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 24px;}"
                                  "QPushButton:focus{background: rgba(255, 255, 255, 0.4);border-radius:24px;outline: none;}");
    sysFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
    m_confirmButton->setFont(sysFont);
    m_cancelButton->setFont(sysFont);
    adjustButtonText();
}

void BlockWidget::setWarning(QVector<InhibitorInfo> list, QVector<WindowInfo::WindowInfo> &windows, int type)
{
    m_type = type;
    if (m_msgTipLabel) {
        m_msgTipLabel->hide();
    }
    if (m_tipLabel2 && m_tipLabel2->isHidden()) {
        m_tipLabel2->show();
    }
    QString subTextCmd= "";
    switch (type) {
        case REBOOT:
            m_tipLabel1->setText(tr("Please wait while applications close and the computer reboot."));
            m_tipLabel2->setText(tr("Cancel to complete your work and avoid data loss."));
            subTextCmd = tr("reboot");
            break;
        case SHUTDOWN:
            m_tipLabel1->setText(tr("Please wait while applications close and the computer shutdown."));
            m_tipLabel2->setText(tr("Cancel to complete your work and avoid data loss."));
            subTextCmd = tr("shutdown");
            break;
        case SLEEP:
            m_tipLabel1->setText(tr("Please wait while applications close and the computer suspend."));
            m_tipLabel2->setText(tr("Cancel to complete your work and avoid data loss."));
            subTextCmd = tr("sleep");
            break;
        case HIBERNATE:
            m_tipLabel1->setText(tr("Please wait while applications close and the computer hibernation."));
            subTextCmd = tr("hibernate");
            break;
        case LOGOUT:
            m_tipLabel1->setText(tr("Please wait while applications close and the computer logging out."));
            m_tipLabel2->setText(tr("Cancel to complete your work and avoid data loss."));
            subTextCmd = tr("log out");
            break;
        default:
            break;
    }
    m_listView->show();

    bool isHideen = false;
    QStandardItemModel *model = new QStandardItemModel(this);
    if (!list.isEmpty()) {
        for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
            QIcon icon;
            QString iconName = it->icon;

            if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
                icon = QIcon::fromTheme(iconName);
            } else if (QIcon::hasThemeIcon("application-x-desktop")) {
                icon = QIcon::fromTheme("application-x-desktop");
            }
            QStandardItem *item = new QStandardItem(icon, it->name);
            model->appendRow(item);
            if (it->uid == 0) {
                isHideen = true;
                QString subText = tr("This app prevents your computer from %1.").arg(subTextCmd);
                item->setData(subText, Qt::UserRole + 1);
            }
        }
    }

    if (!windows.isEmpty()) {
        for (auto it = windows.constBegin(); it != windows.constEnd(); ++it) {
            QIcon icon = QIcon::fromTheme(it->icon);
            QStandardItem *item = new QStandardItem(
                !icon.isNull() ?
                    icon : QIcon::fromTheme("application-x-desktop"),
                it->name
            );
            model->appendRow(item);
        }
    }

    setConfirmButton(isHideen);
    m_listView->verticalScrollMode();
    m_listView->setStyleSheet(
        "QListView#applist{font:10pt;color:white;background-color: rgba(255,255,255,80);border-style: outset;border-width: 0px;border-radius: 6px;}\
                              QListView#applist::item{height:48px;margin-top:2px;border-radius: 6px;}\
                              QListView#applist::item::selected {background-color: rgba(255,255,255,80);border: 1px solid #296CD9;\
                              height:48px;margin-top:2px;border-radius: 6px;}\
                              QListView#applist::item::hover {background-color: rgba(255,255,255,80);height:48px;margin-top:2px;border-radius: 6px;}");

    sysFont.setPointSize((10 + m_curFontSize) * m_ptToPx);
    m_listView->setFont(sysFont);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setIconSize(QSize(32, 32));
    m_listView->setModel(model);
    m_listView->setItemDelegate(new SimpleDelegate(this));
//    m_listView->update();
    m_listView->setFixedSize(520, 320);
    m_cancelButton->setFixedSize(120, 48);
    m_confirmButton->setFixedSize(120, 48);
    m_confirmButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 24px;color: white;}"
                                   "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius:24px;}"
                                   "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 24px;}"
                                   "QPushButton:focus{background: rgba(255, 255, 255, 0.4);border-radius:24px;outline: none;}");
    m_cancelButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 24px;color: white;}"
                                  "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius:24px;}"
                                  "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 24px;}"
                                  "QPushButton:focus{background: rgba(255, 255, 255, 0.4);border-radius:24px;outline: none;}");
    sysFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
    m_confirmButton->setFont(sysFont);
    m_cancelButton->setFont(sysFont);
    adjustButtonText();
}

QString BlockWidget::getHibited_tr_lowcase(int type)
{
    switch (type) {
        case SHUTDOWN:
            return tr("shut down");
            break;
        case REBOOT:
            return tr("restart");
            break;
        default:
            break;
    }
    return "";
}

QString BlockWidget::getElidedText(QFont font, int width, QString strInfo)
{
    QFontMetrics fontMetrics(font);
    // 如果当前字体下，字符串长度大于指定宽度
    if (fontMetrics.width(strInfo) > width) {
        strInfo = QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}

void BlockWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            if (!isHidden()) {
                Q_EMIT cancelButtonclicked();
            }
            break;
        case Qt::Key_Enter:
            if (m_confirmButton && m_confirmButton->isEnabled() && !m_confirmButton->isHidden() && m_confirmButton->hasFocus()) {
                m_confirmButton->clicked();
                m_confirmButton->setFocus();
            } else if (m_cancelButton && m_cancelButton->isEnabled() && !m_cancelButton->isHidden() && m_cancelButton->hasFocus()) {
                m_cancelButton->clicked();
            }
            break;
        case Qt::Key_Return:
            if (m_confirmButton && m_confirmButton->isEnabled() && !m_confirmButton->isHidden() && m_confirmButton->hasFocus()) {
                m_confirmButton->clicked();
                m_confirmButton->setFocus();
            } else if (m_cancelButton && m_cancelButton->isEnabled() && !m_cancelButton->isHidden() && m_cancelButton->hasFocus()) {
                m_cancelButton->clicked();
            }
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

SimpleDelegate::SimpleDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SimpleDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    painter->save();

    // 获取数据
    QString name = index.data(Qt::DisplayRole).toString();
    QString subText = index.data(Qt::UserRole + 1).toString();
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();

    // 绘制选中背景
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(255, 255, 255, 80));
    }

    // 绘制图标
    int iconSize = 32;
    int margin = 12;
    QRect iconRect(option.rect.left() + margin,
                  option.rect.top() + (option.rect.height() - iconSize) / 2,
                  iconSize, iconSize);

    if (!icon.isNull()) {
        icon.paint(painter, iconRect);
    }

    // 绘制文本区域
    int textLeft = iconRect.right() + margin;
    QRect textRect(textLeft, option.rect.top(),
                  option.rect.width() - textLeft - margin,
                  option.rect.height());

    // 设置主文本样式
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(12);
    painter->setFont(font);

    if (!subText.isEmpty()) {
        // 绘制主文本（上面）
        font.setPointSizeF(font.pointSizeF() * 0.9);
        QRect nameRect = textRect;
        nameRect.setHeight(textRect.height() / 2);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);

        // 绘制子文本（下面，灰色）
        painter->setPen(QColor(255, 255, 255, 180));
        QFont subFont = font;
        subFont.setPointSizeF(subFont.pointSizeF() * 0.8);
        painter->setFont(subFont);

        QRect subRect = textRect;
        subRect.setTop(nameRect.bottom());
        painter->drawText(subRect, Qt::AlignLeft | Qt::AlignTop, subText);
    } else {
        // 只有主文本，垂直居中
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, name);
    }

    painter->restore();
}
