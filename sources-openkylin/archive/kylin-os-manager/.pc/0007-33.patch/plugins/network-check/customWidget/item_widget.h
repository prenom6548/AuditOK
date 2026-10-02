/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef ITEM_WIDGET_H
#define ITEM_WIDGET_H

#include <QWidget>
#include <custom_label.h>
#include "detailbutton.h"
#include <QPushButton>
#include <QGSettings>
#include "return_code.h"
#include <kborderlessbutton.h>
const QByteArray UKUI_STYLE = "org.ukui.style";
const QString UKUI_THEME = "styleName";

using namespace kdk;

namespace Ui
{
class ItemWidget;
}
// enum class CheckStatus : int { INIT = 0, CHECKING, EVERTHING_IS_OK, ERROR, WARNING, };
class ItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ItemWidget(QWidget *parent = nullptr);
    ~ItemWidget();
    void statusChanged(const statusStruct &curStatus);
    void setDetailMode(bool trigger);
    void setStatusCheck(CheckStatus type);
    void setCheckPoint(QString checkPointName);
    void setCheckInfo(QString checkDetailInfo);
    void setCheckRes(const QString &checkDetailInfo, const QString &checkResult);

    void setInitStyle();

    void setCheckingStyle();

    void setFailedStyle();

    void setSucceedStyle();

    void setWarningStyle();

    void setSystemFontSizeChange(int size);
signals:
    void showContent();

private:
    CustomLabel *checkInfo = nullptr;
    CustomLabel *checkPoint = nullptr;
    CustomLabel *checkRes = nullptr;
    KBorderlessButton *showContentBtn = nullptr;

    void initSettings();
    void setThemeStyle();

    QGSettings *m_themeSettings = nullptr;
    QString m_curStyle = "default-style";
    CheckStatus m_curType = CheckStatus::INIT;

    bool m_isArrowDown = true;
};

#endif // ITEM_WIDGET_H
