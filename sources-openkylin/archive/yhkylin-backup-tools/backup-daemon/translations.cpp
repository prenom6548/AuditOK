/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "translations.h"
#include <QMap>
#include <QDebug>

Translations::Translations(token)
{}

Translations::~Translations()
{}

QString Translations::tr(const QString& src)
{
    if (m_locale == "zh_CN") {
        return zh_CN(src);
    } else if (m_locale == "zh_HK") {
        return zh_HK(src);
    } else if (m_locale == "bo_CN") {
        return bo_CN(src);
    } else if (m_locale == "mn_MN") {
        return mn_MN(src);
    } else if (m_locale == "ky_KG") {
        return ky(src);
    } else if (m_locale == "kk_KZ") {
        return kk(src);
    } else if (m_locale == "ug_CN") {
        return ug(src);
    }

    return src;
}

QString Translations::zh_CN(const QString& src)
{
    static QMap<QString, QString> en_2_zh_CN {
        {"The system will reboot automatically after the restore is successful", "还原成功，系统将自动重启"},
        {"Restore failed", "还原失败"},
        {"Do not perform other operations during restore to avoid data loss", "还原期间不要做其它操作，以防数据丢失"},
        {"Restore Progress : %1 %", "还原进度：%1 %"},
        {"Restoring system", "正在进行系统还原"},
        {"Saving cached data, please wait", "正在保存缓存数据，请稍等"}
    };

    return en_2_zh_CN.value(src, src);
}

QString Translations::zh_HK(const QString& src)
{
    static QMap<QString, QString> en_2_zh_HK {
        {"The system will reboot automatically after the restore is successful", "還原成功，系統將自動重啟"},
        {"Restore failed", "還原失敗"},
        {"Do not perform other operations during restore to avoid data loss", "還原過程中不要執行其他操作，以免數據丟失"},
        {"Restore Progress : %1 %", "還原進度：%1 %"},
        {"Restoring system", "正在進行系統還原"},
        {"Saving cached data, please wait", "正在保存緩存數據，請稍等"}
    };

    return en_2_zh_HK.value(src, src);
}

QString Translations::bo_CN(const QString& src)
{
    static QMap<QString, QString> en_2_bo_CN {
        {"The system will reboot automatically after the restore is successful", "སླར་གསོ་བྱས་རྗེས་མ་ལག་རང་འགུལ་གྱིས་བསྐྱར་དུ་ཐོན་རྒྱུ་རེད།"},
        {"Restore failed", "མ་ལག་སླར་གསོ་བྱེད་པར་ཕམ་ཉེས་བྱུང་བ།"},
        {"Do not perform other operations during restore to avoid data loss", "གཞི་གྲངས་བོར་བརླག་མི་ཡོང་བའི་ཆེད་དུ་སླར་གསོ་བྱེད་རིང་ལས་སྒོ་གཞན་དག་མི་སྒྲུབ་པ།"},
        {"Restore Progress : %1 %", "སོར་ཆུད་ཀྱི་འཕེལ་རིམ་དང་། %1 %"},
        {"Restoring system", "མ་ལག་སླར་གསོ་བྱེད་པ"},
        {"Saving cached data, please wait", "བཞིན་ཉར་འཛིན་བྱས་པའི་གཞི་གྲངས་ཅུང་ཙམ་སྒུགས་དང།"}
    };

    return en_2_bo_CN.value(src, src);
}

QString Translations::mn_MN(const QString& src)
{
    static QMap<QString, QString> en_2_mn_MN {
        {"The system will reboot automatically after the restore is successful", "ᠰᠡᠷᠬᠦᠬᠡᠭ᠌ᠰᠡᠨ ᠎ᠦ ᠳᠠᠷᠠᠭ᠎ᠠ ᠱᠢᠰᠲ᠋ᠧᠮ ᠨᠢ ᠦᠪᠡᠷ ᠎ᠢᠶᠡᠨ ᠡᠬᠢᠯᠡᠨ᠎ᠡ"},
        {"Restore failed", "ᠱᠢᠰᠲ᠋ᠧᠮ ᠎ᠢ ᠰᠡᠷᠬᠦᠬᠡᠵᠤ ᠴᠢᠳᠠᠭ᠍ᠰᠠᠨ ᠦᠬᠡᠢ"},
        {"Do not perform other operations during restore to avoid data loss", "ᠰᠡᠷᠬᠦᠬᠡᠬᠦ᠌ ᠬᠤᠭᠤᠴᠠᠭᠠᠨ᠎ᠳᠤ ᠪᠤᠰᠤᠳ ᠠᠵᠢᠯᠯᠠᠬᠤᠢ᠎ᠶ᠋ᠢ ᠪᠢᠳᠡᠬᠡᠢ ᠬᠢᠬᠡᠷᠡᠢ᠂ ᠳ᠋ᠠᠢᠲ᠋ᠠ ᠬᠡᠬᠡᠭ᠌ᠳᠡᠬᠦ᠌᠎ᠡᠴᠡ ᠰᠡᠷᠬᠡᠢᠯᠡᠬᠡᠷᠡᠢ"},
        {"Restore Progress : %1 %", "ᠰᠡᠷᠬᠦᠬᠡᠵᠤ ᠪᠠᠢᠨ᠎ᠠ : %1 %"},
        {"Restoring system", "ᠱᠢᠰᠲ᠋ᠧᠮ ᠎ᠢ ᠰᠡᠷᠬᠦᠬᠡᠬᠦ"},
        {"Saving cached data, please wait", "ᠱᠢᠰᠲ᠋ᠧᠮ ᠵᠠᠪ ᠦᠬᠡᠢ ᠪᠠᠢᠨ᠎ᠠ᠂ ᠳᠦᠷ ᠬᠦᠯᠢᠶᠡᠬᠡᠷᠡᠢ"}
    };

    return en_2_mn_MN.value(src, src);
}

QString Translations::ug(const QString& src)
{
    static QMap<QString, QString> en_2_ug {
        {"The system will reboot automatically after the restore is successful", "تېزىملىك قايتىمۇچە تەكشۈرۈلگەندىن كېيىن ئۆزى ئۆزۈنۈكىمۈكى قايتا ئېچىلەدۇ"},
        {"Restore failed", "قايتىمۇچە تەكشۈرۈش مۇشكىلەپ كەلتتى"},
        {"Do not perform other operations during restore to avoid data loss", "سانلار يۈچۈرۈشىنى يوقىتىڭقايتىمۇچە تەكشۈرۈش جەرەنەسىدە باشقا ئىشلەشلەرنى ئەمەلغا ئاشۇرماڭ، بۇنىڭ ئارقىلىق "},
        {"Restore Progress : %1 %", "قايتىمۇچە تەكشۈرۈش ئالىمى : %1 %"},
        {"Restoring system", "تېزىملىك قايتىمۇچە تەكشۈرۈلۈۋاتىدۇ"},
        {"Saving cached data, please wait", "كېش سانلارنى ساقلىۋاتىدۇ، ئەڭگىزىڭلار"}
    };

    return en_2_ug.value(src, src);
}

QString Translations::kk(const QString& src)
{
    static QMap<QString, QString> en_2_kk {
        {"The system will reboot automatically after the restore is successful", "Система қалпына келтіру сәтті аяқталғаннан кейін өзектік түрде қайта қосу болады"},
        {"Restore failed", "Қалпына келтіру сәтсіз болды"},
        {"Do not perform other operations during restore to avoid data loss", "Рекламалық кезеңде басқа операцияларды орындамаңыз, өйткені деректер жоғалуы мүмкін"},
        {"Restore Progress : %1 %", "Қайтаруový ийри-шырыны: %1 %"},
        {"Restoring system", "Системаны қайтаруда"},
        {"Saving cached data, please wait", "Кэшделген деректерді сақтауда, лطفен қانتалуыңыз"}
    };

    return en_2_kk.value(src, src);
}

QString Translations::ky(const QString& src)
{
    static QMap<QString, QString> en_2_ky {
        {"The system will reboot automatically after the restore is successful", "Жөнөтүү ийри-шырыны ийри болсоңуздан кийин, система автоматтык түрдө кайра ачылат"},
        {"Restore failed", "Жөнөтүү ийри болду"},
        {"Do not perform other operations during restore to avoid data loss", "Жөнөтүү кезеңінде басқа операцияларды орындамаңыз, деректер жоғалуын болдырмау үшін"},
        {"Restore Progress : %1 %", "Жөнөтүү ийри-шырыны: %1 %"},
        {"Restoring system", "Системаны жөнөтүп жатат"},
        {"Saving cached data, please wait", "Кэштенген деректерди сактап жатат, жөнөкөйлөп күтөңüz"}
    };

    return en_2_ky.value(src, src);
}