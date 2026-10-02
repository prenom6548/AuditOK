#include "keywordhighlighter.h"

KeywordHighlighter::KeywordHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // 定义高亮规则
    HighlightRule ruleWarn;
    ruleWarn.pattern = QRegularExpression("\\bwarn\\b");
    ruleWarn.format.setForeground(QColor(255, 165, 0)); // 橙色
    rules.append(ruleWarn);

    HighlightRule ruleWarning;
    ruleWarning.pattern = QRegularExpression("\\bwarning\\b");
    ruleWarning.format.setForeground(QColor(255, 165, 0)); // 橙色
    rules.append(ruleWarning);

    HighlightRule ruleError;
    ruleError.pattern = QRegularExpression("\\berror\\b");
    ruleError.format.setForeground(Qt::red); // 红色
    rules.append(ruleError);
}

void KeywordHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightRule &rule : qAsConst(rules))
    {
         QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
         while (it.hasNext())
         {
             QRegularExpressionMatch match = it.next();
             setFormat(match.capturedStart(), match.capturedLength(), rule.format);
         }
    }
}
