#ifndef FILE_UTILS_TEST_H
#define FILE_UTILS_TEST_H

#include <QObject>

class FileUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testMakeDocUterm();
    void testIsOrUnder();
    void testEscapeHtmlAndBoldHtml();
    void testFindMultiToneWords();
    void testOcrSupportSize();
    void testSnippetFormattingBranches();
    void testSnippetBoundaryAndElideBranches();
    void testSnippetWithoutKeywordBranches();
    void testHtmlAndAdvanceBoundaryBranches();
    void testWrapDataAndClipboard();
    void testEncryptionAndFormatSupportBranches();
    void testAdditionalFormatAndOcrBranches();
    void testIconAndOpenFileFallbacks();
    void testOpenFileUsesProcessManagerWhenDefaultAppExists();
};

#endif // FILE_UTILS_TEST_H
