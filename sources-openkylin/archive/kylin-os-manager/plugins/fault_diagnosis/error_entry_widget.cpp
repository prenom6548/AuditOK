#include <fstream>
#include <sstream>
#include <iomanip>
#include <QHBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include "error_entry_widget.h"

namespace fault_diagnosis {

static const char *TEMP_SAVE_DIR= "/tmp/kylin-os-manager-fault-diagnosis/";

ErrorEntryWidget::ErrorEntryWidget(const DiagnosticError &diagnostic_error, const Database::SolutionRecord &solution_record, QWidget *parent)
    : QWidget(parent),
      diagnostic_error_(diagnostic_error),
      solution_record_(solution_record),
      solution_category_(-1),
      select_checkbox_(new QCheckBox(this)),
      name_label_(new K::TextLabel(this)),
      describe_label_(new K::TextLabel(this)),
      manual_repair_btn_(new kdk::KBorderlessButton(this)),
      status_label_(new K::TextLabel(this))
{
    setMinimumHeight(36);
    setFixedWidth(820);

    name_label_->SetText(diagnostic_error.name);
    describe_label_->SetFontColor(Qt::red);
    SetDescribe(diagnostic_error.describe);
    manual_repair_btn_->setText(tr("Manual repair"));
    manual_repair_btn_->hide();
    SetStatus(ErrorEntryWidget::Status::kException);
    connect(select_checkbox_, &QCheckBox::stateChanged, this, &ErrorEntryWidget::sig_SelectChange);
    connect(manual_repair_btn_, &kdk::KBorderlessButton::clicked, this, &ErrorEntryWidget::on_ManualRepair);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(96, 0, 68, 0);
    hbox_layout->addWidget(select_checkbox_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(name_label_);
    hbox_layout->addSpacing(32);
    hbox_layout->addWidget(describe_label_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(manual_repair_btn_);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(status_label_);
    setLayout(hbox_layout);

    bool ok;
    qulonglong solution_type = solution_record.solution_type.toULongLong(&ok, 16);
    if (ok) {
        if (solution_type >= 0xE000000000000000) {
            // 手动修复

            // 共 64 位，最高 8 位为分类
            solution_category_ = (solution_type >> 56) & 0xFF;
            select_checkbox_->setCheckState(Qt::CheckState::Unchecked);
            select_checkbox_->setEnabled(false);
            manual_repair_btn_->show();
        } else {
            select_checkbox_->setEnabled(true);
            select_checkbox_->setCheckState(Qt::CheckState::Checked);
            manual_repair_btn_->hide();
        }
    } else {
        qCritical() << "Solution type QString to int fail.";
    }
}

ErrorEntryWidget::~ErrorEntryWidget() {

}

qint64 ErrorEntryWidget::ErrorCode() const {
    return diagnostic_error_.error_code;
}

bool ErrorEntryWidget::Selected() {
   return (select_checkbox_->isEnabled() && select_checkbox_->checkState() == Qt::CheckState::Checked);
}

bool ErrorEntryWidget::ManualRepair() {
    return !manual_repair_btn_->isHidden();
}

bool ErrorEntryWidget::SetCheckState(Qt::CheckState state) {
    switch (state) {
    case Qt::CheckState::PartiallyChecked:
        [[fallthrough]];
    case Qt::CheckState::Checked:
        if (select_checkbox_->isEnabled()) {
            select_checkbox_->blockSignals(true);
            select_checkbox_->setCheckState(Qt::CheckState::Checked);
            select_checkbox_->blockSignals(false);
            return true;
        }
        return false;
    case Qt::CheckState::Unchecked:
        if (select_checkbox_->isEnabled()) {
            select_checkbox_->blockSignals(true);
            select_checkbox_->setCheckState(Qt::CheckState::Unchecked);
            select_checkbox_->blockSignals(false);
        }
        return true;
    }
    return true;
}

const Database::SolutionRecord &ErrorEntryWidget::SolutionRecord() const {
    return solution_record_;
}

void ErrorEntryWidget::SetStatus(Status status) {
    switch (status) {
    case ErrorEntryWidget::Status::kException:
        status_ = ErrorEntryWidget::Status::kException;
        status_label_->SetFontColor(Qt::red);
        status_label_->SetText(tr("Exception"));
        break;
    case ErrorEntryWidget::Status::kRepairing:
        status_ = ErrorEntryWidget::Status::kRepairing;
        status_label_->SetFontColor(Qt::white);
        status_label_->SetText(tr("Repairing..."));
        break;
    case ErrorEntryWidget::Status::kFixed:
        status_ = ErrorEntryWidget::Status::kFixed;
        status_label_->SetFontColor(Qt::green);
        status_label_->SetText(tr("Fixed"));
        break;
    default:
        break;
    }
}

ErrorEntryWidget::Status ErrorEntryWidget::GetStatus() const {
    return status_;
}

void ErrorEntryWidget::SetCheckEnable(bool enable) {
    select_checkbox_->setEnabled(enable);
}

void ErrorEntryWidget::SetDescribe(const QString &describe) {
    describe_label_->SetText(describe);
}

void ErrorEntryWidget::on_ManualRepair() const {
    if (solution_category_ == -1) {
        qCritical() << "Wrong solution type: " << solution_category_;
        return;
    }

    // 创建用于存储临时生成的 html 的目录
    QDir dir(TEMP_SAVE_DIR);
    if (!dir.exists()) {
        if (!dir.mkdir(TEMP_SAVE_DIR)) {
            ShowError();
            qCritical() << "create temp save dir fail.";
            return;
        }
    }

    /*
     * 0xE000000000000000 - 0xE0FFFFFFFFFFFFFF 提示文本
     * 0xE100000000000000 - 0xE1FFFFFFFFFFFFFF 显示连接
     * 0xE200000000000000 - 0xE2FFFFFFFFFFFFFF 显示图片
     */
    switch (solution_category_) {
        case 0xE0: {
            // 提示文本
            auto path = CreateTextHtml();
            if (path.isEmpty()) {
                ShowError();
                return;
            }
            QProcess::startDetached("gio", {"open", path});
            break;
        }
        case 0xE1: {
            // 显示链接
            QProcess::startDetached("gio", {"open", solution_record_.link});
            break;
        }
        case 0xE2: {
            // 显示图片
            auto path = CreateImageHtml();
            if (path.isEmpty()) {
                ShowError();
                return;
            }
            QProcess::startDetached("gio", {"open", path});
            break;
        }
        default: {
            QMessageBox box(QMessageBox::Icon::Information, "", tr("We currently do not support fixing this issue. Stay tuned!"));
            box.exec();
            qInfo() << "Unsupported solution type: " << solution_category_;
        }
    }
}

void ErrorEntryWidget::ShowError() const {
    QMessageBox box(QMessageBox::Icon::Critical, "", tr("Fail to create solution."));
    box.exec();
}

QString ErrorEntryWidget::CreateTextHtml() const {
    std::string path{TEMP_SAVE_DIR};
    path.append(solution_record_.error_code.toStdString());
    path.append(".html");

    std::ofstream f(path, std::ios::trunc | std::ios::out);
    if (!f.is_open()) {
        qCritical() << "Create text html open file fail.";
        return "";
    }

    f << "<!DOCTYPE html>"                                          << "\n";
    f << "<html>"                                                   << "\n";
    f << "<head>"                                                   << "\n";
    f << "    <meta charset=\"UTF-8\">"                             << "\n";
    f << "    <title>手动修复方案</title>"                            << "\n";
    f << "</head>"                                                  << "\n";
    f << "<body>"                                                   << "\n";
    f << "    <p>" << solution_record_.text.toStdString() << "</p>" << "\n";
    f << "</body>"                                                  << "\n";
    f << "</html>";

    f.flush();
    f.close();
    return QString::fromStdString(path);
}

QString ErrorEntryWidget::CreateImageHtml() const {
    std::string path{TEMP_SAVE_DIR};
    path.append(solution_record_.error_code.toStdString());
    path.append(".html");

    std::ofstream f(path, std::ios::trunc | std::ios::out);
    if (!f.is_open()) {
        qCritical() << "Create image html open file fail. (" << solution_record_.error_code << ")";
        return "";
    }

    f << "<!DOCTYPE html>"                                                       << "\n";
    f << "<html>"                                                                << "\n";
    f << "<head>"                                                                << "\n";
    f << "    <meta charset=\"UTF-8\">"                                          << "\n";
    f << "    <title>手动修复方案</title>"                                         << "\n";
    f << "</head>"                                                               << "\n";
    f << "<body>"                                                                << "\n";
    f << "    <img src=\"" << solution_record_.image_list.toStdString() << "\">" << "\n";
    f << "</body>"                                                               << "\n";
    f << "</html>";

    f.flush();
    f.close();
    return QString::fromStdString(path);
}

void ErrorEntryWidget::SwitchToManualRepair(qint64 solution_type, const QString &solution_context) {
    auto solution_category = (solution_type >> 56) & 0xFF;
    switch (solution_category) {
        case 0xE0: {
            // 提示文本
            solution_record_.text = solution_context;
            break;
        }
        case 0xE1: {
            // 显示连接
            solution_record_.link = solution_context;
            break;
        }
        case 0xE2: {
            // 显示图片
            solution_record_.image_list = solution_context;
            break;
        }
        default: {
            qCritical() << "Swtich to manual repair fail, solution type not supported.";
            return;
        }
    }
    solution_category_ = solution_category;

    std::stringstream ss;
    ss << "0x" << std::hex << std::setfill('0') << std::setw(16) << solution_type;
    solution_record_.solution_type = QString::fromStdString(ss.str());

    select_checkbox_->setCheckState(Qt::CheckState::Unchecked);
    select_checkbox_->setEnabled(false);
    manual_repair_btn_->show();
    SetStatus(ErrorEntryWidget::Status::kException);
}

QString ErrorEntryWidget::Name() const {
    return diagnostic_error_.name;
}

}
