#ifndef TALKSWIDGET_H
#define TALKSWIDGET_H

#include <QWidget>
#include "iconedit.h"

class KLabel;
class QLineEdit;

class TalksWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TalksWidget(LockDialogModel *model, QWidget *parent = nullptr, int editType = CONFIDENTIALEDIT);
    void setQuestion(QString question);
    QString getEditText();
    void setPrompt(QString prompt);
    void clearText();
    int getTalksWdgHeight();
    void setEchoMode(QLineEdit::EchoMode type);
    void updateFontSize(double lfFontSize);

Q_SIGNALS:
    void textChanged(const QString &text);
    void talkEditClicked();

private:
    void initUI();

private:
    LockDialogModel *m_modelLockDialog = nullptr;
    KLabel          *m_pQuestionLabel = nullptr;
    IconEdit        *m_pAnswerEdit = nullptr;
    int             m_editType;
};

#endif // TALKSWIDGET_H
