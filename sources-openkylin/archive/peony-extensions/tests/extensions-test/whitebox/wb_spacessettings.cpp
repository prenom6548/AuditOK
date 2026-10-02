/*
 * 白盒：peony-intelligent-data-management-service/model/spacessettings.{h,cpp}
 */
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QObject>
#include <QSettings>
#include <QSignalSpy>
#include <QThread>
#include <QVariant>
#include "spacessettings.h"

namespace {

class SpacessettingsWhiteboxTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        s = SpacesSettings::globalInstance();
        ASSERT_NE(s, nullptr);
    }
    SpacesSettings *s = nullptr;
};

// ---------- SpacesSettings::globalInstance / 单例 ----------
TEST_F(SpacessettingsWhiteboxTest, globalInstance_returns_same_pointer)
{
    EXPECT_EQ(SpacesSettings::globalInstance(), s);
}

// ---------- getSortOrder / setSortOrder ----------
TEST_F(SpacessettingsWhiteboxTest, sort_order_idempotent_and_round_trip)
{
    const Qt::SortOrder before = s->getSortOrder();
    s->setSortOrder(before);
    EXPECT_EQ(s->getSortOrder(), before);

    s->setSortOrder(Qt::AscendingOrder);
    EXPECT_EQ(s->getSortOrder(), Qt::AscendingOrder);
    s->setSortOrder(Qt::DescendingOrder);
    EXPECT_EQ(s->getSortOrder(), Qt::DescendingOrder);

    QThread::msleep(600);
    QCoreApplication::processEvents();
}

// ---------- getSortType / setSortType ----------
TEST_F(SpacessettingsWhiteboxTest, sort_type_idempotent)
{
    s->setSortType(3);
    EXPECT_EQ(s->getSortType(), 3);
    s->setSortType(3);
    EXPECT_EQ(s->getSortType(), 3);
}

// ---------- getUserAddedFirst / setUserAddedFirst（与 setSortType 联动）----------
TEST_F(SpacessettingsWhiteboxTest, user_added_first_resets_sort_type_per_impl)
{
    const int target = s->getUserAddedFirst() == 9 ? 10 : 9;
    s->setSortType(3);
    s->setUserAddedFirst(target);
    EXPECT_EQ(s->getUserAddedFirst(), target);
    EXPECT_EQ(s->getSortType(), 1);
}

// ---------- getUseGlobalSorting / setUseGlobalSorting ----------
TEST_F(SpacessettingsWhiteboxTest, use_global_sorting_toggle)
{
    s->setUseGlobalSorting(false);
    EXPECT_FALSE(s->getUseGlobalSorting());
    s->setUseGlobalSorting(false);
    s->setUseGlobalSorting(true);
    EXPECT_TRUE(s->getUseGlobalSorting());
}

// ---------- setSpacesSortConfig / getSpacesSortConfig ----------
TEST_F(SpacessettingsWhiteboxTest, spaces_sort_config_keys_present)
{
    s->setSpacesSortConfig(4242, 2, 0, 5);
    QVariantHash h = s->getSpacesSortConfig(4242);
    EXPECT_TRUE(h.contains(QStringLiteral("sortType")));
    EXPECT_TRUE(h.contains(QStringLiteral("sortOrder")));
    EXPECT_TRUE(h.contains(QStringLiteral("userAddedFirstType")));

    QVariantHash hMissing = s->getSpacesSortConfig(99999);
    EXPECT_TRUE(hMissing.contains(QStringLiteral("sortType")));
}

// ---------- getIsSpacesCollapsed / setIsSpacesCollapsed ----------
TEST_F(SpacessettingsWhiteboxTest, is_spaces_collapsed_idempotent)
{
    s->setIsSpacesCollapsed(true);
    EXPECT_TRUE(s->getIsSpacesCollapsed());
    s->setIsSpacesCollapsed(true);
    EXPECT_TRUE(s->getIsSpacesCollapsed());
}

// ---------- getIsRecentCollapsed / setIsRecentCollapsed ----------
TEST_F(SpacessettingsWhiteboxTest, is_recent_collapsed)
{
    s->setIsRecentCollapsed(true);
    EXPECT_TRUE(s->getIsRecentCollapsed());
}

TEST_F(SpacessettingsWhiteboxTest, is_recent_collapsed_idempotent_early_return)
{
    const bool v = s->getIsRecentCollapsed();
    s->setIsRecentCollapsed(v);
    EXPECT_EQ(s->getIsRecentCollapsed(), v);
}

// ---------- getIsSpaceListMode / setIsSpaceListMode ----------
TEST_F(SpacessettingsWhiteboxTest, is_space_list_mode)
{
    s->setIsSpaceListMode(true);
    EXPECT_TRUE(s->getIsSpaceListMode());
}

TEST_F(SpacessettingsWhiteboxTest, is_space_list_mode_idempotent_early_return)
{
    const bool v = s->getIsSpaceListMode();
    s->setIsSpaceListMode(v);
    EXPECT_EQ(s->getIsSpaceListMode(), v);
}

// ---------- getIsPreviewMode / setIsPreviewMode ----------
TEST_F(SpacessettingsWhiteboxTest, is_preview_mode)
{
    s->setIsPreviewMode(true);
    EXPECT_TRUE(s->getIsPreviewMode());
}

TEST_F(SpacessettingsWhiteboxTest, is_preview_mode_idempotent_early_return)
{
    const bool v = s->getIsPreviewMode();
    s->setIsPreviewMode(v);
    EXPECT_EQ(s->getIsPreviewMode(), v);
}

// ---------- explicit SpacesSettings(QObject *) ----------
TEST_F(SpacessettingsWhiteboxTest, stack_instance_reads_settings_group)
{
    QObject holder;
    SpacesSettings local(&holder);
    EXPECT_GE(local.getSortType(), 0);
    local.setSortType(local.getSortType());
}

// ---------- sortConfigChanged（delayer 超时后由实现发出）----------
TEST_F(SpacessettingsWhiteboxTest, sort_config_changed_after_debounce)
{
    QSignalSpy spy(s, &SpacesSettings::sortConfigChanged);
    s->setSortType(s->getSortType() == 0 ? 1 : 0);
    QThread::msleep(600);
    QCoreApplication::processEvents();
    EXPECT_GE(spy.count(), 1);
}

// ---------- setIsSpacesCollapsed 切换覆盖函数体 (lines 196-203) ----------
TEST_F(SpacessettingsWhiteboxTest, is_spaces_collapsed_toggle_covers_setter_body)
{
    bool before = s->getIsSpacesCollapsed();
    s->setIsSpacesCollapsed(!before);
    EXPECT_EQ(s->getIsSpacesCollapsed(), !before);
    s->setIsSpacesCollapsed(before);
    EXPECT_EQ(s->getIsSpacesCollapsed(), before);
}

// ---------- setIsRecentCollapsed 切换覆盖函数体 (lines 214-221) ----------
TEST_F(SpacessettingsWhiteboxTest, is_recent_collapsed_toggle_covers_setter_body)
{
    bool before = s->getIsRecentCollapsed();
    s->setIsRecentCollapsed(!before);
    EXPECT_EQ(s->getIsRecentCollapsed(), !before);
    s->setIsRecentCollapsed(before);
    EXPECT_EQ(s->getIsRecentCollapsed(), before);
}

// ---------- setIsSpaceListMode 切换覆盖函数体 (lines 232-239) ----------
TEST_F(SpacessettingsWhiteboxTest, is_space_list_mode_toggle_covers_setter_body)
{
    bool before = s->getIsSpaceListMode();
    s->setIsSpaceListMode(!before);
    EXPECT_EQ(s->getIsSpaceListMode(), !before);
    s->setIsSpaceListMode(before);
    EXPECT_EQ(s->getIsSpaceListMode(), before);
}

// ---------- setIsPreviewMode 切换覆盖函数体 (lines 250-257) ----------
TEST_F(SpacessettingsWhiteboxTest, is_preview_mode_toggle_covers_setter_body)
{
    bool before = s->getIsPreviewMode();
    s->setIsPreviewMode(!before);
    EXPECT_EQ(s->getIsPreviewMode(), !before);
    s->setIsPreviewMode(before);
    EXPECT_EQ(s->getIsPreviewMode(), before);
}

// ---------- setUserAddedFirst 相同值 → 覆盖早退 (line 134) ----------
TEST_F(SpacessettingsWhiteboxTest, set_user_added_first_same_value_early_return)
{
    int current = s->getUserAddedFirst();
    s->setUserAddedFirst(current);
    EXPECT_EQ(s->getUserAddedFirst(), current);
}

// ---------- getSpacesSortConfig 非 null userAddedFirstType → 覆盖 line 179 ----------
TEST_F(SpacessettingsWhiteboxTest, spaces_sort_config_user_added_first_type_non_null)
{
    // 直接写入 QSettings，使用与 getSpacesSortConfig 读取时相同的 key 名（"userAddedFirstType"）
    {
        QSettings settings;
        settings.beginGroup(QStringLiteral("Spaces"));
        settings.beginWriteArray(QStringLiteral("8888"), 1);
        settings.setValue(QStringLiteral("userAddedFirstType"), 77);
        settings.endArray();
        settings.endGroup();
        settings.sync();
    }
    QVariantHash h = s->getSpacesSortConfig(8888);
    EXPECT_TRUE(h.contains(QStringLiteral("userAddedFirstType")));
    EXPECT_EQ(h.value(QStringLiteral("userAddedFirstType")).toInt(), 77);
}

// ---------- getSpacesSortConfig 已存在 sortType/sortOrder → 覆盖 lines 165, 172 ----------
TEST_F(SpacessettingsWhiteboxTest, spaces_sort_config_sort_type_order_non_null)
{
    s->setSpacesSortConfig(7777, 2, 0, 5);
    QVariantHash h = s->getSpacesSortConfig(7777);
    EXPECT_TRUE(h.contains(QStringLiteral("sortType")));
    EXPECT_TRUE(h.contains(QStringLiteral("sortOrder")));
}

// ---------- setUserAddedFirst 切换后再切回 → 覆盖所有信号路径 ----------
TEST_F(SpacessettingsWhiteboxTest, set_user_added_first_change_and_revert)
{
    int before = s->getUserAddedFirst();
    int other = (before == 7) ? 8 : 7;
    s->setUserAddedFirst(other);
    EXPECT_EQ(s->getUserAddedFirst(), other);
    EXPECT_EQ(s->getSortType(), 1);
    s->setUserAddedFirst(before);
    EXPECT_EQ(s->getUserAddedFirst(), before);
}

// ---------- SpacesSettings 独立实例信号覆盖 ----------
TEST_F(SpacessettingsWhiteboxTest, local_instance_signals_emitted)
{
    SpacesSettings local;
    QSignalSpy spySortOrder(&local, &SpacesSettings::sortOrderChanged);
    QSignalSpy spySortType(&local, &SpacesSettings::sortTypeChanged);
    QSignalSpy spyUseGlobal(&local, &SpacesSettings::useGlobalSortingChanged);
    QSignalSpy spyUserAdded(&local, &SpacesSettings::userAddedFirstChanged);
    QSignalSpy spyCollapsed(&local, &SpacesSettings::isSpacesCollapsedChanged);
    QSignalSpy spyRecent(&local, &SpacesSettings::isRecentCollapsedChanged);
    QSignalSpy spyListMode(&local, &SpacesSettings::isSpaceListModeChanged);
    QSignalSpy spyPreview(&local, &SpacesSettings::isPreviewModeChanged);

    local.setSortOrder(local.getSortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder);
    EXPECT_EQ(spySortOrder.count(), 1);

    local.setSortType(local.getSortType() == 0 ? 1 : 0);
    EXPECT_EQ(spySortType.count(), 1);

    local.setUseGlobalSorting(!local.getUseGlobalSorting());
    EXPECT_EQ(spyUseGlobal.count(), 1);

    local.setUserAddedFirst(local.getUserAddedFirst() == 7 ? 8 : 7);
    EXPECT_EQ(spyUserAdded.count(), 1);

    local.setIsSpacesCollapsed(!local.getIsSpacesCollapsed());
    EXPECT_EQ(spyCollapsed.count(), 1);

    local.setIsRecentCollapsed(!local.getIsRecentCollapsed());
    EXPECT_EQ(spyRecent.count(), 1);

    local.setIsSpaceListMode(!local.getIsSpaceListMode());
    EXPECT_EQ(spyListMode.count(), 1);

    local.setIsPreviewMode(!local.getIsPreviewMode());
    EXPECT_EQ(spyPreview.count(), 1);
}

} // namespace
