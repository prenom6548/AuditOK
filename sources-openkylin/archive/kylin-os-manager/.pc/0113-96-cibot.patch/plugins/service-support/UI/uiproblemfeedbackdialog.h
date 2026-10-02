#ifndef UIPROBLEMFEEDBACKDIALOG_H
#define UIPROBLEMFEEDBACKDIALOG_H

#include <QLabel>
#include <kprogressbar.h>
#include <QPushButton>
#include <kdialog.h>
#include "feedbackmanager.h"
#include "service-support-backend/src/settings.h"

class UiProblemFeedbackDialog : public kdk::KDialog
{
    Q_OBJECT
public:
    UiProblemFeedbackDialog(QWidget *parent);

public slots:
    void showDialog(FeedBackFinishType type, QString str);
    void showProgress(int i);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void showDialogPri();

    kdk::KProgressBar *m_progressBar = nullptr;
    QLabel *m_titleText = nullptr;
    QLabel *m_secondText = nullptr;
    QLabel *m_icon = nullptr;
    QPushButton *m_retryBtn = nullptr;
    QPushButton *m_OKBtn = nullptr;
    QPushButton *m_CancelBtn = nullptr;
    QWidget *m_parent = nullptr;
    bool m_isChanged = false;
    enum Settings::Customize m_environment;
};

#endif // UIPROBLEMFEEDBACKDIALOG_H
