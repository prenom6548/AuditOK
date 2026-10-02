/*
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "app-category-model.h"
#include "basic-app-filter-model.h"

#include <QDebug>

namespace UkuiMenu {

// GCOVR_EXCL_BR_START
AppCategoryModel::AppCategoryModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    m_mainCategories.insert(QStringLiteral("AudioVideo"), {tr("AudioVideo"), 0});
    m_mainCategories.insert(QStringLiteral("Audio"), {tr("Audio"), 1});
    m_mainCategories.insert(QStringLiteral("Video"), {tr("Video"), 2});
    m_mainCategories.insert(QStringLiteral("Development"), {tr("Development"), 3});
    m_mainCategories.insert(QStringLiteral("Education"), {tr("Education"), 4});
    m_mainCategories.insert(QStringLiteral("Game"), {tr("Game"), 5});
    m_mainCategories.insert(QStringLiteral("Graphics"), {tr("Graphics"), 6});
    m_mainCategories.insert(QStringLiteral("Network"), {tr("Network"), 7});
    m_mainCategories.insert(QStringLiteral("Office"), {tr("Office"), 8});
    m_mainCategories.insert(QStringLiteral("Science"), {tr("Science"), 9});
    m_mainCategories.insert(QStringLiteral("Settings"), {tr("Settings"), 10});
    m_mainCategories.insert(QStringLiteral("System"), {tr("System"), 11});
    m_mainCategories.insert(QStringLiteral("Utility"), {tr("Utility"), 12});
    m_mainCategories.insert(QStringLiteral("Other"), {tr("Other"), 13});

    m_additionalCategories.insert(QStringLiteral("AudioVideo"),
                                  {"Midi", "Mixer", "Sequencer", "Tuner", "TV", "AudioVideoEditing",
                                   "Player", "Recorder", "DiscBurning", "Music"});
    m_additionalCategories.insert(QStringLiteral("Development"),
                                  {"Building", "Debugger", "IDE", "GUIDesigner", "Profiling",
                                   "RevisionControl", "Translation"});
    m_additionalCategories.insert(QStringLiteral("Education"),
                                  {"Art", "Construction", "Languages", "ArtificialIntelligence",
                                   "Astronomy", "Biology", "Chemistry", "ComputerScience", "DataVisualization",
                                   "Economy", "Electricity", "Geography", "Geology", "Geoscience",
                                   "History", "Humanities", "ImageProcessing", "Literature", "Maps",
                                   "Math", "NumericalAnalysis", "MedicalSoftware", "Physics", "Robotics",
                                   "Spirituality", "Sports", "ParallelComputing"});
    m_additionalCategories.insert(QStringLiteral("Game"),
                                  {"ActionGame", "AdventureGame", "ArcadeGame", "BoardGame", "BlocksGame",
                                   "CardGame", "KidsGame", "LogicGame", "RolePlaying", "Shooter",
                                   "Simulation", "SportsGame", "StrategyGame", "LauncherStore", "GameTool"});
    m_additionalCategories.insert(QStringLiteral("Graphics"),
                                  {"2DGraphics", "VectorGraphics", "RasterGraphics", "3DGraphics", "Scanning",
                                   "OCR", "Photography", "Publishing", "Viewer"});
    m_additionalCategories.insert(QStringLiteral("Network"),
                                  {"Dialup", "InstantMessaging", "Chat", "IRCClient", "Feed", "FileTransfer",
                                   "HamRadio", "News", "P2P", "RemoteAccess", "Telephony", "VideoConference",
                                   "WebBrowser", "WebDevelopment"});
    m_additionalCategories.insert(QStringLiteral("Office"),
                                  {"Calendar", "ContactManagement", "Database", "Dictionary", "Chart",
                                   "Email", "Finance", "FlowChart", "PDA", "ProjectManagement",
                                   "Presentation", "Spreadsheet", "WordProcessor"});
    m_additionalCategories.insert(QStringLiteral("Settings"),
                                  {"DesktopSettings", "HardwareSettings", "Printing", "PackageManager",
                                   "Security", "Accessibility"});
    m_additionalCategories.insert(QStringLiteral("System"),
                                  {"Emulator", "FileManager", "TerminalEmulator", "Filesystem", "Monitor"});
    m_additionalCategories.insert(QStringLiteral("Utility"),
                                  {"TextTools", "TelephonyTools", "Archiving", "Compression", "FileTools",
                                   "Calculator", "Clock", "TextEditor"});

    QSortFilterProxyModel::setSourceModel(BasicAppFilterModel::instance());
    QSortFilterProxyModel::sort(0);
}
// GCOVR_EXCL_BR_STOP

AppCategoryModel::Mode AppCategoryModel::mode() const
{
    return m_mode;
}

void AppCategoryModel::setMode(AppCategoryModel::Mode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    // TODO: 刷新
    invalidateFilter();
    invalidate();
}

QVariant AppCategoryModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    if (role == DataEntity::Group) {
        if (m_mode == All) {
            return "";
        } else if (m_mode == FirstLatter) {
            return AppCategoryModel::getFirstLatterUpper(QSortFilterProxyModel::data(index, DataEntity::FirstLetter).toString());
        } else {
            return getCategoryName(QSortFilterProxyModel::data(index, DataEntity::Category).toString());
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

bool AppCategoryModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_mode == All) {
        return false;
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool AppCategoryModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int result = 0;
    if (m_mode == Category) {
        // 按功能分类进行排序
        int leftIndex = getCategoryIndex(source_left.data(DataEntity::Category).toString());
        int rightIndex = getCategoryIndex(source_right.data(DataEntity::Category).toString());

        result = (leftIndex < rightIndex) ? -1 : (leftIndex > rightIndex) ? 1 : 0;
    }

    // 功能分类相同时，也按字母排序
    if ((result == 0) || (m_mode == FirstLatter)) {
        QString leftKey = source_left.data(DataEntity::FirstLetter).toString();
        QString rightKey = source_right.data(DataEntity::FirstLetter).toString();

        result = leftKey.compare(rightKey, Qt::CaseInsensitive);
    }

    return result < 0;
}

inline QString AppCategoryModel::getCategoryName(const QString &categories) const
{
    return m_mainCategories[getCategoryKey(categories)].first;
}

inline int AppCategoryModel::getCategoryIndex(const QString &categories) const
{
    return m_mainCategories[getCategoryKey(categories)].second;
}

QString AppCategoryModel::getCategoryKey(const QString &categories) const
{
    QStringList list;
    // 某些应用使用空格(" ")进行分隔，此处做一下处理
    if (categories.contains( QStringLiteral(";"))) {
        list = categories.split(QStringLiteral(";"));
    } else {
        list = categories.split(QStringLiteral(" "));
    }

    for (const auto &key : list) {
        if (m_mainCategories.contains(key)) {
            return key;
        }
    }

    QMap<QString, QStringList>::const_iterator it;
    for (const auto &key : list) {
        for (const auto & pair : m_additionalCategories) {
            for (it = m_additionalCategories.constBegin(); it != m_additionalCategories.constEnd(); ++it) {
                if (it.value().contains(key)) {
                    return it.key();
                }
            }
        }
    }

    return QStringLiteral("Other");
}

QString AppCategoryModel::getFirstLatterUpper(const QString &pinyinName)
{
    if (!pinyinName.isEmpty()) {
        QChar first = pinyinName.at(0).toUpper();
        if (first >= 'A' && first <= 'Z') {
            return first;
        } else if (first >= '0' && first <= '9') {
            return QStringLiteral("#");
        }
    }

    return QStringLiteral("&");
}

} // UkuiMenu
