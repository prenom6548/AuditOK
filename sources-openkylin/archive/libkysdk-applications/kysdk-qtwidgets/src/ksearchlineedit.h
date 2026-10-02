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

#ifndef KSEARCHBAR_H
#define KSEARCHBAR_H

#include "gui_g.h"
#include "ktoolbutton.h"
#include <QLineEdit>
#include <QWidget>

namespace kdk
{

/** @defgroup InputBoxModule
 * @{
 */

class KSearchLineEditPrivate;

/**
 * @brief 搜索输入框，可搜索匹配条件的item
 * @since 1.0.0
 */
class GUI_EXPORT KSearchLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit KSearchLineEdit(QWidget *parent = nullptr);
    ~KSearchLineEdit();

    /**
     * @brief 设置是否可用
     */
    void setEnabled(bool);

    /**
     * @brief 返回是否可用
     * @return
     */
    bool isEnabled();

    /**
     * @brief 设置是否显示清除按钮
     * @return
     */
    void setClearButtonEnabled(bool enable);

    /**
     * @brief 返回是否显示清除按钮
     * @return
     */
    bool isClearButtonEnabled() const;

    /**
     * @brief 返回placeholder
     * @return
     */
    QString placeholderText() const;

    /**
     * @brief 设置placeholder
     * @return
     */
    void setPlaceholderText(const QString &);

    /**
     * @brief 返回placeholder的对齐方式
     * @return
     */
    Qt::Alignment placeholderAlignment() const;

    /**
     * @brief 设置placeholder的对齐方式
     * @return
     */
    void setPlaceholderAlignment(Qt::Alignment flag);

    /**
     * @brief 设置输入文本的对齐方式
     * @return
     */
    Qt::Alignment alignment() const;

    /**
     * @brief 设置输入文本的对齐方式返回
     * @return
     */
    void setAlignment(Qt::Alignment flag);

    /**
     * @brief 设置是否启用半透明效果
     * @since 1.2.0
     * @param flag
     */
    void setTranslucent(bool flag);

    /**
     * @brief 获取是否启用半透明效果
     * @since 1.2.0
     * @return flag
     */
    bool isTranslucent();

    /**
     * @brief 重新加载style
     * @since 2.0.2.1
     */
    void reloadStyle();

    /**
     * @brief 获取自定义按钮
     * @return
     * @since 2.4.1.0
     */
    KToolButton *customButton();

    /**
     * @brief 是否启用渐变色
     * @param flag
     * @since 2.4.1.0
     */
    void setGradation(bool flag);

    /**
     * @brief 设置智能语义按钮是否可见
     * @param flag
     * @since 2.4.1.0
     */
    void setCustomButtonVisible(bool flag);

    /**
     * @brief 返回智能语义按钮是否可见
     * @return bool
     * @since 2.4.1.0
     */
    bool isCustomButtonVisible();

public Q_SLOTS:
    void clear();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void setVisible(bool visible);
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KSearchLineEdit)
    KSearchLineEditPrivate *d_ptr;
};

}
/**
 * @example testsearchlinedit/widget.h
 * @example testsearchlinedit/widget.cpp
 * @}
 */

#endif // KSEARCHBAR_H
