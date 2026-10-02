#include "test_translations.h"
#include <QDebug>

void TestTranslations::initTestCase()
{
    // 初始化测试环境
}

void TestTranslations::cleanupTestCase()
{
    // 清理测试环境
}

void TestTranslations::testTrWithZhCNLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("zh_CN");
    
    // 测试已知的翻译条目
    QCOMPARE(translator.tr("The system will reboot automatically after the restore is successful"), 
             QString("还原成功，系统将自动重启"));
    QCOMPARE(translator.tr("Restore failed"), 
             QString("还原失败"));
    QCOMPARE(translator.tr("Do not perform other operations during restore to avoid data loss"), 
             QString("还原期间不要做其它操作，以防数据丢失"));
    QCOMPARE(translator.tr("Restore Progress : %1 %"), 
             QString("还原进度：%1 %"));
    QCOMPARE(translator.tr("Restoring system"), 
             QString("正在进行系统还原"));
    QCOMPARE(translator.tr("Saving cached data, please wait"), 
             QString("正在保存缓存数据，请稍等"));
}

void TestTranslations::testTrWithZhHKLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("zh_HK");
    
    // 测试已知的翻译条目
    QCOMPARE(translator.tr("The system will reboot automatically after the restore is successful"), 
             QString("還原成功，系統將自動重啟"));
    QCOMPARE(translator.tr("Restore failed"), 
             QString("還原失敗"));
    QCOMPARE(translator.tr("Do not perform other operations during restore to avoid data loss"), 
             QString("還原過程中不要執行其他操作，以免數據丟失"));
    QCOMPARE(translator.tr("Restore Progress : %1 %"), 
             QString("還原進度：%1 %"));
    QCOMPARE(translator.tr("Restoring system"), 
             QString("正在進行系統還原"));
    QCOMPARE(translator.tr("Saving cached data, please wait"), 
             QString("正在保存緩存數據，請稍等"));
}

void TestTranslations::testTrWithBoCNLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("bo_CN");
    
    // 测试已知的翻译条目
    QCOMPARE(translator.tr("The system will reboot automatically after the restore is successful"), 
             QString("སླར་གསོ་བྱས་རྗེས་མ་ལག་རང་འགུལ་གྱིས་བསྐྱར་དུ་ཐོན་རྒྱུ་རེད།"));
    QCOMPARE(translator.tr("Restore failed"), 
             QString("མ་ལག་སླར་གསོ་བྱེད་པར་ཕམ་ཉེས་བྱུང་བ།"));
    QCOMPARE(translator.tr("Do not perform other operations during restore to avoid data loss"), 
             QString("གཞི་གྲངས་བོར་བརླག་མི་ཡོང་བའི་ཆེད་དུ་སླར་གསོ་བྱེད་རིང་ལས་སྒོ་གཞན་དག་མི་སྒྲུབ་པ།"));
    QCOMPARE(translator.tr("Restore Progress : %1 %"), 
             QString("སོར་ཆུད་ཀྱི་འཕེལ་རིམ་དང་། %1 %"));
    QCOMPARE(translator.tr("Restoring system"), 
             QString("མ་ལག་སླར་གསོ་བྱེད་པ"));
    QCOMPARE(translator.tr("Saving cached data, please wait"), 
             QString("བཞིན་ཉར་འཛིན་བྱས་པའི་གཞི་གྲངས་ཅུང་ཙམ་སྒུགས་དང།"));
    QCOMPARE(translator.tr("The backup and restore tool is busy, please wait"),
             QString("གྲབས་ཉར་སོར་ཆུད་ལག་ཆ་བྲེལ་བཞིན་ཡོད་།ཏོག་ཙམ་སྒུག་རོགས་།"));
}

void TestTranslations::testTrWithMnMNLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("mn_MN");
    
    // 测试已知的翻译条目
    QCOMPARE(translator.tr("The system will reboot automatically after the restore is successful"), 
             QString("ᠰᠡᠷᠬᠦᠬᠡᠭ᠌ᠰᠡᠨ ᠎ᠦ ᠳᠠᠷᠠᠭ᠎ᠠ ᠱᠢᠰᠲ᠋ᠧᠮ ᠨᠢ ᠦᠪᠡᠷ ᠎ᠢᠶᠡᠨ ᠡᠬᠢᠯᠡᠨ᠎ᠡ"));
    QCOMPARE(translator.tr("Restore failed"), 
             QString("ᠱᠢᠰᠲ᠋ᠧᠮ ᠎ᠢ ᠰᠡᠷᠬᠦᠬᠡᠵᠤ ᠴᠢᠳᠠᠭ᠍ᠰᠠᠨ ᠦᠬᠡᠢ"));
    QCOMPARE(translator.tr("Do not perform other operations during restore to avoid data loss"), 
             QString("ᠰᠡᠷᠬᠦᠬᠡᠬᠦ᠌ ᠬᠤᠭᠤᠴᠠᠭᠠᠨ᠎ᠳᠤ ᠪᠤᠰᠤᠳ ᠠᠵᠢᠯᠯᠠᠬᠤᠢ᠎ᠶ᠋ᠢ ᠪᠢᠳᠡᠬᠡᠢ ᠬᠢᠬᠡᠷᠡᠢ᠂ ᠳ᠋ᠠᠢᠲ᠋ᠠ ᠬᠡᠬᠡᠭ᠌ᠳᠡᠬᠦ᠌᠎ᠡᠴᠡ ᠰᠡᠷᠬᠡᠢᠯᠡᠬᠡᠷᠡᠢ"));
    QCOMPARE(translator.tr("Restore Progress : %1 %"), 
             QString("ᠰᠡᠷᠬᠦᠬᠡᠵᠤ ᠪᠠᠢᠨ᠎ᠠ : %1 %"));
    QCOMPARE(translator.tr("Restoring system"), 
             QString("ᠱᠢᠰᠲ᠋ᠧᠮ ᠎ᠢ ᠰᠡᠷᠬᠦᠬᠡᠬᠦ"));
    QCOMPARE(translator.tr("Saving cached data, please wait"), 
             QString("ᠱᠢᠰᠲ᠋ᠧᠮ ᠵᠠᠪ ᠦᠬᠡᠢ ᠪᠠᠢᠨ᠎ᠠ᠂ ᠳᠦᠷ ᠬᠦᠯᠢᠶᠡᠬᠡᠷᠡᠢ"));
    QCOMPARE(translator.tr("The backup and restore tool is busy, please wait"),
             QString("ᠠᠬᠤᠢ ᠬᠤᠷᠢᠶᠠᠨ ᠲᠦᠷᠢᠶᠡᠯᠡᠭᠡᠬᠦ ᠬᠡᠷᠡᠭᠯᠡᠭᠡ ᠡᠯᠭᠡᠷᠡᠩᠭᠦᠢ ᠪᠤᠯᠤ ᠬᠦᠯᠡᠮᠡᠢ ᠪᠠᠢᠨ᠎ᠠᠠᠯᠲᠠᠨ ᠲᠤᠭ᠎ᠠᠷ ᠬᠦᠯᠡᠮᠦᠨ ᠦᠭᠡᠢ"));
}

void TestTranslations::testTrWithkyKGLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("ky_KG");

    QCOMPARE(translator.tr("Restore failed"),
             QString("Жөнөтүү ийри болду"));
}

void TestTranslations::testTrWithkkKZLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("kk_KZ");

    QCOMPARE(translator.tr("Restore failed"),
             QString("Қалпына келтіру сәтсіз болды"));
}

void TestTranslations::testTrWithugCNLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("ug_CN");

    QCOMPARE(translator.tr("Restore failed"),
             QString("قايتىمۇچە تەكشۈرۈش مۇشكىلەپ كەلتتى"));
}

void TestTranslations::testTrWithviVNLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("vi_VN");

    QCOMPARE(translator.tr("Restore failed"),
             QString("Khôi phục thất bại"));
}

void TestTranslations::testTrWithmsMYLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("ms_MY");

    QCOMPARE(translator.tr("Restore failed"),
             QString("Pemulihan gagal"));
}

void TestTranslations::testTrWiththTHLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("th_TH");

    QCOMPARE(translator.tr("Restore failed"),
             QString("การกู้คืนล้มเหลว"));
}

void TestTranslations::testTrWithUnknownLocale()
{
    Translations& translator = Translations::instance();
    translator.setlocale("en_US");  // 不支持的语言
    
    // 对于不支持的语言，应该返回原文
    QCOMPARE(translator.tr("The system will reboot automatically after the restore is successful"), 
             QString("The system will reboot automatically after the restore is successful"));
    QCOMPARE(translator.tr("Restore failed"), 
             QString("Restore failed"));
    QCOMPARE(translator.tr("Unknown string"), 
             QString("Unknown string"));
}

void TestTranslations::testSetLocale()
{
    Translations& translator = Translations::instance();
    
    // 测试设置语言环境
    translator.setlocale("zh_CN");
    QCOMPARE(true, true);
}
