/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
*/
#ifndef RESIZEIMAGEDIALOG_H
#define RESIZEIMAGEDIALOG_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <kpushbutton.h>
#include <kdialog.h>
#include <klineedit.h>
#include "controller/interaction.h"
#include <QSize>
#include <QKeyEvent>
#include <QPalette>
using namespace kdk;
class ResizeImageDialog : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit ResizeImageDialog(QWidget *parent = nullptr);
    void setOriginalSize(int width, int height);  // 设置原图尺寸
    inline void setOriginalSize(const QSize &size){setOriginalSize( size.width(),size.height());}

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_widthLayout = nullptr;
    QHBoxLayout *m_heightLayout = nullptr;
    QHBoxLayout *m_checkBoxLayout = nullptr;
    QHBoxLayout *m_btnLayout = nullptr;

    QLabel *m_widthLabel = nullptr;
    QLabel *m_heightLabel = nullptr;

    QLineEdit *m_widthValue = nullptr;
    QLineEdit *m_heightValue = nullptr;
    QCheckBox *m_checkBox = nullptr;

    kdk::KPushButton *m_save = nullptr;
    kdk::KPushButton *m_cancel = nullptr;

private:
    void initComponents();
    void initConnect();
    void validateInputs();  // 新增：统一的输入验证函数
    int m_origW = 0;
    int m_origH = 0;
    int m_originalWidth = 0;   // 原图宽度
    int m_originalHeight = 0;  // 原图高度
    void updatePlaceholders();  // 更新占位符文本
    QPalette m_widthDefaultPalette;
    QPalette m_heightDefaultPalette;

protected:
    void keyPressEvent(QKeyEvent *event) override;

public Q_SLOTS:
    void widthChanged(QString value);
    void heightChanged(QString value);
    void saveAsCopy();
    void cancel();
    void onWidthEdited(const QString &text);
    void onHeightEdited(const QString &text);


};

#endif // RESIZEIMAGEDIALOG_H
