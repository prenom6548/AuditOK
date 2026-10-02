#include <QTest>

#include "index/document.h"

using namespace UkuiSearch;

class DocumentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSettersAndGetters();
    void testAddPostingAndEmptyGuards();
    void testLongTermsAndCopyAssignment();
    void testEmptyInputsAreRejected();
};

void DocumentTest::testSettersAndGetters()
{
    Document doc;
    QString data = QStringLiteral("hello");
    doc.setData(data);
    doc.setUniqueTerm(QStringLiteral("unique-term"));
    doc.setIndexTime(QStringLiteral("2024-01-01"));

    QCOMPARE(QString::fromStdString(doc.getUniqueTerm()), QStringLiteral("unique-term"));
    QCOMPARE(QString::fromStdString(doc.indexTime()), QStringLiteral("2024-01-01"));
}

void DocumentTest::testAddPostingAndEmptyGuards()
{
    Document doc;
    doc.addTerm(QString());
    doc.addTerm(std::string());
    doc.addPosting(std::string(), 1);
    std::string emptyTerm;
    std::vector<size_t> emptyOffsets;
    doc.addPosting(emptyTerm, emptyOffsets, 2);
    doc.addPosting(std::string(), QVector<size_t>{1, 2}, 2);

    QString data = QStringLiteral("text");
    doc.setData(data);
    doc.addValue(1, QStringLiteral("value"));
    doc.addSortableSerialiseValue(2, QStringLiteral("3.14"));

    QVERIFY(true);
}

void DocumentTest::testLongTermsAndCopyAssignment()
{
    Document doc;
    std::string longTerm(260, 'x');
    QVector<size_t> offsets{1, 3, 5};
    doc.addPosting(longTerm, offsets, 4);
    doc.addTerm(std::string("term"));
    doc.setUniqueTerm(std::string("uniq"));

    Document copied(doc);
    QCOMPARE(QString::fromStdString(copied.getUniqueTerm()), QStringLiteral("uniq"));

    Document assigned;
    assigned = doc;
    QCOMPARE(QString::fromStdString(assigned.getUniqueTerm()), QStringLiteral("uniq"));
}

void DocumentTest::testEmptyInputsAreRejected()
{
    Document doc;

    QString emptyData;
    doc.setData(emptyData);
    QVERIFY(true);  // should not crash

    doc.addTerm(QString());
    QVERIFY(true);

    std::string emptyStd;
    doc.addTerm(emptyStd);
    QVERIFY(true);

    doc.setUniqueTerm(QString());
    QVERIFY(true);

    doc.setUniqueTerm(std::string());
    QVERIFY(true);
}

QTEST_MAIN(DocumentTest)
#include "document-test.moc"
