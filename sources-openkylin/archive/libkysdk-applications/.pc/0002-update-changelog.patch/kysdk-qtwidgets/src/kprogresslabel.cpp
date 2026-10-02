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

#include "kprogresslabel.h"
#include "themeController.h"
#include <QDebug>
#include <QGSettings/QGSettings>
#include <QIcon>
#include <QVariant>

namespace kdk
{

class Q_DECL_HIDDEN LineLabel : public QLabel
{
    Q_OBJECT
public:
    LineLabel(QWidget *parent = nullptr);
    virtual ~LineLabel();

    void setBackgroundHighlight(bool flag);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QColor m_backgroundColor;
    bool colorFlag;
};

class Q_DECL_HIDDEN CircleLable : public QLabel
{
    Q_OBJECT
public:
    CircleLable(const QString &text, QWidget *parent = nullptr);
    virtual ~CircleLable();
    void setText(const QString &text);
    void setBackgroundHighlight(bool flag);
    void setSucceed(bool flag);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QString m_text;
    QColor m_textColor;
    QColor m_backgroundColor;
    bool colorFlag;
    bool isSucceed;
    int m_radius;
};

class Q_DECL_HIDDEN TextLabel : public QLabel, public ThemeController
{
    Q_OBJECT
public:
    TextLabel(QString str, QWidget *parent = nullptr);
    virtual ~TextLabel();

    void setTextHighlight(bool flag);
    void followPalette(bool flag);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    bool isPalette;
};

class Q_DECL_HIDDEN KProgressLabelPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KProgressLabel)
public:
    KProgressLabelPrivate(KProgressLabel *parent);

protected:
    void changeTheme();

private:
    KProgressLabel *const q_ptr;

    QVBoxLayout *mainLayout;
    QHBoxLayout *circleLayout;
    QHBoxLayout *textLayout;

    int m_count;
    QSize m_circleSize;
    QList<CircleLable *> circleList;
    QList<LineLabel *> lineList;
    QList<TextLabel *> textList;
};

KProgressLabel::KProgressLabel(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KProgressLabelPrivate(this))
{
    Q_D(KProgressLabel);
    d->mainLayout = new QVBoxLayout(this);
    QWidget *wid1 = new QWidget();
    QWidget *wid2 = new QWidget();
    d->circleLayout = new QHBoxLayout(wid1);
    d->circleLayout->setContentsMargins(0, 0, 0, 0);
    d->textLayout = new QHBoxLayout(wid2);
    d->textLayout->setContentsMargins(8, 0, 8, 0);
    d->mainLayout->setSpacing(0);
    d->mainLayout->addWidget(wid1);
    d->mainLayout->addSpacing(8);
    d->mainLayout->addWidget(wid2);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

KProgressLabel::~KProgressLabel()
{
}

void KProgressLabel::setStringList(QStringList list1, QStringList list2)
{
    Q_D(KProgressLabel);
    // 若布局不为空，则清空布局
    if (!d->circleLayout->isEmpty() || !d->textLayout->isEmpty()) {
        // 先清除list
        d->circleList.clear();
        d->lineList.clear();
        d->textList.clear();

        QLayoutItem *child;
        while ((child = d->circleLayout->takeAt(0)) != 0) {
            // 删除Stretch(弹簧)等布局
            if (child->spacerItem()) {
                d->circleLayout->removeItem(child);
                continue;
            }
            // 删除布局
            d->circleLayout->removeWidget(child->widget());
            child->widget()->setParent(nullptr);
            delete child;
            child = nullptr;
        }
        while ((child = d->textLayout->takeAt(0)) != 0) {
            if (child->spacerItem()) {
                d->textLayout->removeItem(child);
                continue;
            }
            d->textLayout->removeWidget(child->widget());
            child->widget()->setParent(nullptr);
            delete child;
            child = nullptr;
        }
        d->circleLayout->addSpacing(80);
        d->textLayout->addSpacing(11);
    }
    for (int i = 0; i < list1.count(); i++) {
        // 第一行圆形label
        QString str;
        if (i < list1.count())
            str = list1.at(i);
        CircleLable *circlelabel = new CircleLable(str, this);
        circlelabel->setFixedSize(d->m_circleSize);
        d->circleList.append(circlelabel);
        d->circleLayout->addWidget(circlelabel);

        if (i < list1.count() - 1) {
            // 第一行线条label
            LineLabel *lineLabel;
            lineLabel = new LineLabel(this);
            d->lineList.append(lineLabel);
            d->circleLayout->addWidget(lineLabel);
        }
    }
    // 第二行文本label
    for (int i = 0; i < list2.count(); i++) {
        TextLabel *textLabel = new TextLabel(list2.at(i), this);
        textLabel->setWordWrap(true);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setScaledContents(true);
        d->textList.append(textLabel);
        d->textLayout->addWidget(textLabel);
        if (i < list2.count() - 1)
            d->textLayout->addStretch();
    }
}

void KProgressLabel::setStringList(QStringList list1)
{
    Q_D(KProgressLabel);
    if (!d->circleLayout->isEmpty() || !d->textLayout->isEmpty()) {
        // 先清除list
        d->circleList.clear();
        d->lineList.clear();
        d->textList.clear();

        QLayoutItem *child;
        while ((child = d->circleLayout->takeAt(0)) != 0) {
            // 删除Stretch(弹簧)等布局
            if (child->spacerItem()) {
                d->circleLayout->removeItem(child);
                continue;
            }
            // 删除布局
            d->circleLayout->removeWidget(child->widget());
            child->widget()->setParent(nullptr);
            delete child;
            child = nullptr;
        }
        while ((child = d->textLayout->takeAt(0)) != 0) {
            if (child->spacerItem()) {
                d->textLayout->removeItem(child);
                continue;
            }
            d->textLayout->removeWidget(child->widget());
            child->widget()->setParent(nullptr);
            delete child;
            child = nullptr;
        }
        d->circleLayout->addSpacing(80);
        //        d->textLayout->addSpacing(11);
    }
    for (int i = 0; i < list1.count(); i++) {
        // 第一行圆形label
        QString str;
        CircleLable *circlelabel = new CircleLable(str, this);
        circlelabel->setFixedSize(d->m_circleSize);
        d->circleList.append(circlelabel);
        d->circleLayout->addWidget(circlelabel);

        if (i < list1.count() - 1) {
            // 第一行线条label
            LineLabel *lineLabel;
            lineLabel = new LineLabel(this);
            d->lineList.append(lineLabel);
            d->circleLayout->addWidget(lineLabel);
        }
    }
    // 第二行文本label
    for (int i = 0; i < list1.count(); i++) {
        TextLabel *textLabel = new TextLabel(list1.at(i), this);
        textLabel->setWordWrap(true);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setScaledContents(true);
        d->textList.append(textLabel);
        d->textLayout->addWidget(textLabel);
        if (i < list1.count() - 1)
            d->textLayout->addStretch();
    }
}

void KProgressLabel::setCurrentIndex(int index)
{
    Q_D(KProgressLabel);
    if (index < -1)
        return;
    if (index > d->circleList.count() - 1)
        return;
    for (int i = 0; i <= index; i++) {
        if (i < index)
            d->circleList.at(i)->setBackgroundHighlight(true);
        if (i < d->lineList.count())
            d->lineList.at(i)->setBackgroundHighlight(true);
        if (i < index)
            d->textList.at(i)->setTextHighlight(false);
        if (i == index) {
            d->circleList.at(i)->setBackgroundHighlight(false);
            d->circleList.at(i)->setSucceed(true);
            d->textList.at(i)->setTextHighlight(true);
        }
    }
    if (d->m_count > index) {
        for (int j = index + 1; j < d->circleList.count(); j++) {
            if (j < d->circleList.count()) {
                d->circleList.at(j)->setSucceed(false);
                d->circleList.at(j)->setBackgroundHighlight(false);
            }
            if (j < d->lineList.count())
                d->lineList.at(j)->setBackgroundHighlight(false);
            if (j < d->textList.count())
                d->textList.at(j)->followPalette(false);
        }
    }
    d->m_count = index;
    repaint();
}

int KProgressLabel::currentIndex()
{
    Q_D(KProgressLabel);
    return d->m_count;
}

void KProgressLabel::setCircleSize(QSize size)
{
    Q_D(KProgressLabel);
    for (int count = 0; count < d->circleList.count(); count++) {
        d->m_circleSize = size;
        d->circleList.at(count)->setFixedSize(size);
        d->textLayout->setContentsMargins(size.width() / 2 - 6, 0, size.width() / 2 - 6, 0);
    }
}

LineLabel::LineLabel(QWidget *parent)
    : QLabel(parent)
    , colorFlag(false)
{
    setMaximumHeight(30);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

LineLabel::~LineLabel()
{
}

void LineLabel::setBackgroundHighlight(bool flag)
{
    colorFlag = flag;
}

void LineLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    if (!colorFlag) {
        m_backgroundColor = ThemeController::getPaletteColorFromDT("midlight");
    } else {
        m_backgroundColor = ThemeController::getPaletteColorFromDT("highlight");
    }
    QPen pen = painter.pen();
    pen.setColor(m_backgroundColor);
    pen.setWidth(2);
    painter.setPen(pen);
    QRect rect = this->rect();
    painter.drawLine(0, rect.height() / 2, rect.width(), rect.height() / 2);

    painter.restore();
    QLabel::paintEvent(event);
}

CircleLable::CircleLable(const QString &text, QWidget *parent)
    : QLabel(parent)
    , m_text(text)
    , m_radius(24)
    , colorFlag(false)
    , isSucceed(false)
    , m_textColor(Qt::white)
{
    setFixedSize(QSize(m_radius, m_radius));
}

CircleLable::~CircleLable()
{
}

void CircleLable::setText(const QString &text)
{
    m_text = text;
    repaint();
}

void CircleLable::setBackgroundHighlight(bool flag)
{
    colorFlag = flag;
}

void CircleLable::setSucceed(bool flag)
{
    isSucceed = flag;
}

void CircleLable::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    // 深浅主题切换
    if (!colorFlag && !isSucceed) {
        m_backgroundColor = ThemeController::getPaletteColorFromDT("midlight");
        m_textColor = QColor(255, 255, 255);
        painter.setBrush(QBrush(m_backgroundColor));
    } else if (isSucceed || colorFlag) {
        painter.setBrush(ThemeController::getPaletteColorFromDT("highlight"));
    }

    painter.setPen(Qt::NoPen);
    QRect rect = this->rect();
    painter.drawRoundedRect(rect, rect.width() / 2, rect.width() / 2);

    painter.setPen(m_textColor);
    if (colorFlag) {
        QPixmap pixmap = QIcon::fromTheme("dialog-apply").pixmap(16, 16);
        pixmap = ThemeController::drawSymbolicColoredPixmap(pixmap);
        QRect pixRect((rect.width() - 16) / 2, (rect.height() - 16) / 2, 16, 16);
        painter.drawPixmap(pixRect, pixmap);
    } else
        painter.drawText(rect, Qt::AlignCenter, m_text);

    QLabel::paintEvent(event);
}

TextLabel::TextLabel(QString str, QWidget *parent)
    : QLabel(parent)
{
    setText(str);
    isPalette = false;
}

TextLabel::~TextLabel()
{
}

void TextLabel::setTextHighlight(bool flag)
{
    isPalette = flag;
    if (flag) {
        QPalette palette = this->palette();
        palette.setColor(QPalette::WindowText, ThemeController::getPaletteColorFromDT("highlight"));
        this->setPalette(palette);
    } else {
        QPalette palette;
        this->setPalette(palette);
    }
}

void TextLabel::followPalette(bool flag)
{
    isPalette = flag;
}

void TextLabel::paintEvent(QPaintEvent *e)
{
    if (!isPalette) {
        QPalette palette = this->palette();
        QColor textColor = ThemeController::getPaletteColorFromDT("brighttext");
        textColor.setAlphaF(0.35);
        palette.setColor(QPalette::WindowText, textColor);
        this->setPalette(palette);
    } else {
        QPalette palette = this->palette();
        palette.setColor(QPalette::WindowText, ThemeController::getPaletteColorFromDT("highlight"));
        this->setPalette(palette);
    }
    QLabel::paintEvent(e);
}

KProgressLabelPrivate::KProgressLabelPrivate(KProgressLabel *parent)
    : q_ptr(parent)
    , m_count(-1)
    , m_circleSize(24, 24)
{
    Q_Q(KProgressLabel);
    connect(m_gsetting, &QGSettings::changed, this, [=] {
        changeTheme();
    });
}

void KProgressLabelPrivate::changeTheme()
{
    Q_Q(KProgressLabel);
    initThemeStyle();
}

}

#include "kprogresslabel.moc"
#include "moc_kprogresslabel.cpp"
