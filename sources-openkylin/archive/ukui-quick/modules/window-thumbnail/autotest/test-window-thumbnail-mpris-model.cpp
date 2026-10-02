/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QGuiApplication>
#include <QHash>
#include <QSignalSpy>
#include <QVector>
#include <QtTest>

#include "../mpris/window-thumbnail-mpris-model.h"

namespace {

class FakeMprisSourceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Row {
        uint pid = 0;
        bool valid = false;
        bool canControl = false;
        bool canGoNext = false;
        bool canGoPrevious = false;
        bool canPlay = false;
        bool canPause = false;
        bool isVideo = false;
        bool isAudio = true;
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : m_rows.size();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
            return {};
        }

        const Row &row = m_rows.at(index.row());
        switch (role) {
        case MprisProperties::Pid:
            return row.pid;
        case MprisProperties::Valid:
            return row.valid;
        case MprisProperties::CanControl:
            return row.canControl;
        case MprisProperties::CanGoNext:
            return row.canGoNext;
        case MprisProperties::CanGoPrevious:
            return row.canGoPrevious;
        case MprisProperties::CanPlay:
            return row.canPlay;
        case MprisProperties::CanPause:
            return row.canPause;
        case MprisProperties::IsCurrentMediaVideo:
            return row.isVideo;
        case MprisProperties::IsCurrentMediaAudio:
            return row.isAudio;
        default:
            return {};
        }
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return {
            {MprisProperties::Pid, "Pid"},
            {MprisProperties::Valid, "Valid"},
            {MprisProperties::CanControl, "CanControl"},
            {MprisProperties::CanGoNext, "CanGoNext"},
            {MprisProperties::CanGoPrevious, "CanGoPrevious"},
            {MprisProperties::CanPlay, "CanPlay"},
            {MprisProperties::CanPause, "CanPause"},
            {MprisProperties::IsCurrentMediaVideo, "IsCurrentMediaVideo"},
            {MprisProperties::IsCurrentMediaAudio, "IsCurrentMediaAudio"},
        };
    }

    QModelIndex appendRowForPid(uint pid,
                                bool valid,
                                bool canControl,
                                bool canGoNext,
                                bool canGoPrevious,
                                bool canPlay,
                                bool canPause,
                                bool isVideo = false,
                                bool isAudio = true)
    {
        beginInsertRows(QModelIndex(), m_rows.size(), m_rows.size());
        m_rows.append(Row{pid, valid, canControl, canGoNext, canGoPrevious, canPlay, canPause, isVideo, isAudio});
        endInsertRows();
        return index(m_rows.size() - 1, 0);
    }

    void setRoleValue(const QModelIndex &index, int role, const QVariant &value)
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
            return;
        }

        Row &row = m_rows[index.row()];
        switch (role) {
        case MprisProperties::Pid:
            row.pid = value.toUInt();
            break;
        case MprisProperties::Valid:
            row.valid = value.toBool();
            break;
        case MprisProperties::CanControl:
            row.canControl = value.toBool();
            break;
        case MprisProperties::CanGoNext:
            row.canGoNext = value.toBool();
            break;
        case MprisProperties::CanGoPrevious:
            row.canGoPrevious = value.toBool();
            break;
        case MprisProperties::CanPlay:
            row.canPlay = value.toBool();
            break;
        case MprisProperties::CanPause:
            row.canPause = value.toBool();
            break;
        case MprisProperties::IsCurrentMediaVideo:
            row.isVideo = value.toBool();
            break;
        case MprisProperties::IsCurrentMediaAudio:
            row.isAudio = value.toBool();
            break;
        default:
            return;
        }

        Q_EMIT dataChanged(index, index, {role});
    }

private:
    QVector<Row> m_rows;
};

} // namespace

class window_thumbnail_mpris_model_test : public QObject
{
    Q_OBJECT

private slots:
    void rowsAreFilteredByPidValidityControlAndOperations();
    void setPidInvalidatesAndEmitsCountChanged();
    void mediaTypeSignalsFollowDataChangedRoles();
};

void window_thumbnail_mpris_model_test::rowsAreFilteredByPidValidityControlAndOperations()
{
    FakeMprisSourceModel source;
    WindowThumbnailMprisModel model;
    model.setSourceModel(&source);

    source.appendRowForPid(0, true, true, true, false, false, false);
    source.appendRowForPid(42, false, true, true, false, false, false);
    source.appendRowForPid(42, true, false, true, false, false, false);
    source.appendRowForPid(42, true, true, false, false, false, false);
    source.appendRowForPid(42, true, true, true, false, false, false);

    QCOMPARE(model.count(), 0);
    model.setPid(42);
    QCOMPARE(model.count(), 1);
}

void window_thumbnail_mpris_model_test::setPidInvalidatesAndEmitsCountChanged()
{
    FakeMprisSourceModel source;
    WindowThumbnailMprisModel model;
    model.setSourceModel(&source);
    source.appendRowForPid(77, true, true, false, false, true, false);

    QCOMPARE(model.desktopEntry(), QString());
    model.setDesktopEntry(QStringLiteral("demo.desktop"));
    QCOMPARE(model.desktopEntry(), QStringLiteral("demo.desktop"));

    QSignalSpy countSpy(&model, &WindowThumbnailMprisModel::countChanged);
    model.setPid(77);

    QVERIFY(countSpy.count() >= 1);
    QCOMPARE(model.pid(), static_cast<uint>(77));
    QCOMPARE(model.count(), 1);
}

void window_thumbnail_mpris_model_test::mediaTypeSignalsFollowDataChangedRoles()
{
    FakeMprisSourceModel source;
    WindowThumbnailMprisModel model;
    model.setSourceModel(&source);
    const QModelIndex row = source.appendRowForPid(88, true, true, false, false, true, false, false, true);
    model.setPid(88);

    QSignalSpy videoSpy(&model, &WindowThumbnailMprisModel::isCurrentMediaVideoChanged);
    QSignalSpy audioSpy(&model, &WindowThumbnailMprisModel::isCurrentMediaAudioChanged);

    source.setRoleValue(row, MprisProperties::IsCurrentMediaVideo, true);
    source.setRoleValue(row, MprisProperties::IsCurrentMediaAudio, false);

    QTRY_VERIFY(videoSpy.count() >= 1);
    QTRY_VERIFY(audioSpy.count() >= 1);
    QVERIFY(model.isCurrentMediaVideo());
    QVERIFY(!model.isCurrentMediaAudio());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");

    QGuiApplication app(argc, argv);
    window_thumbnail_mpris_model_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-window-thumbnail-mpris-model.moc"
