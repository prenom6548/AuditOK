/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef UKUI_SEARCH_AISEARCHPLUGIN_H
#define UKUI_SEARCH_AISEARCHPLUGIN_H

#include <QObject>
#include <QThreadPool>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QAction>
#include <QDBusInterface>
#include <QTimer>

#include "action-label.h"
#include "separation-line.h"
#include "search-plugin-iface.h"
#include "icon-loader.h"
#include "kylin-ai/business/data-management/datamanagement.h"

namespace UkuiSearch {
class LIBSEARCH_EXPORT AiSearchPlugin : public QObject, public SearchPluginIface {
    Q_OBJECT

public:
    explicit AiSearchPlugin(QObject *parent = nullptr);
    ~AiSearchPlugin() override;
    PluginType pluginType() override { return PluginType::SearchPlugin; }
    const QString name() override;
    const QString description() override;
    const QIcon icon() override { return IconLoader::loadIconQt(""); }
    void setEnable(bool enable) override { m_enable = enable; }
    bool isEnable() override { return m_enable; }
    QString getPluginName() override;

    size_t KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult) override;

    void stopSearch() override;

    QList<SearchPluginIface::Actioninfo> getActioninfo(int type) override;

    void openAction(int actionkey, QString key, int type) override;

    QWidget *detailPage(const ResultInfo &ri) override;

    static size_t uniqueSymbolAi;
    static QMutex mutex;

private:
    void initDetailPage();
    static void setAutoTestAttribute(QWidget *widget, const QString &widgetType, const QString &widgetName, const QString &desc);
    QString m_currentActionKey;
    QWidget *m_detailPageWithoutModel;
    QVBoxLayout *m_detailLytWithoutModel = nullptr;
    QLabel *m_iconLabelWithoutModel = nullptr;
    QFrame *m_actionFrameWithoutModel = nullptr;
    QVBoxLayout *m_actionFrameLytWithoutModel = nullptr;
    QLabel *m_actionLabelWithoutModel = nullptr;
    ActionLabel *m_clickableActionLabelWithoutModel = nullptr;

    QWidget *m_detailPage;
    QVBoxLayout *m_detailLyt = nullptr;
    QWidget *m_contentDetailPage;
    QVBoxLayout *m_contentDetailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_nameFrame = nullptr;
    QHBoxLayout *m_nameFrameLyt = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_pluginLabel = nullptr;
    SeparationLine *m_line_1 = nullptr;
    QFrame *m_pathFrame = nullptr;
    QLabel *m_pathLabel1 = nullptr;
    QLabel *m_pathLabel2 = nullptr;
    QHBoxLayout *m_pathFrameLyt = nullptr;
    QFrame *m_timeFrame = nullptr;
    QLabel *m_timeLabel1 = nullptr;
    QLabel *m_timeLabel2 = nullptr;
    QHBoxLayout *m_timeFrameLyt = nullptr;

    SeparationLine *m_line_2 = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;
    ActionLabel *m_actionLabel2 = nullptr;
    ActionLabel *m_actionLabel3 = nullptr;

    QVBoxLayout * m_actionLyt = nullptr;

    bool m_enable = true;
    bool m_useImage = false;
    QIcon m_detailPageIcon;
    QList<SearchPluginIface::Actioninfo> m_actionInfo;
    QThreadPool m_pool;
    QThreadPool m_thumbnailPool;
    DataManagementSession* m_session;
    QTimer *m_timer = nullptr;
    QString m_keyword;
    DataQueue<ResultInfo> *m_searchResult;
};

class AiSearch : public QRunnable {
public:
    explicit AiSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, size_t uniqueSymbol, const QString& keyword);
    ~AiSearch() override;
    static bool createSession();
    static void destroySession();
    static bool modelReady();
    static bool notSupported();

protected:
    void run() override;

private:
    void keywordSearch();
    bool createResultInfo(SearchPluginIface::ResultInfo &ri, const QString& path);

    DataQueue<SearchPluginIface::ResultInfo> *m_searchResult = nullptr;
    size_t m_uniqueSymbol;
    QString m_keyword;
    static QMutex s_sessionMutex;
    static bool s_failed;
    static bool s_aiNotSupported;
    static bool s_supportState;
    static DataManagementSession* s_session;

};
}
#endif //UKUI_SEARCH_AISEARCHPLUGIN_H
