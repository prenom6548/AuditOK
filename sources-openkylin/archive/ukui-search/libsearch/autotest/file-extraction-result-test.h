#ifndef FILE_EXTRACTION_RESULT_TEST_H
#define FILE_EXTRACTION_RESULT_TEST_H

#include <QObject>

class FileExtractionResultTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAddAppendAndTypes();
    void testAssignmentCopiesStoredData();
};

#endif // FILE_EXTRACTION_RESULT_TEST_H
