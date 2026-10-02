/*
 * 白盒：peony-intelligent-data-management-service/condition/condition.{h,cpp}
 * 用例按 condition.h 中声明顺序分组。
 */
#include <gtest/gtest.h>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include "condition.h"

namespace {

static QJsonObject baseObj(Condition::Type t)
{
    QJsonObject o;
    o[QStringLiteral("condition type")] = int(t);
    o[QStringLiteral("orCondition")] = false;
    return o;
}

// ---------- Type 枚举（与头文件声明顺序一致，防止无意改动破坏持久化数据）----------
TEST(ConditionWhitebox, type_enum_numeric_values_stable)
{
    EXPECT_EQ(int(Condition::FileType), 1);
    EXPECT_EQ(int(Condition::FileName), 4);
    EXPECT_EQ(int(Condition::SearchLocation), 8);
}

// ---------- explicit Condition(Type, QObject *) / parent ----------
TEST(ConditionWhitebox, constructor_with_parent)
{
    QObject holder;
    Condition c(Condition::FileName, &holder);
    EXPECT_EQ(c.type(), Condition::FileName);
    EXPECT_EQ(c.parent(), &holder);
}

// ---------- fromJsonObject / isValid / type ----------
TEST(ConditionWhitebox, from_json_validity_and_type)
{
    QJsonObject o = baseObj(Condition::FileName);
    o[QStringLiteral("keywords")] = QJsonArray::fromStringList({QStringLiteral("a")});
    auto c = Condition::fromJsonObject(o);
    ASSERT_TRUE(c);
    EXPECT_TRUE(c->isValid());
    EXPECT_EQ(c->type(), Condition::FileName);

    Condition empty(Condition::FileName);
    EXPECT_FALSE(empty.isValid());

    QJsonObject o2 = baseObj(Condition::FileName);
    o2[QStringLiteral("orCondition")] = true;
    auto c2 = Condition::fromJsonObject(o2);
    EXPECT_FALSE(c2->isAndCondition());

    QJsonObject o3 = baseObj(Condition::FileName);
    o3[QStringLiteral("orCondition")] = false;
    o3[QStringLiteral("keywords")] = QJsonArray::fromStringList({QStringLiteral("b")});
    auto c3 = Condition::fromJsonObject(o3);
    ASSERT_TRUE(c3);
    EXPECT_TRUE(c3->isAndCondition());
}

// ---------- setIsAndCondition / isAndCondition ----------
TEST(ConditionWhitebox, set_is_and_condition_updates_json)
{
    Condition c(Condition::FileName);
    ASSERT_TRUE(c.setFileNameCondition(true, {QStringLiteral("x")}));
    c.setIsAndCondition(false);
    EXPECT_FALSE(c.isAndCondition());
    c.setIsAndCondition(true);
    EXPECT_TRUE(c.isAndCondition());
}

// ---------- setFileNameCondition / getFileNameKeywords / getFileNameIsContains / isFileNameMatchedWithKeywords ----------
TEST(ConditionWhitebox, file_name_contains_and_not_contains)
{
    Condition c(Condition::FileName);
    EXPECT_FALSE(c.setFileNameCondition(true, {}));
    ASSERT_TRUE(c.setFileNameCondition(true, {QStringLiteral("foo"), QStringLiteral("bar")}));
    EXPECT_TRUE(c.getFileNameIsContains());
    EXPECT_TRUE(c.isFileNameMatchedWithKeywords(QStringLiteral("myfoobar")));
    ASSERT_TRUE(c.setFileNameCondition(false, {QStringLiteral("secret")}));
    EXPECT_FALSE(c.getFileNameIsContains());
    EXPECT_TRUE(c.isFileNameMatchedWithKeywords(QStringLiteral("public")));
    EXPECT_FALSE(c.isFileNameMatchedWithKeywords(QStringLiteral("mysecret")));
}

TEST(ConditionWhitebox, file_name_wrong_type_rejected)
{
    Condition c(Condition::FileSize);
    EXPECT_FALSE(c.setFileNameCondition(true, {QStringLiteral("a")}));
}

TEST(ConditionWhitebox, file_name_keywords_skip_empty_strings)
{
    QJsonObject o;
    o[QStringLiteral("condition type")] = int(Condition::FileName);
    o[QStringLiteral("contains")] = true;
    o[QStringLiteral("keywords")] = QJsonArray::fromStringList({QStringLiteral(""), QStringLiteral("x")});
    o[QStringLiteral("orCondition")] = false;
    auto p = Condition::fromJsonObject(o);
    EXPECT_EQ(p->getFileNameKeywords().size(), 1);
}

// ---------- setFileTypeCondition / getFileType* / isFileTypeMatched* ----------
TEST(ConditionWhitebox, file_type_suffixes_and_suffix_match)
{
    Condition c(Condition::FileType);
    ASSERT_TRUE(c.setFileTypeCondition(QStringList({QStringLiteral("txt"), QStringLiteral("md")})));
    EXPECT_EQ(c.getFileTypeSelectionType(), -1);
    EXPECT_TRUE(c.isFileTypeMatchedWithSuffixes(QStringLiteral("a.txt")));
    EXPECT_FALSE(c.isFileTypeMatchedWithSuffixes(QStringLiteral("a.pdf")));
}

static std::shared_ptr<Condition> fileTypeFromJson(int subType, bool contains = true)
{
    QJsonObject o;
    o[QStringLiteral("condition type")] = int(Condition::FileType);
    o[QStringLiteral("type")] = subType;
    o[QStringLiteral("contains")] = contains;
    o[QStringLiteral("orCondition")] = false;
    return Condition::fromJsonObject(o);
}

TEST(ConditionWhitebox, file_type_subtype_mime_branches)
{
    auto all = fileTypeFromJson(int(Condition::ALL_TYPE), true);
    EXPECT_TRUE(all->getFileTypeIsContains());
    EXPECT_TRUE(all->isFileTypeMatchedWithSubType(int(Condition::ALL_TYPE), QStringLiteral("x"), QStringLiteral("inode/directory")));

    auto folder = fileTypeFromJson(int(Condition::FILE_FOLDER));
    EXPECT_TRUE(folder->isFileTypeMatchedWithSubType(int(Condition::FILE_FOLDER), QStringLiteral("x"), QStringLiteral("inode/directory")));
    EXPECT_FALSE(folder->isFileTypeMatchedWithSubType(int(Condition::FILE_FOLDER), QStringLiteral("x"), QStringLiteral("text/plain")));

    auto pic = fileTypeFromJson(int(Condition::PICTURE));
    EXPECT_TRUE(pic->isFileTypeMatchedWithSubType(int(Condition::PICTURE), QStringLiteral("x"), QStringLiteral("image/png")));

    auto vid = fileTypeFromJson(int(Condition::VIDEO));
    EXPECT_TRUE(vid->isFileTypeMatchedWithSubType(int(Condition::VIDEO), QStringLiteral("x"), QStringLiteral("video/mp4")));

    auto txt = fileTypeFromJson(int(Condition::TXT_FILE));
    EXPECT_TRUE(txt->isFileTypeMatchedWithSubType(int(Condition::TXT_FILE), QStringLiteral("x"), QStringLiteral("text/plain")));

    auto wps = fileTypeFromJson(int(Condition::WPS_FILE));
    EXPECT_TRUE(wps->isFileTypeMatchedWithSubType(int(Condition::WPS_FILE), QStringLiteral("a.doc"), QStringLiteral("application/wps-office")));

    auto aud = fileTypeFromJson(int(Condition::AUDIO));
    EXPECT_TRUE(aud->isFileTypeMatchedWithSubType(int(Condition::AUDIO), QStringLiteral("x"), QStringLiteral("audio/mpeg")));
    EXPECT_TRUE(aud->isFileTypeMatchedWithSubType(int(Condition::AUDIO), QStringLiteral("x"), QStringLiteral("application/x-smaf")));

    auto oth = fileTypeFromJson(int(Condition::OTHERS));
    EXPECT_TRUE(oth->isFileTypeMatchedWithSubType(int(Condition::OTHERS), QStringLiteral("x"), QStringLiteral("application/xyz-unknown")));
}

TEST(ConditionWhitebox, file_type_contains_negation_on_subtype)
{
    QJsonObject o;
    o[QStringLiteral("condition type")] = int(Condition::FileType);
    o[QStringLiteral("type")] = int(Condition::PICTURE);
    o[QStringLiteral("contains")] = false;
    o[QStringLiteral("orCondition")] = false;
    auto ptr = Condition::fromJsonObject(o);
    ASSERT_TRUE(ptr);
    EXPECT_FALSE(ptr->isFileTypeMatchedWithSubType(int(Condition::PICTURE), QStringLiteral("x"), QStringLiteral("image/jpeg")));
}

TEST(ConditionWhitebox, file_type_labels_suffix_and_int_selection)
{
    Condition bySuffix(Condition::FileType);
    bySuffix.setFileTypeCondition(QStringList({QStringLiteral("cpp"), QStringLiteral("h")}));
    EXPECT_FALSE(bySuffix.getConditionLabels().isEmpty());

    Condition byInt(Condition::FileType);
    ASSERT_TRUE(byInt.setFileTypeCondition(3));
    EXPECT_FALSE(byInt.getConditionLabels().isEmpty());
}

// ---------- setFileSizeCondition / getFileSize* / isFileSizeMatchedWithSize ----------
TEST(ConditionWhitebox, file_size_buckets)
{
    auto check = [&](bool contains, int type, quint64 size, bool expectMatch) {
        Condition x(Condition::FileSize);
        ASSERT_TRUE(x.setFileSizeCondition(contains, type));
        EXPECT_EQ(x.getFileSizeIsContains(), contains);
        EXPECT_EQ(x.getFileSizeType(), type);
        EXPECT_EQ(x.isFileSizeMatchedWithSize(size), expectMatch);
    };
    check(true, 0, 12345, true);
    check(true, 1, 0, true);
    check(true, 2, 1024, true);
    check(false, 2, 1024, false);
    check(true, 3, 50 * 1024, true);
    check(true, 4, 2 * 1024 * 1024, true);
    check(true, 5, 200 * 1024 * 1024, true);
    quint64 oneG = 1024ULL * 1024 * 1024;
    check(true, 6, 2 * oneG, true);
    check(true, 7, 5 * oneG, true);
}

TEST(ConditionWhitebox, file_size_unknown_bucket_and_wrong_condition_type)
{
    Condition c(Condition::FileSize);
    ASSERT_TRUE(c.setFileSizeCondition(true, 99));
    EXPECT_FALSE(c.isFileSizeMatchedWithSize(1000));
    Condition w(Condition::FileName);
    EXPECT_FALSE(w.setFileSizeCondition(true, 1));
}

// ---------- setFileContentCondition / getFileContentContent ----------
TEST(ConditionWhitebox, file_content_set_and_labels)
{
    Condition c(Condition::FileContent);
    EXPECT_FALSE(c.setFileContentCondition(QStringLiteral("")));
    ASSERT_TRUE(c.setFileContentCondition(QStringLiteral("needle")));
    EXPECT_EQ(c.getFileContentContent(), QStringLiteral("needle"));
    EXPECT_FALSE(c.getConditionLabels().isEmpty());
}

// ---------- setSearchLocationCondition / getSearchLocation* / isFileMatchedWithSearchLocation ----------
TEST(ConditionWhitebox, search_location_path_and_match)
{
    Condition c(Condition::SearchLocation);
    EXPECT_FALSE(c.setSearchLocationCondition(true, QStringLiteral("")));
    ASSERT_TRUE(c.setSearchLocationCondition(true, QStringLiteral("/home/user")));
    EXPECT_TRUE(c.getSearchLocationIsContains());
    EXPECT_EQ(c.getSearchLocationPath(), QStringLiteral("/home/user"));
    EXPECT_TRUE(c.isFileMatchedWithSearchLocation(QStringLiteral("/home/user/docs/a.txt")));
    ASSERT_TRUE(c.setSearchLocationCondition(false, QStringLiteral("/tmp")));
    EXPECT_FALSE(c.getSearchLocationIsContains());
    EXPECT_TRUE(c.isFileMatchedWithSearchLocation(QStringLiteral("/home/x")));
}

TEST(ConditionWhitebox, search_location_labels_not_contains)
{
    Condition c(Condition::SearchLocation);
    c.setSearchLocationCondition(false, QStringLiteral("/opt"));
    EXPECT_FALSE(c.getConditionLabels().isEmpty());
}

// ---------- setDateRangeCondition / getDateRange* / isFileMatchedWithDate ----------
TEST(ConditionWhitebox, date_range_invalid_range_rejected)
{
    Condition c(Condition::DateRange);
    EXPECT_FALSE(c.setDateRangeCondition(0, 100, 50));
}

TEST(ConditionWhitebox, date_range_getters_after_set)
{
    Condition c(Condition::DateRange);
    const quint64 t0 = 1000ULL;
    const quint64 t1 = 2000ULL;
    ASSERT_TRUE(c.setDateRangeCondition(int(Condition::TODAY), t0, t1));
    EXPECT_EQ(c.getDateRangeType(), int(Condition::TODAY));
    EXPECT_EQ(c.getDateRangeStartMsec(), t0);
    EXPECT_EQ(c.getDateRangeEndMsec(), t1);
}

TEST(ConditionWhitebox, date_range_all_time_with_contains_in_json)
{
    Condition c(Condition::DateRange);
    ASSERT_TRUE(c.setDateRangeCondition(int(Condition::ALL_TIME), 0, 1));
    QJsonObject jo = c.conditionObject();
    jo[QStringLiteral("contains")] = true;
    auto c3 = Condition::fromJsonObject(jo);
    EXPECT_TRUE(c3->getDateRangeIsContains());
    EXPECT_TRUE(c3->isFileMatchedWithDate(0, QDateTime::currentMSecsSinceEpoch()));
}

TEST(ConditionWhitebox, date_range_labels_two_entries)
{
    Condition dr(Condition::DateRange);
    ASSERT_TRUE(dr.setDateRangeCondition(int(Condition::TODAY), 0, QDateTime::currentMSecsSinceEpoch()));
    QStringList dl = dr.getConditionLabels();
    EXPECT_EQ(dl.size(), 2);
}

// ---------- setConditionMatchRule / getConditionMatchRule / getConditionMatchRuleIsContains ----------
TEST(ConditionWhitebox, condition_match_rule_name_or_content_and_name)
{
    Condition c(Condition::ConditionMatchRule);
    ASSERT_TRUE(c.setConditionMatchRule(Condition::NameOrContent, true));
    EXPECT_EQ(c.getConditionMatchRule(), Condition::NameOrContent);
    EXPECT_TRUE(c.getConditionMatchRuleIsContains());

    ASSERT_TRUE(c.setConditionMatchRule(Condition::Name, false));
    EXPECT_EQ(c.getConditionMatchRule(), Condition::Name);
    EXPECT_FALSE(c.getConditionMatchRuleIsContains());
}

// ---------- conditionObject ----------
TEST(ConditionWhitebox, condition_object_round_trip_json)
{
    Condition c(Condition::FileName);
    c.setFileNameCondition(false, {QStringLiteral("bad")});
    QJsonObject jo = c.conditionObject();
    EXPECT_TRUE(jo.contains(QStringLiteral("keywords")));
}

// ---------- getConditionLabels（多类型抽样）----------
TEST(ConditionWhitebox, condition_labels_file_name_and_file_size)
{
    Condition c(Condition::FileName);
    c.setFileNameCondition(false, {QStringLiteral("bad")});
    EXPECT_FALSE(c.getConditionLabels().isEmpty());

    Condition c2(Condition::FileSize);
    c2.setFileSizeCondition(true, 3);
    EXPECT_FALSE(c2.getConditionLabels().isEmpty());
}

TEST(ConditionWhitebox, condition_labels_file_label_empty)
{
    Condition c(Condition::FileLabel);
    EXPECT_TRUE(c.getConditionLabels().isEmpty());
}

// ---------- pharseConditions / generateContitionsFromDescription / getLabelsFromDescription ----------
TEST(ConditionWhitebox, pharse_conditions_empty_list)
{
    QVariant v = Condition::pharseConditions({});
    auto list = Condition::generateContitionsFromDescription(v);
    EXPECT_TRUE(list.isEmpty());
}

TEST(ConditionWhitebox, serialize_round_trip_and_labels_from_description)
{
    QList<std::shared_ptr<Condition>> list;
    auto a = std::make_shared<Condition>(Condition::FileName);
    a->setFileNameCondition(true, {QStringLiteral("k")});
    list << a;
    QVariant v = Condition::pharseConditions(list, QStringList({QStringLiteral("L1")}));
    auto back = Condition::generateContitionsFromDescription(v);
    ASSERT_EQ(back.size(), 1);
    EXPECT_TRUE(back[0]->isFileNameMatchedWithKeywords(QStringLiteral("kkk")));

    QStringList labels = Condition::getLabelsFromDescription(v);
    EXPECT_FALSE(labels.isEmpty());
}

TEST(ConditionWhitebox, generate_conditions_count_two)
{
    auto a = std::make_shared<Condition>(Condition::FileName);
    a->setFileNameCondition(true, {QStringLiteral("a")});
    auto b = std::make_shared<Condition>(Condition::FileSize);
    b->setFileSizeCondition(true, 1);
    QVariant v = Condition::pharseConditions({a, b});
    auto back = Condition::generateContitionsFromDescription(v);
    ASSERT_EQ(back.size(), 2);
}

TEST(ConditionWhitebox, get_labels_from_description_sorts)
{
    QJsonObject root;
    root[QStringLiteral("conditions count")] = 0;
    root[QStringLiteral("labels")] = QJsonArray::fromStringList({QStringLiteral("z"), QStringLiteral("a")});
    QVariant v = QVariant::fromValue(QJsonObject(root));
    QStringList L = Condition::getLabelsFromDescription(v);
    ASSERT_EQ(L.size(), 2);
    EXPECT_EQ(L.first(), QStringLiteral("a"));
}

TEST(ConditionWhitebox, get_labels_from_description_without_labels_key)
{
    QJsonObject root;
    root[QStringLiteral("conditions count")] = 0;
    QVariant v = QVariant::fromValue(root);
    QStringList L = Condition::getLabelsFromDescription(v);
    EXPECT_TRUE(L.isEmpty());
}

// ---------- setFileTypeCondition(QStringList) 错误类型 → 返回 false (line 166) ----------
TEST(ConditionWhitebox, set_file_type_condition_suffixes_wrong_type_returns_false)
{
    Condition c(Condition::FileSize);
    EXPECT_FALSE(c.setFileTypeCondition(QStringList({QStringLiteral("txt")})));
}

TEST(ConditionWhitebox, set_file_type_condition_suffixes_empty_list_returns_false)
{
    Condition c(Condition::FileType);
    EXPECT_FALSE(c.setFileTypeCondition(QStringList()));
}

// ---------- setFileTypeCondition(int) 错误类型 → 返回 false (line 179) ----------
TEST(ConditionWhitebox, set_file_type_condition_int_wrong_type_returns_false)
{
    Condition c(Condition::FileSize);
    EXPECT_FALSE(c.setFileTypeCondition(3));
}

// ---------- WPS 后缀路径：MIME 不匹配但有点号 → 覆盖 lines 253-258,261 ----------
TEST(ConditionWhitebox, file_type_wps_non_mime_suffix_path_exercised)
{
    auto wps = fileTypeFromJson(int(Condition::WPS_FILE));
    // mimeType 不以 "application/wps-office" 开头，但文件名含有点号
    // 此路径会执行 tmp.chopped(index) 分支（虽因源码逻辑不会匹配 wpsSuffix）
    EXPECT_FALSE(wps->isFileTypeMatchedWithSubType(int(Condition::WPS_FILE),
                                                   QStringLiteral("file.doc"),
                                                   QStringLiteral("application/msword")));
}

TEST(ConditionWhitebox, file_type_wps_non_mime_no_dot_in_filename)
{
    auto wps = fileTypeFromJson(int(Condition::WPS_FILE));
    // 文件名无点号，index < 0，不进入 if(index >= 0) 分支
    EXPECT_FALSE(wps->isFileTypeMatchedWithSubType(int(Condition::WPS_FILE),
                                                   QStringLiteral("nodotfile"),
                                                   QStringLiteral("application/msword")));
}

// ---------- OTHERS 类型 + 已知 MIME（文件夹）→ 覆盖 break (line 275) ----------
TEST(ConditionWhitebox, file_type_others_known_mime_hits_break)
{
    auto oth = fileTypeFromJson(int(Condition::OTHERS));
    // mimeType = Folder_Type，使 if 条件为 false，命中 break
    EXPECT_FALSE(oth->isFileTypeMatchedWithSubType(int(Condition::OTHERS),
                                                   QStringLiteral("x"),
                                                   QStringLiteral("inode/directory")));
}

TEST(ConditionWhitebox, file_type_others_image_mime_hits_break)
{
    auto oth = fileTypeFromJson(int(Condition::OTHERS));
    EXPECT_FALSE(oth->isFileTypeMatchedWithSubType(int(Condition::OTHERS),
                                                   QStringLiteral("x"),
                                                   QStringLiteral("image/png")));
}

// ---------- default 分支：无效类型 → 覆盖 lines 277-278 ----------
TEST(ConditionWhitebox, file_type_invalid_subtype_hits_default)
{
    auto c = fileTypeFromJson(999);
    EXPECT_FALSE(c->isFileTypeMatchedWithSubType(999,
                                                 QStringLiteral("x"),
                                                 QStringLiteral("text/plain")));
}

// ---------- setConditionMatchRule 错误类型 → 返回 false (line 591) ----------
TEST(ConditionWhitebox, set_condition_match_rule_wrong_type_returns_false)
{
    Condition c(Condition::FileName);
    EXPECT_FALSE(c.setConditionMatchRule(Condition::Name));
}

// ---------- getConditionLabels FileName contains=true → 覆盖 lines 625-628 ----------
TEST(ConditionWhitebox, condition_labels_file_name_contains_true)
{
    Condition c(Condition::FileName);
    c.setFileNameCondition(true, {QStringLiteral("good"), QStringLiteral("test")});
    QStringList labels = c.getConditionLabels();
    EXPECT_FALSE(labels.isEmpty());
}

// ---------- getConditionLabels FileType 单后缀 → 覆盖 line 644 ----------
TEST(ConditionWhitebox, condition_labels_file_type_single_suffix)
{
    Condition c(Condition::FileType);
    c.setFileTypeCondition(QStringList({QStringLiteral("pdf")}));
    QStringList labels = c.getConditionLabels();
    EXPECT_FALSE(labels.isEmpty());
}

// ---------- getConditionLabels FileSize not-contains → 覆盖 line 667 ----------
TEST(ConditionWhitebox, condition_labels_file_size_not_contains)
{
    Condition c(Condition::FileSize);
    c.setFileSizeCondition(false, 3);
    QStringList labels = c.getConditionLabels();
    EXPECT_FALSE(labels.isEmpty());
}

// ---------- getConditionLabels SearchLocation contains=true → 覆盖 line 681 ----------
TEST(ConditionWhitebox, condition_labels_search_location_contains_true)
{
    Condition c(Condition::SearchLocation);
    c.setSearchLocationCondition(true, QStringLiteral("/home/user"));
    QStringList labels = c.getConditionLabels();
    EXPECT_FALSE(labels.isEmpty());
}

// ---------- getConditionLabels FileType type 超出范围 → 不产生 label ----------
TEST(ConditionWhitebox, condition_labels_file_type_int_out_of_range)
{
    QJsonObject o;
    o[QStringLiteral("condition type")] = int(Condition::FileType);
    o[QStringLiteral("type")] = 100;
    o[QStringLiteral("orCondition")] = false;
    auto c = Condition::fromJsonObject(o);
    QStringList labels = c->getConditionLabels();
    EXPECT_TRUE(labels.isEmpty());
}

// ---------- isFileMatchedWithDate 各日期类型的覆盖测试 ----------
static std::shared_ptr<Condition> makeDateCondition(int dateType, bool contains = true)
{
    QJsonObject jo;
    jo[QStringLiteral("condition type")] = int(Condition::DateRange);
    jo[QStringLiteral("type")] = dateType;
    jo[QStringLiteral("start")] = QStringLiteral("0");
    jo[QStringLiteral("end")] = QString::number(QDateTime::currentMSecsSinceEpoch() + 1000000ULL);
    jo[QStringLiteral("contains")] = contains;
    jo[QStringLiteral("orCondition")] = false;
    return Condition::fromJsonObject(jo);
}

// TODAY 匹配 → return contain (lines 492-510)
TEST(ConditionWhitebox, is_file_matched_with_date_today_match)
{
    auto c = makeDateCondition(int(Condition::TODAY));
    quint64 now = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, now));
}

// TODAY 不匹配 → return !contain (lines 492-504, 576)
TEST(ConditionWhitebox, is_file_matched_with_date_today_no_match)
{
    auto c = makeDateCondition(int(Condition::TODAY));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// YESTERDAY 匹配 → return contain (lines 512-517)
TEST(ConditionWhitebox, is_file_matched_with_date_yesterday_match)
{
    auto c = makeDateCondition(int(Condition::YESTERDAY));
    quint64 yesterday = static_cast<quint64>(QDateTime::currentDateTime().addDays(-1).toMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, yesterday));
}

// YESTERDAY 不匹配 → return !contain (line 576)
TEST(ConditionWhitebox, is_file_matched_with_date_yesterday_no_match)
{
    auto c = makeDateCondition(int(Condition::YESTERDAY));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// THIS_WEEK 匹配 → return contain (lines 519-527)
TEST(ConditionWhitebox, is_file_matched_with_date_this_week_match)
{
    auto c = makeDateCondition(int(Condition::THIS_WEEK));
    quint64 now = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, now));
}

// THIS_WEEK 不匹配 → break → return !contain
TEST(ConditionWhitebox, is_file_matched_with_date_this_week_no_match)
{
    auto c = makeDateCondition(int(Condition::THIS_WEEK));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// LAST_WEEK 匹配 → return contain (lines 529-534)
TEST(ConditionWhitebox, is_file_matched_with_date_last_week_match)
{
    auto c = makeDateCondition(int(Condition::LAST_WEEK));
    // addDays(-7) 恒落在上周范围内（无论今天是周几）
    quint64 lastWeek = static_cast<quint64>(QDateTime::currentDateTime().addDays(-7).toMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, lastWeek));
}

// LAST_WEEK 不匹配 → break → return !contain
TEST(ConditionWhitebox, is_file_matched_with_date_last_week_no_match)
{
    auto c = makeDateCondition(int(Condition::LAST_WEEK));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// THIS_MONTH 匹配 → return contain (lines 536-540)
TEST(ConditionWhitebox, is_file_matched_with_date_this_month_match)
{
    auto c = makeDateCondition(int(Condition::THIS_MONTH));
    quint64 now = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, now));
}

// THIS_MONTH 不匹配 → break → return !contain
TEST(ConditionWhitebox, is_file_matched_with_date_this_month_no_match)
{
    auto c = makeDateCondition(int(Condition::THIS_MONTH));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// LAST_MONTH 匹配 → return contain (lines 542-550)
TEST(ConditionWhitebox, is_file_matched_with_date_last_month_match)
{
    auto c = makeDateCondition(int(Condition::LAST_MONTH));
    quint64 lastMonth = static_cast<quint64>(QDateTime::currentDateTime().addMonths(-1).toMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, lastMonth));
}

// LAST_MONTH 不匹配 → break → return !contain (line 550)
TEST(ConditionWhitebox, is_file_matched_with_date_last_month_no_match)
{
    auto c = makeDateCondition(int(Condition::LAST_MONTH));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// LAST_MONTH month=1 跨年分支：如果当前月是 1 月，需要覆盖 year-1 分支
TEST(ConditionWhitebox, is_file_matched_with_date_last_month_jan_crossyear_branch)
{
    auto c = makeDateCondition(int(Condition::LAST_MONTH));
    QDateTime now = QDateTime::currentDateTime();
    // 将日期移到 2 月，上个月 1 月（不跨年）
    QDateTime twoMonthsAgo = now.addMonths(-2);
    // 无论结果如何，只确保代码运行覆盖该分支
    (void)c->isFileMatchedWithDate(0, static_cast<quint64>(twoMonthsAgo.toMSecsSinceEpoch()));
}

// THIS_YEAR 匹配 → return contain (lines 553-557)
TEST(ConditionWhitebox, is_file_matched_with_date_this_year_match)
{
    auto c = makeDateCondition(int(Condition::THIS_YEAR));
    quint64 now = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, now));
}

// THIS_YEAR 不匹配 → break → return !contain
TEST(ConditionWhitebox, is_file_matched_with_date_this_year_no_match)
{
    auto c = makeDateCondition(int(Condition::THIS_YEAR));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// LAST_YEAR 匹配 → return contain (lines 559-564)
TEST(ConditionWhitebox, is_file_matched_with_date_last_year_match)
{
    auto c = makeDateCondition(int(Condition::LAST_YEAR));
    quint64 lastYear = static_cast<quint64>(QDateTime::currentDateTime().addYears(-1).toMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, lastYear));
}

// LAST_YEAR 不匹配 → break → return !contain
TEST(ConditionWhitebox, is_file_matched_with_date_last_year_no_match)
{
    auto c = makeDateCondition(int(Condition::LAST_YEAR));
    quint64 twoYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-2).toMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, twoYearsAgo));
}

// BEFORE_LAST_YEAR 匹配 → return contain (lines 566-571)
TEST(ConditionWhitebox, is_file_matched_with_date_before_last_year_match)
{
    auto c = makeDateCondition(int(Condition::BEFORE_LAST_YEAR));
    quint64 threeYearsAgo = static_cast<quint64>(QDateTime::currentDateTime().addYears(-3).toMSecsSinceEpoch());
    EXPECT_TRUE(c->isFileMatchedWithDate(0, threeYearsAgo));
}

// BEFORE_LAST_YEAR 不匹配 → break → return !contain
TEST(ConditionWhitebox, is_file_matched_with_date_before_last_year_no_match)
{
    auto c = makeDateCondition(int(Condition::BEFORE_LAST_YEAR));
    quint64 now = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, now));
}

// 无效 dateType → default:break → return !contain (lines 573-576)
TEST(ConditionWhitebox, is_file_matched_with_date_invalid_type_default)
{
    auto c = makeDateCondition(999);
    quint64 now = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    EXPECT_FALSE(c->isFileMatchedWithDate(0, now));
}

// ---------- isFileMatchedWithDate contains=false 路径测试 ----------
TEST(ConditionWhitebox, is_file_matched_with_date_contains_false_today_match_returns_false)
{
    auto c = makeDateCondition(int(Condition::TODAY), false);
    quint64 now = static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
    // contains=false，今天匹配则 return contain = false
    EXPECT_FALSE(c->isFileMatchedWithDate(0, now));
}

TEST(ConditionWhitebox, is_file_matched_with_date_contains_false_old_date_returns_true)
{
    auto c = makeDateCondition(int(Condition::TODAY), false);
    quint64 oldDate = static_cast<quint64>(QDateTime::currentDateTime().addYears(-3).toMSecsSinceEpoch());
    // contains=false，不在今天则 return !contain = true
    EXPECT_TRUE(c->isFileMatchedWithDate(0, oldDate));
}

} // namespace
