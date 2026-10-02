#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_PROMPT_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_PROMPT_H

#include <QLabel>

namespace fault_diagnosis {

class Prompt : public QLabel {
    Q_OBJECT
public:
    Prompt(QWidget *parent = nullptr);
    ~Prompt();
    void SetPromptContent(QString content);

private:
    QString content_;
};

}

#endif
