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

#include "quick-look-info-panel.h"

#include <file-info.h>
#include <file-info-job.h>
#include <file-count-operation.h>
#include <file-utils.h>
#include <thumbnail-manager.h>

#include <glib.h>

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QTextLayout>
#include <QtMath>
#include <QThreadPool>
#include <QUrl>
#include <QVBoxLayout>

using namespace Peony;

static const int kIconSize = 96;
static const int kPanelMargin = 48;
static const int kRowSpacing = 32;
static const int kTextRightPadding = 8;

class WrapAnywhereLabel : public QLabel
{
public:
    explicit WrapAnywhereLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setMinimumWidth(0);
        setTextFormat(Qt::PlainText);
        auto policy = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        setSizePolicy(policy);
    }

    void setText(const QString &text)
    {
        QLabel::setText(text);
        syncMinimumWidth();
        updateGeometry();
    }

    int idealTextWidth() const
    {
        return naturalTextWidth();
    }

    bool hasHeightForWidth() const override
    {
        return needsWrapping(widthCap());
    }

    int heightForWidth(int width) const override
    {
        const QMargins margins = contentsMargins();
        const int constraint = constraintWidth(qMax(1, width - margins.left() - margins.right()));
        const int textWidth = lineWidthForConstraint(constraint);
        return layoutHeight(textWidth) + margins.top() + margins.bottom();
    }

    QSize minimumSizeHint() const override
    {
        const int w = qMax(1, hintWidth());
        return QSize(w, heightForWidth(w));
    }

    QSize sizeHint() const override
    {
        const int w = qMax(1, hintWidth());
        return QSize(w, heightForWidth(w));
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setFont(font());
        const QColor textColor = isEnabled()
                ? palette().color(QPalette::WindowText)
                : palette().color(QPalette::Disabled, QPalette::Text);
        painter.setPen(textColor);

        const QRect rect = contentsRect();
        const int constraint = constraintWidth(qMax(1, rect.width()));
        const int lineWidth = lineWidthForConstraint(constraint);
        const qreal height = layoutHeight(lineWidth);

        QTextLayout layout(text(), font());
        layout.setTextOption(textOptionForLineWidth(lineWidth));

        layout.beginLayout();
        qreal y = 0;
        while (true) {
            QTextLine line = layout.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(lineWidth);
            line.setPosition(QPointF(0, y));
            y += line.height();
        }
        layout.endLayout();

        qreal drawY = rect.top();
        if (alignment() & Qt::AlignVCenter)
            drawY += qMax<qreal>(0, (rect.height() - height) / 2);
        else if (alignment() & Qt::AlignBottom)
            drawY += qMax<qreal>(0, rect.height() - height);

        layout.draw(&painter, QPointF(rect.left(), drawY));
    }

private:
    static const int kMeasureSlack = 4;

    int widthCap() const
    {
        if (maximumWidth() > 0 && maximumWidth() < QWIDGETSIZE_MAX)
            return maximumWidth();
        return QWIDGETSIZE_MAX;
    }

    int constraintWidth(int contentWidth) const
    {
        int w = qMax(1, contentWidth);
        if (maximumWidth() > 0 && maximumWidth() < QWIDGETSIZE_MAX)
            w = qMin(w, maximumWidth());
        return w;
    }

    int naturalTextWidth() const
    {
        if (text().isEmpty())
            return 0;

        QTextLayout layout(text(), font());
        QTextOption option;
        option.setWrapMode(QTextOption::NoWrap);
        layout.setTextOption(option);

        layout.beginLayout();
        QTextLine line = layout.createLine();
        if (!line.isValid()) {
            layout.endLayout();
            return 0;
        }
        line.setLineWidth(1 << 20);
        const int w = qCeil(line.naturalTextWidth());
        layout.endLayout();

        return w + kMeasureSlack;
    }

    bool needsWrapping(int constraintWidth) const
    {
        return naturalTextWidth() > constraintWidth;
    }

    int lineWidthForConstraint(int constraintWidth) const
    {
        const int natural = naturalTextWidth();
        if (natural <= constraintWidth)
            return natural;
        return qMax(1, constraintWidth);
    }

    QTextOption textOptionForLineWidth(int lineWidth) const
    {
        QTextOption option;
        if (lineWidth >= naturalTextWidth()) {
            option.setWrapMode(QTextOption::NoWrap);
        } else {
            // 长文件名无空格/混排中英文时，按字符填满行宽，避免词边界导致提前换行
            option.setWrapMode(QTextOption::WrapAnywhere);
        }
        return option;
    }

    int hintWidth() const
    {
        return qMin(naturalTextWidth(), widthCap());
    }

    void syncMinimumWidth()
    {
        const int cap = widthCap();
        if (needsWrapping(cap))
            setMinimumWidth(cap);
        else
            setMinimumWidth(hintWidth());
    }

    int layoutHeight(int lineWidth) const
    {
        if (text().isEmpty())
            return QFontMetrics(font()).height();

        QTextLayout layout(text(), font());
        layout.setTextOption(textOptionForLineWidth(lineWidth));

        layout.beginLayout();
        qreal height = 0;
        while (true) {
            QTextLine line = layout.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(lineWidth);
            line.setPosition(QPointF(0, height));
            height += line.height();
        }
        layout.endLayout();

        return qCeil(height);
    }
};

static int labelIdealTextWidth(const QLabel *label)
{
    return static_cast<const WrapAnywhereLabel *>(label)->idealTextWidth();
}

static void setLabelIcon(QLabel *label, const QIcon &icon)
{
    if (!label)
        return;
    if (icon.isNull()) {
        label->clear();
        return;
    }
    label->setPixmap(icon.pixmap(QSize(kIconSize, kIconSize)));
}

QuickLookInfoPanel::QuickLookInfoPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    outer->addStretch(1);

    auto *centerRow = new QHBoxLayout;
    centerRow->addStretch(1);

    m_contentWidget = new QWidget(this);
    m_contentWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    auto *row = new QHBoxLayout(m_contentWidget);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(kRowSpacing);
    row->setAlignment(Qt::AlignCenter);

    m_icon = new QLabel(m_contentWidget);
    m_icon->setFixedSize(kIconSize, kIconSize);
    m_icon->setAlignment(Qt::AlignCenter);
    row->addWidget(m_icon, 0, Qt::AlignVCenter);

    auto *infoColumn = new QVBoxLayout;
    infoColumn->setContentsMargins(0, 0, kTextRightPadding, 0);
    infoColumn->setSpacing(8);
    infoColumn->setAlignment(Qt::AlignVCenter);

    m_nameLabel = new WrapAnywhereLabel(m_contentWidget);
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() + 4);
    m_nameLabel->setFont(nameFont);
    m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    infoColumn->addWidget(m_nameLabel);

    m_metaLabel = new WrapAnywhereLabel(m_contentWidget);
    m_metaLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QPalette pal = m_metaLabel->palette();
    pal.setColor(QPalette::WindowText, pal.color(QPalette::Disabled, QPalette::Text));
    m_metaLabel->setPalette(pal);
    infoColumn->addWidget(m_metaLabel);

    row->addLayout(infoColumn);
    centerRow->addWidget(m_contentWidget, 0, Qt::AlignCenter);
    centerRow->addStretch(1);

    outer->addLayout(centerRow);
    outer->addStretch(1);
}

void QuickLookInfoPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateTextWidths();
}

void QuickLookInfoPanel::updateTextWidths()
{
    if (!m_nameLabel || !m_contentWidget)
        return;

    const int available = qMax(0, width() - kPanelMargin * 2);
    const int maxTextWidth = qMax(80, available - kIconSize - kRowSpacing - kTextRightPadding);

    m_contentWidget->setMaximumWidth(available);
    m_nameLabel->setMaximumWidth(maxTextWidth);
    m_metaLabel->setMaximumWidth(maxTextWidth);

    const int columnWidth = qMax(labelIdealTextWidth(m_nameLabel), labelIdealTextWidth(m_metaLabel));
    const int contentWidth = qMin(available, kIconSize + kRowSpacing + qMin(columnWidth, maxTextWidth));
    m_contentWidget->setMinimumWidth(contentWidth);

    const int nameIdeal = labelIdealTextWidth(m_nameLabel);
    const int metaIdeal = labelIdealTextWidth(m_metaLabel);
    m_nameLabel->setMinimumWidth(nameIdeal > maxTextWidth ? maxTextWidth : qMin(nameIdeal, maxTextWidth));
    m_metaLabel->setMinimumWidth(metaIdeal > maxTextWidth ? maxTextWidth : qMin(metaIdeal, maxTextWidth));

    m_nameLabel->updateGeometry();
    m_metaLabel->updateGeometry();
    m_contentWidget->updateGeometry();
}

void QuickLookInfoPanel::clearPanel()
{
    m_uri.clear();
    m_icon->clear();
    m_nameLabel->clear();
    m_metaLabel->clear();
}

void QuickLookInfoPanel::updateIconAndName(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    if (info->displayName().isEmpty() || info->fileType().isEmpty()) {
        FileInfoJob job(uri);
        job.querySync();
    }

    auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(uri);
    if (!thumbnail.isNull()) {
        QUrl url(uri);
        thumbnail.addFile(url.path());
    }
    QIcon icon = QIcon::fromTheme(info->iconName(), QIcon::fromTheme(QStringLiteral("unknown")));
    setLabelIcon(m_icon, thumbnail.isNull() ? icon : thumbnail);

    QString displayName = info->displayName();
    if (displayName.isEmpty())
        displayName = FileUtils::getUriBaseName(uri);
    m_nameLabel->setText(displayName);
    m_nameLabel->setToolTip(displayName);
    updateTextWidths();
}

void QuickLookInfoPanel::setFileInfo(const QString &uri)
{
    m_uri = uri;
    updateIconAndName(uri);

    auto info = FileInfo::fromUri(uri);
    m_metaLabel->setText(info->fileSize());
    updateTextWidths();
}

void QuickLookInfoPanel::setFolderInfo(const QString &uri)
{
    m_uri = uri;
    updateIconAndName(uri);
    m_metaLabel->setText(tr("Calculating..."));
    startFolderCount(uri);
}

void QuickLookInfoPanel::startFolderCount(const QString &uri)
{
    auto *countOp = new FileCountOperation(QStringList() << uri, false);
    countOp->setAutoDelete(true);
    connect(countOp, &FileCountOperation::countDone, this, [this, uri](quint64 fileCount, quint64 hiddenCount, quint64 totalSize) {
        Q_UNUSED(hiddenCount)
        if (m_uri != uri)
            return;

        auto format = g_format_size(totalSize);
        const QString sizeText = QString::fromUtf8(format);
        g_free(format);
        m_metaLabel->setText(tr("Total %1 items, %2").arg(fileCount).arg(sizeText));
        updateTextWidths();
    });
    QThreadPool::globalInstance()->start(countOp);
}
