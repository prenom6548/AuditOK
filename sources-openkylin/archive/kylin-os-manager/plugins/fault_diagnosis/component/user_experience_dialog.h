#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_COMPONENT_USER_EXPERIENCE_DIALOG_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_COMPONENT_USER_EXPERIENCE_DIALOG_H

#include <functional>
#include <QPushButton>
#include <QDialog>

namespace K {

class UserExperienceDialog: public QDialog {
    Q_OBJECT
public:
    using FeedBackCallback = std::function<void()>;
    using FixedCallback = std::function<void()>;

    UserExperienceDialog(QWidget *parent = nullptr);
    void SetFeedBackCallback(FeedBackCallback &&callback);
    void SetFixedCallback(FixedCallback &&callback);

private Q_SLOTS:
    void on_Feedback();
    void on_Fixed();

private:
    QPushButton *       feedback_btn_;
    QPushButton *       fixed_btn_;
    FeedBackCallback    feedback_callback_;
    FixedCallback       fixed_callback_;
};

}

#endif
