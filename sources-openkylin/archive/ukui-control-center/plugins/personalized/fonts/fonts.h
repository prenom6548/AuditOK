/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef FONTS_H
#define FONTS_H

#include <QObject>
#include <QtPlugin>
#include <QPushButton>
#include <QAbstractButton>
#include <QFontDatabase>
#include <QGSettings>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QtDBus>
#include <kslider.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <stdlib.h>
#include "shell/interface.h"
#include "ukcccommon.h"
#include "ksearchlineedit.h"
#include "kdragwidget.h"
#include "ktoolbutton.h"
#include "blankpage.h"
#include "flowlayout.h"
#include "kballontip.h"
#include "kprogressbar.h"
#include "kprogressdialog.h"
#include "libfun.h"
#include "fontunit.h"
#include <klabel.h>
#include <QComboBox>

using namespace ukcc;
using namespace kdk;

namespace Ui {
class Fonts;
}

class Fonts : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Fonts();
    ~Fonts();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void initSearchText();
    void setupComponent();
    void setupConnect();
    void updateDragWidgetState();
    void initFontStatus();
    int fontConvertToSlider(float size) const;
    float sliderConvertToSize(int value) const;
    void updateFontSizeSliderAccessible();
    void connectToServer();
    void resetDefaultSlot();
    void fontValueChangeSlot();
    void searchTextChangeSlot(QString search);
    void dragWidgetGetPathSlot(QString path);
    void dragWidgetGetPathListSlot(QStringList pathList);
    void styleSettingChangedSlot(const QString &key);
    bool getFontFile(QString fileName);
    void clickFontFile();
    void initFontUnitFrame();
    void deleteAllWidgets(QLayout* layout);
    void clear();
    void insertWidgetAt(QWidget* widget, int index);
    void checkFontFile(QStringList list);
    void addFont(QStringList fontList);
    bool chooseFontFile(QString path);
    bool judgeRepeatFontFile(QString path);
    void updateFontList();
    bool judgeBadFontFile(QString path);
    void updateFontAfterRemoveSlot(QStringList pathList, QString name);
    void loadFontdata();
    void loadBatch(int initSize);
    void comboxTextChangedSlot(const QString& text);
    QStringList splitFontNameSize(QString value);
    void monoSettingChangeSlot(const QString &key);

public Q_SLOTS:
    void keyChangedSlot(const QString &key);
    void showSearchBlankPageSlot();
    void showBlankPageSlot();
    void showFontPageSlot();
    void addItemSlot(FontUnit* unit);

private:
    Ui::Fonts *ui;
    QWidget * pluginWidget;
    QString pluginName;
    int pluginType;

    bool mFirstLoad = true;
    KSlider* m_fontSizeSlider = nullptr;
    QHBoxLayout* m_fontTitleLayout = nullptr;
    QLabel* m_fontTitleLabel = nullptr;
    KSearchLineEdit* m_searchArea;
    QVBoxLayout* m_fontContentLayout = nullptr;
    KDragWidget* m_kDragWidget = nullptr;
    QHBoxLayout* m_fontBottomLayout = nullptr;
    KPushButton* m_fontDefaultButton = nullptr;
    QDBusInterface* m_fontInterface = nullptr;
    QGSettings* m_styleSettings = nullptr;
    QGSettings* m_ukccSettings = nullptr;
    QStackedWidget* m_stackedWid = nullptr;
    BlankPage* m_searchBlankPage = nullptr;
    BlankPage *m_blankPage = nullptr;
    QFrame* m_fontUnitFrame = nullptr;
    FlowLayout* m_flowLayout = nullptr;
    ViewDataList m_allFontData;
    ViewDataList m_installViewData;
    QList<FontUnit*> m_fontUnitList;
    QHash<QString, QStandardItem*> m_fontItemsHash;
    QString m_installPath;
    int m_installSucSize = 0;
    KBallonTip* m_pNormalTip = nullptr;
    KProgressDialog *m_progressDialog = nullptr;
    QComboBox* m_monoCombox = nullptr;
    QGSettings* m_ifsettings = nullptr;
    QGSettings* m_gnomeSettings = nullptr;
    QStringList m_monospacefontStrList;
    QStandardItemModel* m_monoModel = nullptr;
    KLabel* m_monoTitleDesc = nullptr;
};

#endif // FONTS_H
