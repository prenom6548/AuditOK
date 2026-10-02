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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kaddfilebutton.h"
#include "themeController.h"
#include <QDebug>
#include <QEvent>
#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QStyleOptionButton>
#include "accessinfohelper.h"

namespace kdk
{

#define BLANK_SPACE 20

class AddFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    AddFileDialog(QWidget *parent = nullptr);
    virtual ~AddFileDialog();

public slots:
    void goAccept();

protected:
    void showEvent(QShowEvent *event) override;
};

class KAddFileButtonPrivate : public QObject, public ThemeController
{
    Q_DECLARE_PUBLIC(KAddFileButton)
    Q_OBJECT
public:
    KAddFileButtonPrivate(KAddFileButton *parent);

    void updateColor(QStyleOptionButton option, QPalette palette);
    void openFile();

protected:
    void changeTheme();

private:
    KAddFileButton *q_ptr;
    QColor m_bkgColor;
    QString m_text;
    QRect m_rect;
    QRect m_textRect;
    AddFileDialog *m_FileDialog;
};

KAddFileButton::KAddFileButton(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new KAddFileButtonPrivate(this))
{
    Q_D(KAddFileButton);
    setFixedSize(104, 110);
    connect(this, &KAddFileButton::clicked, this, [=] {
        d->openFile();
    });
}

void KAddFileButton::paintEvent(QPaintEvent *event)
{
    Q_D(KAddFileButton);
    QStyleOptionButton option;
    initStyleOption(&option);
    d->updateColor(option, palette());

    d->m_rect = QRect(QPoint(20, 8), QSize(64, 64));

    QPainter painter(this);
    painter.save();
    painter.setBrush(ThemeController::getPaletteColorFromDT("base-active"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 0, 0);
    painter.restore();
    QPen pen = painter.pen();
    pen.setStyle(Qt::DashLine);
    pen.setColor(ThemeController::getPaletteColorFromDT("highlight-active"));
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.setBrush(d->m_bkgColor);
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;
    painter.drawRoundedRect(d->m_rect, radius, radius);
    painter.setPen(ThemeController::getPaletteColorFromDT("highlight-active"));
    painter.drawLine(QPoint(d->m_rect.center().x() - 10, d->m_rect.center().y() + 1), QPoint(d->m_rect.center().x() + 10, d->m_rect.center().y() + 1));
    painter.drawLine(QPoint(d->m_rect.center().x() + 1, d->m_rect.center().y() - 10), QPoint(d->m_rect.center().x() + 1, d->m_rect.center().y() + 10));
    painter.restore();

    painter.save();
    QFontMetrics fm = painter.fontMetrics();
    int width = fm.width(d->m_text);
    painter.setPen(ThemeController::getPaletteColorFromDT("highlight-active"));
    d->m_textRect = QRect(QPoint(rect().center().x() - width / 2, d->m_rect.bottom() + 8), QSize(width, fm.height()));
    painter.drawText(d->m_textRect, d->m_text);
    painter.restore();
}

bool KAddFileButton::eventFilter(QObject *watched, QEvent *event)
{
    return QPushButton::eventFilter(watched, event);
}

void KAddFileButton::mouseMoveEvent(QMouseEvent *event)
{
    return QPushButton::mouseMoveEvent(event);
}

void KAddFileButton::mousePressEvent(QMouseEvent *event)
{
    return QPushButton::mousePressEvent(event);
}

void KAddFileButton::mouseReleaseEvent(QMouseEvent *event)
{
    return QPushButton::mouseReleaseEvent(event);
}

void KAddFileButton::setText(QString str)
{
    Q_D(KAddFileButton);
    d->m_text = str;
}

QFileDialog *KAddFileButton::fileDialog()
{
    Q_D(KAddFileButton);
    return dynamic_cast<QFileDialog *>(d->m_FileDialog);
}

void KAddFileButton::setNameFilter(const QString &filter)
{
    Q_D(KAddFileButton);
    QString f(filter);
    if (f.isEmpty())
        return;
    QString sep(QLatin1String(";;"));
    int i = f.indexOf(sep, 0);
    if (i == -1) {
        if (f.indexOf(QLatin1Char('\n'), 0) != -1) {
            sep = QLatin1Char('\n');
            i = f.indexOf(sep, 0);
        }
    }
    setNameFilters(f.split(sep));
}

void KAddFileButton::setNameFilters(const QStringList &filters)
{
    Q_D(KAddFileButton);
    d->m_FileDialog->setNameFilters(filters);
}

KAddFileButtonPrivate::KAddFileButtonPrivate(KAddFileButton *parent)
    : q_ptr(parent)
    , m_text("添加文件")
{
    Q_Q(KAddFileButton);
    setParent(parent);
    m_FileDialog = new AddFileDialog(q);
    m_FileDialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    m_FileDialog->setFileMode(QFileDialog::ExistingFiles);

    KDK_ALL_INFO_FORMAT(m_FileDialog,"");
}

void KAddFileButtonPrivate::updateColor(QStyleOptionButton option, QPalette palette)
{
    Q_Q(KAddFileButton);
    m_bkgColor = ThemeController::getPaletteColorFromDT("base-active");
    QColor hightColor = ThemeController::getPaletteColorFromDT("highlight-active");
    if (option.state.testFlag(QStyle::State_MouseOver)) {
        m_bkgColor = hightColor;
        if (option.state.testFlag(QStyle::State_Sunken))
            m_bkgColor.setAlphaF(0.4);
        else
            m_bkgColor.setAlphaF(0.3);
    }
}

void KAddFileButtonPrivate::openFile()
{
    Q_Q(KAddFileButton);

    m_FileDialog->setWindowTitle(tr("Please select file"));

    m_FileDialog->exec();
}

void KAddFileButtonPrivate::changeTheme()
{
    initThemeStyle();
}

AddFileDialog::AddFileDialog(QWidget *parent)
{
    this->setFileMode(QFileDialog::ExistingFiles);
}

AddFileDialog::~AddFileDialog()
{
}

void AddFileDialog::goAccept()
{
    accept();
}

void AddFileDialog::showEvent(QShowEvent *event)
{
    if (this->isVisible()) {
        for (QWidget *widget : this->findChildren<QWidget *>()) {
            if (widget->objectName() == "acceptButton") {
                QPushButton *btn = qobject_cast<QPushButton *>(widget);
                disconnect(btn, &QPushButton::clicked, 0, 0);
                connect(btn, &QPushButton::clicked, [=](bool checked) {
                    Q_UNUSED(checked)
                    this->goAccept();
                });
            }
        }
    }
}

}

#include "kaddfilebutton.moc"
#include "moc_kaddfilebutton.cpp"
