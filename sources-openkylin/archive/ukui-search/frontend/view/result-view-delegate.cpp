/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */
#include "result-view-delegate.h"
#include <QPainterPath>
#include <QApplication>
#include <QPen>
#include <QStyleFactory>
#include <QTreeView>
#include "global-settings.h"
#include "search-result-model.h"
using namespace UkuiSearch;
static ResultItemStyle *global_instance_of_item_style = nullptr;

ResultViewDelegate::ResultViewDelegate(QObject *parent) : QStyledItemDelegate(parent),
    m_textDoc(new QTextDocument(this)),
    m_hightLightEffectHelper(new HightLightEffectHelper(this))
{
    m_textDoc->setDefaultFont(QApplication::font());
    connect(qApp, &QApplication::fontChanged, m_textDoc, &QTextDocument::setDefaultFont);
}

void ResultViewDelegate::setSearchKeyword(const QString &regFindKeyWords)
{
    m_hightLightEffectHelper->setExpression(regFindKeyWords);
}

void ResultViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.displayAlignment = QApplication::isLeftToRight() ?
            Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter) : Qt::Alignment(Qt::AlignRight|Qt::AlignVCenter);

    QString originalText = opt.text;
    opt.text = QString();

    if (opt.icon.isNull()) {
        opt.features |= QStyleOptionViewItem::HasDecoration;
        QString prefix = GlobalSettings::getInstance().getValue(STYLE_NAME_KEY).toString() == "ukui-dark" ? ":res/icons/dark/" : ":res/icons/default/";
        opt.icon = QIcon::fromTheme(index.data(IconName).toString(), QIcon(prefix + index.data(IconName).toString()));
    }

    QStyle *style = opt.widget->style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget); //绘制非文本区域内容

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
    QFontMetrics fontMetrics(opt.font);
    QString text = fontMetrics.elidedText(originalText, Qt::ElideRight, textRect.width() - m_textDoc->documentMargin() - 1); //富余5px的宽度

    //无需显示tooltip时，将状态位写回model
    auto view = qobject_cast<QTreeView *>(this->parent());
    if(text == originalText && originalText == index.data(Qt::ToolTipRole).toString()) {
        if(view) {
            view->model()->setData(index, false, AdditionalRoles::ShowToolTip);
        }
    } else {
        if(view) {
            view->model()->setData(index, true, AdditionalRoles::ShowToolTip);
        }
    }
    opt.text = text;


    painter->save();

    bool displayNameAsUrl = index.data(AdditionalRoles::DisplayNameAsUrl).toBool();
    if (opt.state & QStyle::State_Selected) {
        QVariant selectPen = opt.widget->property("textSelectPen");
        if(selectPen.isValid() && selectPen.canConvert<QPen>()) {
            m_hightLightEffectHelper->setTextColor(selectPen.value<QPen>().brush());
        } else {
            m_hightLightEffectHelper->setTextColor(QBrush(opt.palette.highlightedText().color()));
        }
    } else {
        if (displayNameAsUrl) {
            m_hightLightEffectHelper->setTextColor(QBrush(opt.palette.link().color()));
        } else {
            m_hightLightEffectHelper->setTextColor(QBrush(opt.palette.text().color()));
        }
    }
    m_hightLightEffectHelper->setBold(!displayNameAsUrl);

    m_textDoc->setPlainText(text);
    m_hightLightEffectHelper->setDocument(m_textDoc);
    m_hightLightEffectHelper->rehighlight();

    int textRectX = QApplication::isLeftToRight() ? textRect.x() : textRect.width() - fontMetrics.horizontalAdvance(text) - m_textDoc->documentMargin();
    painter->translate(textRectX, textRect.y() + (textRect.height() - fontMetrics.height()) / 2 - m_textDoc->documentMargin());
    m_textDoc->drawContents(painter);
    painter->restore();
}

ResultItemStyle *ResultItemStyle::getStyle()
{
    if (!global_instance_of_item_style) {
        global_instance_of_item_style = new ResultItemStyle;
    }
    return global_instance_of_item_style;
}

ResultItemStyle::ResultItemStyle()
{
    connect(qApp, &QApplication::paletteChanged, this, [&](){
        auto style = QStyleFactory::create("ukui");
        if(style) {
            setBaseStyle(style);
            Q_EMIT baseStyleChanged();
        }
    });
}

int ResultItemStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_ItemView_ActivateItemOnSingleClick:
        return false;
    default:
        return qApp->style()->styleHint(hint, option, widget, returnData);
    }
}

HightLightEffectHelper::HightLightEffectHelper(QObject *parent) : QSyntaxHighlighter(parent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_expression.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
#else
    m_expression.setCaseSensitivity(Qt::CaseInsensitive);
    m_expression.setPatternSyntax(QRegExp::FixedString);
#endif
}

void HightLightEffectHelper::setExpression(const QString &text)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_expression.setPattern(QRegularExpression::escape(text)); // 转义字面字符串
#else
    m_expression.setPattern(text);
#endif
}

void HightLightEffectHelper::setTextColor(const QBrush &brush)
{
    m_textCharFormat.setForeground(brush);
}

void HightLightEffectHelper::highlightBlock(const QString &text)
{
    setFormat(0, text.length(), m_textCharFormat);
    if (m_need2BBolded) {
        m_textCharFormat.setFontWeight(QFont::Bold);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QRegularExpressionMatchIterator it = m_expression.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            setFormat(match.capturedStart(),
                     match.capturedLength(),
                     m_textCharFormat);
        }
#else
        int index = text.indexOf(m_expression);
        while(index >= 0){
            int length = m_expression.matchedLength();
            setFormat(index, length, m_textCharFormat);
            index = text.indexOf(m_expression, index+length);
        }
#endif
    }
    m_textCharFormat.setFontWeight(QFont::Normal);
}

void HightLightEffectHelper::setBold(bool need2BBolded)
{
    m_need2BBolded = need2BBolded;
}
