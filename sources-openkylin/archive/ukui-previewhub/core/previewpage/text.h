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

#ifndef TEXTPREVIEWPAGE_H
#define TEXTPREVIEWPAGE_H

#include "previewpage/base.h"

#include <QPlainTextEdit>

#ifdef USE_KSyntaxHighlighting
#include <repository.h>
namespace KSyntaxHighlighting
{
class SyntaxHighlighter;
class Repository;
}
#endif

class TextPreviewPage : public BasePreviewPage
{
    Q_OBJECT
public:
    explicit TextPreviewPage(QWidget *parent = nullptr);
    ~TextPreviewPage() = default;

protected:
    void changeEvent(QEvent *event) override;

private:
    void applyTheme();

    static const int max_line_cnt = 1000;
    static const int max_line_len = 1000;
    static const int max_char_cnt = 10000;

    QPlainTextEdit *m_text_edit;
    QString m_file_path;

#ifdef USE_KSyntaxHighlighting
    KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::SyntaxHighlighter *m_highlighter;
#endif

    // BasePreviewPage interface
public Q_SLOTS:
    void updateInfo(PreviewHubFileInfo *info) override;
public:
    void cancel() override;

};

#endif // TEXTPREVIEWPAGE_H
