/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef FONTUNIT_H
#define FONTUNIT_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGSettings>
#include <QDBusInterface>
#include "lightlabel.h"
#include "libfun.h"
#include "popupfontinfo.h"

#include <kysdk/applications/klabel.h>
using namespace kdk;

class FontUnit : public QWidget
{
    Q_OBJECT
public:
    explicit FontUnit(const ViewData* viewData, QString installPath, QWidget *parent = nullptr);
    ~FontUnit();

    void setInstallFlag(bool flag);
    void updateViewData(ViewData* viewData);

private:
    void setWidgetUi();
    void setWidgetLayout();
    void setupConnect();
    void setWidgetContent();
    void fontSizeChangeSlot();
    bool isUsingFont();
    void fontStyleChangedSlot(QString style, QString path);
    void fontStyleRemovedSlot(QString path, QString newStyle, QString newPath);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

public slots:
    void applyFontSlot();
    void removeFontSlot();
    void checkFontSlot();
    void styleSettingChangeSlot();
    ViewData* getViewData();

Q_SIGNALS:
    void remoteFontSig(QStringList pathList, QString name);

private:
    QLabel* m_fontPreviewLabel = nullptr;
    KLabel* m_fontNameLabel = nullptr;
    KLabel* m_fontStyleLabel = nullptr;
    QToolButton* m_fontToolButton = nullptr;
    QMenu *m_fontMenu = nullptr;
    QAction* m_applyFontAction = nullptr;
    QAction* m_removeFontAction = nullptr;
    QAction* m_checkFontAction = nullptr;
    QVBoxLayout* m_leftVerLayout = nullptr;
    QHBoxLayout* m_bottomHorLayout = nullptr;
    QVBoxLayout* m_baseVerLayout = nullptr;
    ViewData* m_viewData = nullptr;
    QGSettings* m_styleSettings = nullptr;
    QGSettings* m_gtkSettings = nullptr;
    QDBusInterface* m_fontInterface = nullptr;
    PopupFontInfo* m_popupFontInfo = nullptr;
    QString m_installPath;
    bool m_isMouseOpt = false;
    bool m_installFlag = false;
    bool m_applyFont = false;
};

#endif // FONTUNIT_H
