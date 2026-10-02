#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_PROMPT_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_PROMPT_H

#include <QLabel>
#include <QLabel>
#include <QEnterEvent>
#include <QEvent>
#include "kbubblewidget.h"

namespace fault_diagnosis {

class Prompt: public QLabel {
    Q_OBJECT
public:
    Prompt(QWidget *parent = nullptr);
    ~Prompt();
    void SetPromptContent(const QString &content);

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    kdk::KBubbleWidget *bubble_widget_;
    QLabel *content_label_;
    QWidget *fault_diagnosis_mainwindow_;
    QString content_;
};

}

#endif
