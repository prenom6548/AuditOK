/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef CHECKEDLINEEDIT_H
#define CHECKEDLINEEDIT_H

#include "kpasswordedit.h"
#include <QObject>
#include <QResizeEvent>

class QEvent;

namespace KInstaller {

enum class CheckState {
    OK,
    EmptyError,
    TooShortError,
    TooLongError,
    InvalidCharError,
    TooWeakError
};

CheckState KCheckUserName(const QString &username);
CheckState KCheckHostName(const QString &hostname);
CheckState KCheckPassWD(const QString &passwd, QString &strerror, QString user);

} // namespace KInstaller

namespace kdk {

/**
 * @brief OOBE / 锁屏等场景下的密码输入框：在 KPasswordEdit 基础上提供透明背景、白色可见性图标等样式。
 * 校验逻辑见 KInstaller::KCheckPassWD（KPasswordCheck / pwquality）。
 */
class CheckedLineEdit : public KPasswordEdit
{
    Q_OBJECT

public:
    explicit CheckedLineEdit(QWidget *parent = nullptr);

    void setIconsWhite(bool white = true);

    /**
     * @brief OOBE 等场景下输入区域背景是否强制透明（锁屏企业页等可设为 false 以保留自定义底色）
     */
    void setInputBackgroundTransparent(bool transparent);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;

private Q_SLOTS:
    void onEchoModeChanged();
    void onTextChanged(const QString &text);

private:
    void forceRepaintWhiteIcons();
    void applyOobePalette(QPalette *pal);
    void hookChildToolButtons();

    bool m_iconsWhite;
    bool m_iconsPainted;
    bool m_transparentInputBg;
    bool m_inPaletteResync;
};

} // namespace kdk

#endif // CHECKEDLINEEDIT_H
