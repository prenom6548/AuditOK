#ifndef KEYWORDHIGHLIGHTER_H
#define KEYWORDHIGHLIGHTER_H
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

class KeywordHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit KeywordHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightRule> rules;
};
#endif // KEYWORDHIGHLIGHTER_H
