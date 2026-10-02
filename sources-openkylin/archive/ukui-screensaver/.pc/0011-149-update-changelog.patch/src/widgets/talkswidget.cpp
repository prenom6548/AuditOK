#include "talkswidget.h"
#include <QLineEdit>
#include <QHBoxLayout>
#include <QDebug>
#include "customlabel.h"
#include "iconbutton.h"

#define LABEL_MINIMUM_HEIGHT    12
#define LABEL_MAXIMUM_HEIGHT    30
#define WIDGET_FIXED_WIDTH      240
#define WIDGET_HEIGHT_SPACING   10

TalksWidget::TalksWidget(LockDialogModel *model, QWidget *parent, int editType)
    : QWidget(parent),
      m_modelLockDialog(model)
{
    m_editType = editType;
    initUI();
}

void TalksWidget::initUI()
{
//    setFixedWidth(WIDGET_FIXED_WIDTH);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 12, 0, 0);
    mainLayout->setSpacing(0);

    QVBoxLayout *quesTionLayout = new QVBoxLayout();
    quesTionLayout->setAlignment(Qt::AlignHCenter);
    quesTionLayout->setContentsMargins(0, 0, 0, 0);
    m_pQuestionLabel = new CustomLabel(this);

    m_pQuestionLabel->setFontSize(14);
    quesTionLayout->addWidget(m_pQuestionLabel);
    mainLayout->addLayout(quesTionLayout);
    mainLayout->addSpacing(4);

    QVBoxLayout *answerLayout = new QVBoxLayout();
    answerLayout->setAlignment(Qt::AlignHCenter);
    answerLayout->setContentsMargins(0, 0, 0, 0);
    m_pAnswerEdit = new IconEdit(m_modelLockDialog, this, m_editType);
    m_pAnswerEdit->setFixedSize(240, 40);
    m_pAnswerEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::white);
    m_pAnswerEdit->setPalette(pal);
    connect(m_pAnswerEdit, &IconEdit::textChanged, this, [=](const QString &text){
        Q_EMIT textChanged(text);
    });
    connect(m_pAnswerEdit, &IconEdit::clickedPassword, this, [=]{
        Q_EMIT talkEditClicked();
    });
    mainLayout->addWidget(m_pAnswerEdit);
    answerLayout->addWidget(m_pAnswerEdit);
    mainLayout->addLayout(answerLayout);
    mainLayout->addSpacing(2);
    setPrompt(tr("Enter your answer"));
    setFocusProxy(m_pAnswerEdit);
}

void TalksWidget::setPrompt(QString prompt)
{
    m_pAnswerEdit->setPrompt(prompt);
}

QString TalksWidget::getEditText()
{
    return m_pAnswerEdit->text();
}

void TalksWidget::setEchoMode(QLineEdit::EchoMode type)
{
    m_pAnswerEdit->setType(type);
    if (type == QLineEdit::Password) {
        m_pAnswerEdit->setAttribute(Qt::WA_InputMethodEnabled, false); //禁用输入法
    }
}

void TalksWidget::setQuestion(QString question)
{
    m_pQuestionLabel->setText(question);
}

void TalksWidget::clearText()
{
    m_pAnswerEdit->clearText();
}

int TalksWidget::getTalksWdgHeight()
{
    return m_pAnswerEdit->height() + m_pQuestionLabel->height() + 18;
}

void TalksWidget::updateFontSize(double lfFontSize)
{
}
