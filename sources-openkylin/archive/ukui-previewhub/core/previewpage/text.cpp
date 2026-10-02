#include "previewpage/text.h"

#include <QApplication>
#include <QColor>
#include <QEvent>

/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
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
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#include <QFile>
#include <QTextStream>

#ifdef USE_KSyntaxHighlighting
#include <syntaxhighlighter.h>
#include <definition.h>
#include <foldingregion.h>
#include <syntaxhighlighter.h>
#include <theme.h>
#endif

TextPreviewPage::TextPreviewPage(QWidget *parent) : BasePreviewPage(parent)
{    
    m_text_edit = new QPlainTextEdit(this);
    m_text_edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    // 预览模式：只读，但仍允许选中和复制文本
    m_text_edit->setReadOnly(true);
    m_text_edit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
#ifdef USE_KSyntaxHighlighting
    m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(m_text_edit->document());
    applyTheme();
#endif

    base_layout->addWidget(m_text_edit);
}

void TextPreviewPage::applyTheme() {
#ifdef USE_KSyntaxHighlighting
    // 以应用级 palette 判断明暗（主题切换的源头，比子控件 palette 更可靠）
    const bool is_dark = qApp->palette().color(QPalette::Base).lightness() < 128;
    const auto theme = is_dark ?
                m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme) :
                m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme);
    m_highlighter->setTheme(theme);

    // 关键：SyntaxHighlighter 只给有语法定义的 token 上色，普通文本(含纯 txt)
    // 的前景/背景由编辑器 palette 决定，必须把 theme 的颜色显式写回编辑器 palette，
    // 否则深色主题下正文仍用旧文字色（表现为黑字看不清）。
    QPalette pal = m_text_edit->palette();
    pal.setColor(QPalette::Base, QColor(theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor)));
    pal.setColor(QPalette::Text, QColor(theme.textColor(KSyntaxHighlighting::Theme::Normal)));
    m_text_edit->setPalette(pal);

    m_highlighter->rehighlight();
#endif
}

void TextPreviewPage::changeEvent(QEvent *event) {
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange) {
        applyTheme();
    }
    BasePreviewPage::changeEvent(event);
}

void TextPreviewPage::cancel() {
    m_text_edit->clear();
    m_file_path.clear();
}

void TextPreviewPage::updateInfo(PreviewHubFileInfo *info) {
    m_file_path = info->filePath();
    QFile file(m_file_path);
    qDebug() << "TextPreviewPage::updateInfo:" << info->uri();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_text_edit->setPlainText(QString("Failed to open %1").arg(m_file_path));
        return;
    }
    QTextStream in(&file);
    m_text_edit->clear();

    int line_cnt = 0, char_cnt = 0;
    while (!in.atEnd()) {
        QString line = in.readLine(max_line_len);
        m_text_edit->appendPlainText(line);
        char_cnt += line.size();
        line_cnt++;
        if (line_cnt > max_line_cnt || char_cnt > max_char_cnt) {
            break;
        }
    }

    m_text_edit->moveCursor(QTextCursor::Start);
#ifdef USE_KSyntaxHighlighting
    const auto def = m_repository.definitionForMimeType(info->mimeType());
    m_highlighter->setDefinition(def);
    m_highlighter->rehighlight();
#endif
    file.close();
}
