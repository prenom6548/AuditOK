/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "klunardateedit.h"
#include "kcustomdateedit.h"
#include <QApplication>
#include <QDateTimeEdit>
#include <QGSettings>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSvgRenderer>
#include <QVBoxLayout>
#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "styleName"

namespace kdk
{
class KLunarDateEditPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KLunarDateEdit)
public:
    KLunarDateEditPrivate(KLunarDateEdit *parent);

private:
    QPixmap loadSvg(const QString &path, int size);
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);

private:
    KLunarDateEdit *q_ptr;

    QGSettings *colorGsettings;
    KCustomDateEdit *dateEdit;
    QWidget *m_pLunarWidget;
    QLabel *lunarLabel;
    QPushButton *popButton;
};

KLunarDateEditPrivate::KLunarDateEditPrivate(KLunarDateEdit *parent)
    : q_ptr(parent)
{
    Q_Q(KLunarDateEdit);
    dateEdit = new KCustomDateEdit(q);
    dateEdit->setContextMenuPolicy(Qt::NoContextMenu);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(dateEdit->sizePolicy().hasHeightForWidth());
    dateEdit->setSizePolicy(sizePolicy);

    // lunarDateEdit
    m_pLunarWidget = new QWidget(q);
    m_pLunarWidget->setObjectName(QString::fromUtf8("LunarWidget"));
    QHBoxLayout *horizontalLayout = new QHBoxLayout(m_pLunarWidget);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setMargin(0);

    lunarLabel = new QLabel(m_pLunarWidget);
    sizePolicy.setHeightForWidth(lunarLabel->sizePolicy().hasHeightForWidth());
    lunarLabel->setMargin(10);
    lunarLabel->setSizePolicy(sizePolicy);

    popButton = new QPushButton(m_pLunarWidget);
    sizePolicy.setHeightForWidth(popButton->sizePolicy().hasHeightForWidth());
    popButton->setSizePolicy(sizePolicy);
    popButton->setFixedWidth(38);
    popButton->setStyleSheet(QString::fromUtf8("QPushButton{background-color: rgba(255, 255, 255, 0);border: none;}"
                                               "QPushButton:hover { background-color: rgba(255, 255, 255, 0); }"
                                               "QPushButton:pressed { background-color: rgba(255, 255, 255, 0);}"));
    popButton->setFlat(true);
    connect(popButton, SIGNAL(clicked()), q, SLOT(slotClicked()));
    horizontalLayout->addWidget(lunarLabel);
    horizontalLayout->addWidget(popButton);

    dateEdit->setVisible(true);
    m_pLunarWidget->setVisible(false);

    connect(dateEdit, SIGNAL(dateChanged(QDate)), q, SIGNAL(dateChanged(QDate)));
    connect(dateEdit, SIGNAL(dateChanged(QDate)), q, SLOT(slotDateChanged(QDate)));

    const QByteArray id(THEME_SCHAME);

    if (QGSettings::isSchemaInstalled(id)) {
        colorGsettings = new QGSettings(id, QByteArray(), q);
        connect(colorGsettings, &QGSettings::changed, this, [=](QString key) {
            if ("themeColor" == key || "styleName" == key) {
                q->slotPaletteChanged();
            }
        });
    }
}

QPixmap KLunarDateEditPrivate::loadSvg(const QString &path, int size)
{
    Q_Q(KLunarDateEdit);
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();

    if (2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }

    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap);
}

QPixmap KLunarDateEditPrivate::drawSymbolicColoredPixmap(const QPixmap &source)
{
    Q_Q(KLunarDateEdit);
    QImage img = source.toImage();

    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);

            if (color.alpha() > 0) {
                QColor colorSet = q->palette().color(popButton->palette().ButtonText);
                color.setRed(colorSet.red());
                color.setGreen(colorSet.green());
                color.setBlue(colorSet.blue());
                img.setPixelColor(x, y, color);
            }
        }
    }

    return QPixmap::fromImage(img);
}

KLunarDateEdit::KLunarDateEdit(QWidget *parent)
    : QWidget{parent}
    , d_ptr(new KLunarDateEditPrivate(this))
{
    Q_D(KLunarDateEdit);
    slotPaletteChanged();
}

void KLunarDateEdit::setShowLunar(bool showLunar)
{
    Q_D(KLunarDateEdit);
    slotDateChanged(d->dateEdit->date());
    d->m_pLunarWidget->setVisible(showLunar);
    d->dateEdit->setShowLunar(showLunar);
}

void KLunarDateEdit::slotPaletteChanged()
{
    Q_D(KLunarDateEdit);

    if (d->popButton == nullptr || d->lunarLabel == nullptr || d->m_pLunarWidget == nullptr)
        return;

    QPalette pal = qApp->palette();
    this->setPalette(pal);
    d->dateEdit->setPalette(pal);
    d->m_pLunarWidget->setPalette(pal);
    d->lunarLabel->setPalette(pal);
    d->popButton->setPalette(pal);

    QGSettings *styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);

    if (QGSettings::isSchemaInstalled(style_id)) {
        styleGsettings = new QGSettings(style_id);
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();

        if (currentTheme == "ukui-default" || currentTheme == "ukui-light") {
            d->dateEdit->updateStyle(false);
            d->m_pLunarWidget->setStyleSheet(QString("QWidget#LunarWidget{background-color: rgb(233,233,233); border:none;border-radius:5px;}"));
            d->lunarLabel->setStyleSheet(QString("color:rgb(0,0,0);"));
        } else if (currentTheme == "ukui-dark") {
            d->dateEdit->updateStyle(true);
            d->m_pLunarWidget->setStyleSheet(QString("QWidget#LunarWidget{background-color: rgb(55,55,59); border:none;border-radius:5px;}"));
            d->lunarLabel->setStyleSheet(QString("color:rgb(233,233,233);"));
        }
    }

    QPixmap pixmap = d->loadSvg("/usr/share/kylin-user-guide/data/ukui-down-symbolic.svg", 64);
    pixmap = pixmap.scaled(QSize(15, 15), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QIcon icon(pixmap);
    d->popButton->setIcon(icon);
    d->popButton->setIconSize(QSize(15, 15));

    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}

QDateTimeEdit *KLunarDateEdit::getDateEdit()
{
    Q_D(KLunarDateEdit);
    return static_cast<QDateTimeEdit *>(d->dateEdit);
}

void KLunarDateEdit::resizeEvent(QResizeEvent *event)
{
    Q_D(KLunarDateEdit);
    d->dateEdit->setFixedSize(event->size());
    d->m_pLunarWidget->setFixedSize(event->size());
}

void KLunarDateEdit::slotDateChanged(QDate date)
{
    Q_D(KLunarDateEdit);
    QString strHoliday;
    QString strSolarTerms;
    QString strLunarFestival;
    QString strLunarYear;
    QString strLunarMonth;
    QString strLunarDay;

    KLunarCalendarInfo::Instance()->getLunarCalendarInfo(date.year(),
                                                         date.month(),
                                                         date.day(),
                                                         strHoliday,
                                                         strSolarTerms,
                                                         strLunarFestival,
                                                         strLunarYear,
                                                         strLunarMonth,
                                                         strLunarDay);

    QString labBottomarg = strLunarMonth + strLunarDay;
    d->lunarLabel->setText(labBottomarg);
}

void KLunarDateEdit::slotClicked()
{
    Q_D(KLunarDateEdit);
    d->dateEdit->showCalendar();
}
}

#include "klunardateedit.moc"
#include "moc_klunardateedit.cpp"
