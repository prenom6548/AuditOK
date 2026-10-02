#include "user_experience_dialog.h"
#include <QVariant>

namespace K {

UserExperienceDialog::UserExperienceDialog(QWidget *parent)
    : QMessageBox(parent),
      feedback_btn_(new QPushButton(this)),
      fixed_btn_(new QPushButton(this))
{
    setText(tr("Please confirm if your issue has been fixed?"));
    setIcon(QMessageBox::Icon::Question);

    feedback_btn_->resize(96, 36);
    feedback_btn_->setText(tr("Feedback"));
    connect(feedback_btn_, &QPushButton::clicked, this, &UserExperienceDialog::on_Feedback);

    fixed_btn_->resize(96, 36);
    fixed_btn_->setText(tr("Fixed"));
    fixed_btn_->setProperty("isImportant", true);
    connect(fixed_btn_, &QPushButton::clicked, this, &UserExperienceDialog::on_Fixed);

    addButton(feedback_btn_, QMessageBox::ButtonRole::NoRole);
    addButton(fixed_btn_, QMessageBox::ButtonRole::YesRole);
}

void UserExperienceDialog::on_Feedback() const {
    if (feedback_callback_) {
        feedback_callback_();
    }
}

void UserExperienceDialog::on_Fixed() {
    accept();

    if (fixed_callback_) {
        fixed_callback_();
    }
}

void UserExperienceDialog::SetFeedBackCallback(FeedBackCallback &&callback) {
    feedback_callback_ = callback;
}

void UserExperienceDialog::SetFixedCallback(FixedCallback &&callback) {
    fixed_callback_ = callback;
}

}
